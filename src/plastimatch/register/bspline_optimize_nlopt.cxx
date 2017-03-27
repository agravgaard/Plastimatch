/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#if (NLOPT_FOUND)
#include "nlopt.h"
#endif

#include "bspline.h"
#include "bspline_optimize.h"
#include "bspline_optimize_nlopt.h"
#include "bspline_parms.h"
#include "bspline_xform.h"
#include "plm_math.h"

#if (NLOPT_FOUND)
/* NLopt score function */
static double
bspline_optimize_nlopt_score (
    int n, 
    const double *x, 
    double *grad, 
    void *data)
{
    int i;
    Bspline_optimize *bod = (Bspline_optimize*) data;
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();
    
    /* Copy x in */
    for (i = 0; i < bxf->num_coeff; i++) {
	bxf->coeff[i] = (float) x[i];
    }

    /* Compute cost and gradient */
    bspline_score (bod);

    /* Copy gradient out */
    for (i = 0; i < bxf->num_coeff; i++) {
	grad[i] = (double) bst->ssd.total_grad[i];
    }

    /* Return cost */
    return (double) bst->ssd.total_score;
}

void
bspline_optimize_nlopt (Bspline_optimize *bod, nlopt_algorithm algorithm)
{
    int i;
    double *lb, *ub, *x;
    double minf;

    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    x = (double*) malloc (sizeof(double) * bxf->num_coeff);
    lb = (double*) malloc (sizeof(double) * bxf->num_coeff);
    ub = (double*) malloc (sizeof(double) * bxf->num_coeff);

    for (i = 0; i < bxf->num_coeff; i++) {
	lb[i] = -HUGE_VAL;
	ub[i] = +HUGE_VAL;
	x[i] = (double) bxf->coeff[i];
    }

    nlopt_result nr = 
	nlopt_minimize (
	    algorithm, 
	    bxf->num_coeff, 
	    bspline_optimize_nlopt_score, 
	    bod, 
	    lb, 
	    ub, 
	    x, 
	    &minf, 
	    -HUGE_VAL, 
	    0, 
	    1., 
	    0, 
	    0, 
	    parms->max_its, 
	    0);
    printf ("nlopt returned: %d\n", nr);

    for (i = 0; i < bxf->num_coeff; i++) {
	bxf->coeff[i] = (float) x[i];
    }

    free (x);
    free (ub);
    free (lb);
}

#endif
