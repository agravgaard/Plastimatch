/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_lut_h_
#define _rt_lut_h_

#include <math.h>

double get_proton_range(double energy);
double get_proton_stop(double energy);

/* Range compensator functions, Highland-based definition or Monte Carlo based definition */
double get_theta0_Highland(double range);
double get_theta0_MC(float energy);
double get_theta_rel_Highland(double rc_over_range);
double get_theta_rel_MC(double rc_over_range);
double get_scat_or_Highland(double rc_over_range);
double get_scat_or_MC(double rc_over_range);

double compute_X0_from_HU(double CT_HU); // Radiation length

extern const double lookup_proton_range_water[][2];
extern const double lookup_proton_stop_water[][2];

/* declaration of a matrix that contains the alpha and p parameters of the particles (Range = f(E, alpha, p) */
extern const double particle_parameters[][2];

/* Definition of some constants for the Plastimatch dose module */
const int PROTON_TABLE_SIZE = 110;		// Look up table size -1 
const double PROTON_E_MAX = 255;		// Maximum proton energy dealt by Plastimatch is 255 MeV.
const double LIGHT_SPEED = 299792458;	// m.s-1
const double PROTON_REST_MASS = 939.4;	// MeV
const double PROTON_WER_AIR = 0.88;
const double AIR_DENSITY = 0.001205;    // g.cm-3


/* Error Function parameters */
const double ERF_A1 =  0.254829592;
const double ERF_A2 = -0.284496736;
const double ERF_A3 =  1.421413741;
const double ERF_A4 = -1.453152027;
const double ERF_A5 =  1.061405429;
const double ERF_P  =  0.3275911;

#endif
