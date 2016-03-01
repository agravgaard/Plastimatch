/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* In this code, x is the location (in mm), and y is the thoracic vertebra 
   number (1 to 12 for T1 to T12).  Slopes are measured in units of 
   vertebra per millimeter. 
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "autolabel_ransac_est.h"
#include "RANSAC.h"
#include "PlaneParametersEstimator.h"
#include "RandomNumberGenerator.h"
#include "itk_point.h"

typedef itk::RANSAC< Autolabel_point, double> Ransac_type;

//#define DELTA         (0.5)
#define DELTA         (1.0)
#define DELTA_SQUARED (DELTA * DELTA)

static double
est_piecewise_spline_point (
    Autolabel_point& data, 
    std::vector<double>& piecewise_parms
)
{
    double slope_a = piecewise_parms[0];
    double t4_loc = piecewise_parms[1];
    double t7_loc = piecewise_parms[2];
    double slope_c = piecewise_parms[3];
    double x = data[0];
    double yest;

    if (x > t4_loc) {
	double intercept = 4. - t4_loc * slope_a;
	yest = x * slope_a + intercept;
    } else if (x < t7_loc) {
	double intercept = 7. - t7_loc * slope_c;
	yest = x * slope_c + intercept;
    } else {
	double xoff = (x - t4_loc) / (t7_loc - t4_loc);
	yest = 4. * (1. - xoff) + 7. * xoff;
    }
    return yest;
}

static double
score_piecewise_spline_point (
    Autolabel_point& data, 
    std::vector<double>& piecewise_parms
)
{
    double y = data[1];
    double yest = est_piecewise_spline_point (data, piecewise_parms);
    double ydiff = y - yest;
    double yscore = ydiff*ydiff / DELTA_SQUARED;
    if (yscore > 1.) {
	yscore = 1.;
    }
    return yscore;
}

static double
score_piecewise_spline (
    Autolabel_point_vector& apv, 
    std::vector<double>& piecewise_parms
)
{
    double score = 0.0;
    Autolabel_point_vector::iterator it;
    for (it = apv.begin(); it != apv.end(); it++) {
	score += score_piecewise_spline_point ((*it), piecewise_parms);
    }
    return score;
}

static void
pattern_search (
    Autolabel_point_vector& apv, 
    std::vector<double>& piecewise_parms,
    double& parm,
    double constraint[2],
    double adjust,
    double& curr_score
)
{
    double curr = parm;
    double test1 = parm - adjust;
    double test2 = parm + adjust;
    double score;
    printf ("[%f %f %f] vs. [%f %f]\n", test1, parm, test2,
	constraint[0], constraint[1]);
    if (test1 > constraint[0]) {
	parm = test1;
	score = score_piecewise_spline (apv, piecewise_parms);
	printf ("  <%f,%f,%f,%f> %f %s\n", 
	    piecewise_parms[0], piecewise_parms[1],
	    piecewise_parms[2], piecewise_parms[3], 
	    score, score < curr_score ? "pass" : "fail");
	if (score < curr_score) {
	    curr_score = score;
	    return;
	} else {
	    parm = curr;
	}
    }
    if (test2 < constraint[1]) {
	parm = test2;
	score = score_piecewise_spline (apv, piecewise_parms);
	printf ("  <%f,%f,%f,%f> %f %s\n", 
	    piecewise_parms[0], piecewise_parms[1],
	    piecewise_parms[2], piecewise_parms[3], 
	    score, score < curr_score ? "pass" : "fail");
	if (score < curr_score) {
	    curr_score = score;
	} else {
	    parm = curr;
	}
    }
}

