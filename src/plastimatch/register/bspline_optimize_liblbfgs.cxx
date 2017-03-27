/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "bspline.h"
#include "bspline_optimize.h"
#include "bspline_optimize_liblbfgs.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "logfile.h"
#include "plm_math.h"

static lbfgsfloatval_t 
evaluate (
    void *instance,
    const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g,
    const int n,
    const lbfgsfloatval_t step)
{
    Bspline_optimize *bod = (Bspline_optimize*) instance;
    Bspline_xform *bxf = bod->get_bspline_xform ();
    Bspline_state *bst = bod->get_bspline_state ();
    int i;
    
    /* Copy x in */
    for (i = 0; i < bxf->num_coeff; i++) {
	bxf->coeff[i] = (float) x[i];
    }

    /* Compute cost and gradient */
    bspline_score (bod);

    /* Copy gradient out */
    for (i = 0; i < bxf->num_coeff; i++) {
	g[i] = (lbfgsfloatval_t) bst->ssd.total_grad[i];
    }

    /* Increment num function evals */
    bst->feval ++;

    /* Return cost */
    return (lbfgsfloatval_t) bst->ssd.total_score;
}

static int
progress(
    void *instance,
    const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g,
    const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm,
    const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step,
    int n,                        /* Size of input vector */
    int k,                        /* Iteration number */
    int ls                        /* feval within this iteration */
)
{
    Bspline_optimize *bod = (Bspline_optimize*) instance;
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();

    logfile_printf (
	"                      XN %9.3f GN %9.3f ST %9.3f\n", 
	xnorm, gnorm, step);
    bst->it = k;
    if (bst->it > parms->max_its
	|| bst->feval > parms->max_feval) {
	return 1;
    }
    return 0;
}

void
bspline_optimize_liblbfgs (Bspline_optimize *bod)
{
    int i, rc;
    lbfgsfloatval_t fx;
    lbfgs_parameter_t param;
    lbfgsfloatval_t *x;

    Bspline_xform *bxf = bod->get_bspline_xform ();

    x = lbfgs_malloc (bxf->num_coeff);

    /* Convert x0 from float to lbfgsfloatval_t */
    for (i = 0; i < bxf->num_coeff; i++) {
	x[i] = (lbfgsfloatval_t) bxf->coeff[i];
    }

    /* Set default parameters */
    lbfgs_parameter_init (&param);

    /* Run the optimizer */
    rc = lbfgs (bxf->num_coeff, x, &fx, 
	evaluate, progress, (void*) bod, &param);

    (void) rc;  /* Suppress compiler warning */

    lbfgs_free (x);
}
