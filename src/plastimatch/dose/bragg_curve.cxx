/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <math.h>
#include "bragg_curve.h"
#include "rt_lut.h"

typedef double doublereal;

/* unmangled symbol in specfun */
// PBDV(V,X,DV,DP,PDF,PDD)
extern "C" void
pbdv_ (
    doublereal* v,
    doublereal* x,
    doublereal* dv,
    doublereal* dp,
    doublereal* pdf,
    doublereal* pdd);

/* Note: Inputs to this function are in mm, but internal computations 
   are done in cm. */
double
bragg_curve (
    double E_0,         /* in MeV */
    double sigma_E0,    /* in MeV */
    double z            /* in mm */
)
{
    doublereal v, x, dv[100], dp[100], pdd;
    doublereal D_v_1, D_v_2;
    double p = particle_parameters[0][1];
    double alpha = particle_parameters[0][0];
    double R_0 = alpha * pow (E_0, p);
    double sigma_mono = 0.012 * pow (R_0, 0.935);
    double epsilon = 0.1;

    double sigma_squared = sigma_mono * sigma_mono 
        + (sigma_E0 * sigma_E0 * alpha * alpha * p * p 
            * pow (E_0 * E_0, (p - 2)));
    double sigma = sqrt (sigma_squared);
    
    double rr;
    double bragg;

    /* Convert z from mm to cm */
    z = 0.1 * z;

    /* Compute residual range (rr) */
    rr = R_0 - z;

    /* Use approximation Dhat in plateau region due to instability
       of computing parabolic cylinder function for large x */
    if (rr > 10.0 * sigma) {
        bragg = 1 / (1 + 0.012 * R_0) 
            * (17.93 * pow (rr, -0.435)
                + (0.444 + 31.7 * epsilon / R_0) * pow (rr, 0.565));
        return bragg;
    }

    /* Term 1 of eqn 29 */
    bragg = exp (- (rr * rr) / (4 * sigma * sigma)) * pow (sigma, 0.565)
        / (1 + 0.012 * R_0);

    /* D_v of -0.565 */
    v = - 0.565;
    x = - rr / sigma;
    pbdv_ (&v, &x, dv, dp, &D_v_1, &pdd);
    
    /* D_v of -1.565 */
    v = - 1.565;
    x = - rr / sigma;
    pbdv_ (&v, &x, dv, dp, &D_v_2, &pdd);
    
    /* Term 2 of eqn 29 */
    bragg = bragg * ((11.26 / sigma) * D_v_1
        + (0.157 + 11.26 * epsilon / R_0) * D_v_2);

    return bragg;
}