static void
optimize_piecewise_spline (
    Autolabel_point_vector& apv, 
    std::vector<double>& piecewise_parms
)
{
    const int MAX_ITS = 6;
    //int changed = 0;
    double constraints[3][2] = {
	{ -0.070, -0.040 },
	{ -0.056, -0.037 },
	{ -0.048, -0.029 }
    };

    /* Double check constraints */
    /* GCS FIX: ignoring constraints[1][*] for now. :( */
    if (piecewise_parms[0] < constraints[0][0]) {
	piecewise_parms[0] = constraints[0][0];
    } else if (piecewise_parms[0] > constraints[0][1]) {
	piecewise_parms[0] = constraints[0][1];
    }
    if (piecewise_parms[3] < constraints[2][0]) {
	piecewise_parms[3] = constraints[2][0];
    } else if (piecewise_parms[3] > constraints[2][1]) {
	piecewise_parms[3] = constraints[2][1];
    }

    double curr_score =  score_piecewise_spline (apv, piecewise_parms);
    printf ("Base score: %f\n", curr_score);

    for (int it = 0; it < MAX_ITS; it++) {
	double adjust;
	double loc_constraint[2];
	double t4_loc, t7_loc, t47_slope;

	/* Search slope_a */
	adjust = 0.01 * rand() / (float) RAND_MAX;
	//adjust = 0.002;
	printf ("-- A --\n");
	pattern_search (
	    apv, 
	    piecewise_parms,
	    piecewise_parms[0],
	    constraints[0],
	    adjust,
	    curr_score);

	/* Search slope_c */
	printf ("-- C --\n");
	adjust = 0.01 * rand() / (float) RAND_MAX;
	pattern_search (
	    apv, 
	    piecewise_parms,
	    piecewise_parms[3],
	    constraints[2],
	    adjust,
	    curr_score);

	/* Search t4_loc */
	printf ("-- T4 --\n");
	adjust = 10. * rand() / (float) RAND_MAX;
	//adjust = 1.
	t4_loc = piecewise_parms[1];
	t7_loc = piecewise_parms[2];
	t47_slope = 3. / (t7_loc - t4_loc);
	loc_constraint[0] = t7_loc + 3. / constraints[1][0];
	loc_constraint[1] = t7_loc + 3. / constraints[1][1];
	printf ("T4 = %f T7 = %f\n", t4_loc, t7_loc);
	printf ("t47_slope = %f constraints = [%f,%f]\n", 
	    t47_slope, constraints[1][0], constraints[1][1]);
	printf ("loc_constraint = [%f,%f]\n", 
	    loc_constraint[0], loc_constraint[1]);
#if defined (commentout)
#endif
	pattern_search (
	    apv, 
	    piecewise_parms,
	    piecewise_parms[1],
	    loc_constraint,
	    adjust,
	    curr_score);

	/* Search t7_loc */
	printf ("-- T7 --\n");
	adjust = 10. * rand() / (float) RAND_MAX;
	t4_loc = piecewise_parms[1];
	t7_loc = piecewise_parms[2];
	t47_slope = 3. / (t7_loc - t4_loc);
	loc_constraint[0] = t4_loc - 3. / constraints[1][1];
	loc_constraint[1] = t4_loc - 3. / constraints[1][0];
	printf ("T4 = %f T7 = %f\n", t4_loc, t7_loc);
	printf ("t47_slope = %f constraints = [%f,%f]\n", 
	    t47_slope, constraints[1][0], constraints[1][1]);
	printf ("loc_constraint = [%f,%f]\n", 
	    loc_constraint[0], loc_constraint[1]);
#if defined (commentout)
#endif
	pattern_search (
	    apv, 
	    piecewise_parms,
	    piecewise_parms[2],
	    loc_constraint,
	    adjust,
	    curr_score);
    }
}

void
autolabel_ransac_est (Autolabel_point_vector& apv)
{
    Autolabel_point_vector tmp;
    std::vector<double> autolabel_parameters;

    itk::Autolabel_ransac_est::Pointer estimator 
	= itk::Autolabel_ransac_est::New();
    estimator->SetDelta (DELTA);

    /* Run RANSAC */
    double desiredProbabilityForNoOutliers = 0.999;
    double percentageOfDataUsed;
    Ransac_type::Pointer ransac_estimator = Ransac_type::New();
    ransac_estimator->SetData (apv);
    ransac_estimator->SetParametersEstimator (estimator.GetPointer());
    percentageOfDataUsed = ransac_estimator->Compute (
	autolabel_parameters, desiredProbabilityForNoOutliers);
    if (autolabel_parameters.empty()) {
	std::cout<<"RANSAC estimate failed, degenerate configuration?\n";
	exit (-1);
    } else {
	printf ("RANSAC parameters: [s,i] = [%f,%f]\n", 
	    autolabel_parameters[0], autolabel_parameters[1]);
	printf ("Used %f percent of data.\n", percentageOfDataUsed);
    }

    /* Use pattern search to fit piecewise linear spline, using 
       RANSAC as initial guess.
       piecewise_parms[0] = slope in T1-T4 region
       piecewise_parms[1] = location of T4
       piecewise_parms[2] = location of T7
       piecewise_parms[3] = slope in T7 region
    */
    std::vector <double> piecewise_parms(4);
    double slope = autolabel_parameters[0];
    double intercept = autolabel_parameters[1];
    printf ("Initializing piecewise parms\n");
    piecewise_parms[0] = slope;
    piecewise_parms[1] = (4. - intercept) / slope;
    piecewise_parms[2] = (7. - intercept) / slope;
    piecewise_parms[3] = slope;

    printf ("Optimizing piecewise parms\n");
    optimize_piecewise_spline (apv, piecewise_parms);
    printf ("Done optimizing.\n");

#if defined (commentout)
    /* Fill in 3rd component of apv with RANSAC estimates */
    Autolabel_point_vector::iterator it;
    for (it = apv.begin(); it != apv.end(); it++) {
	double x = (*it)[0];
	//double y = (*it)[1];
	double ry = intercept + slope * x;
	(*it)[2] = ry;
    }
#endif

    /* Fill in 3rd component of apv with piecewise estimates */
    Autolabel_point_vector::iterator it;
    for (it = apv.begin(); it != apv.end(); it++) {
	double ry = est_piecewise_spline_point (*it, piecewise_parms);
	(*it)[2] = ry;
    }
}

namespace itk {

Autolabel_ransac_est::Autolabel_ransac_est()
{
    this->deltaSquared = NumericTraits<double>::min();
    this->minForEstimate = 2;
    this->min_slope = -0.079;
    this->max_slope = -0.030;
}

Autolabel_ransac_est::~Autolabel_ransac_est()
{
}

void Autolabel_ransac_est::SetDelta( double delta )
{
    this->deltaSquared = delta*delta;
}

double Autolabel_ransac_est::GetDelta()
{
    return sqrt( this->deltaSquared );
}

void
Autolabel_ransac_est::set_slope_constraints (
    double min_slope, 
    double max_slope
)
{
    this->min_slope = min_slope;
    this->max_slope = max_slope;
}

/* parameters[0] is slope, parameters[1] is offset */
void
Autolabel_ransac_est::Estimate (
    std::vector<Autolabel_point *> &data, 
    std::vector<double> &parameters
)
{
    const double EPS = 2*NumericTraits<double>::epsilon(); 

    /* Make sure we have two points */
    parameters.clear();
    if (data.size() < this->minForEstimate) {
	return;
    }

    /* Check x displacement is sufficient */
    Autolabel_point& p1 = *(data[0]);
    Autolabel_point& p2 = *(data[1]);
    double xdiff = p2[0] - p1[0];
    if (fabs(xdiff) < 10 * EPS) {
	return;
    }

    /* Compute slope */
    double ydiff = p2[1] - p1[1];
    double slope = ydiff / xdiff;
    double intercept = p1[1] - slope * p1[0];
    if (slope < this->min_slope || slope > this->max_slope) {
#if defined (commentout)
	printf ("slope failed (%f,%f) (%f,%f) s=%f, i=%f\n",
	    p1[0],p1[1],p2[0],p2[1],slope,intercept);
#endif
	return;
    }

    /* Compute intercept */

    parameters.push_back (slope);
    parameters.push_back (intercept);
}

/* WTF? */
void 
Autolabel_ransac_est::Estimate (
    std::vector< Autolabel_point > &data, 
    std::vector<double> &parameters
)
{
    std::vector< Autolabel_point *> usedData;
    int dataSize = data.size();
    for( int i=0; i<dataSize; i++ )
	usedData.push_back( &(data[i]) );
    Estimate( usedData, parameters );
}


void 
Autolabel_ransac_est::LeastSquaresEstimate (
    std::vector< Autolabel_point *> &data, 
    std::vector<double> &parameters
)
{
    /* Do something here */
}

/* WTF? */
void 
Autolabel_ransac_est::LeastSquaresEstimate (
    std::vector< Autolabel_point > &data, 
    std::vector<double> &parameters
)
{
    std::vector< Autolabel_point *> usedData;
    int dataSize = data.size();
    for( int i=0; i<dataSize; i++ )
	usedData.push_back( &(data[i]) );
    LeastSquaresEstimate( usedData, parameters );
}

bool
Autolabel_ransac_est::Agree (
    std::vector<double> &parameters, 
    Autolabel_point &data
)
{
    double slope = parameters[0];
    double intercept = parameters[1];

    double yest = intercept + slope * data[0];
    double ydiff = yest - data[1];
    return (ydiff*ydiff < this->deltaSquared);
}

} // end namespace itk
