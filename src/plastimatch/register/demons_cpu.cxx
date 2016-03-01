/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "demons.h"
#include "gaussian.h"
#include "plm_int.h"
#include "plm_math.h"
#include "plm_timer.h"
#include "vf_convolve.h"
#include "volume.h"

/* Vector fields are all in mm units */
Volume*
demons_c (
    Volume* fixed, 
    Volume* moving, 
    Volume* moving_grad, 
    Volume* vf_init, 
    Demons_parms* parms)
{
    plm_long i, j, k, v;
    int	it;	           /* Iterations */
    plm_long fi, fj, fk, fv; /* Indices into fixed image */
    float f2mo[3];         /* Origin difference (in mm) from fixed to moving */
    float f2ms[3];         /* Slope to convert fixed to moving */
    float invmps[3];       /* 1/pixel spacing of moving image */
    float *kerx, *kery, *kerz;
    float *dxyz;
    float mi, mj, mk;
    int mx, my, mz;
    plm_long mv;
    int fw[3];
    double diff_run;
    Volume *vf_est, *vf_smooth;
    Volume *m_grad_mag;
    float *f_img = (float*) fixed->img;
    float *m_img = (float*) moving->img;
    float *m_grad_img = (float*) moving_grad->img;
    float *m_grad_mag_img;
    float *vox_grad, vox_grad_mag;
    float diff, mult, denom;
    float *vf_est_img, *vf_smooth_img;
    size_t inliers;
    float ssd;

    /* Allocate memory for vector fields */
    if (vf_init) {
	/* If caller has an initial estimate, we copy it */
	vf_smooth = volume_clone (vf_init);
	vf_convert_to_interleaved (vf_smooth);
    } else {
	/* Otherwise initialize to zero */
	vf_smooth = new Volume (fixed->dim, fixed->origin, fixed->spacing, 
	    fixed->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3);
    }
    vf_est = new Volume (fixed->dim, fixed->origin, fixed->spacing, 
	fixed->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3);
    vf_est_img = (float*) vf_est->img;
    vf_smooth_img = (float*) vf_smooth->img;
    m_grad_mag = new Volume (moving->dim, moving->origin, moving->spacing, 
	moving->direction_cosines, PT_FLOAT, 1);
    m_grad_mag_img = (float*) m_grad_mag->img;

    /* Create gradient magnitude image */
    for (v = 0, k = 0; k < moving->dim[2]; k++) {
	for (j = 0; j < moving->dim[1]; j++) {
	    for (i = 0; i < moving->dim[0]; i++, v++) {
		vox_grad = &m_grad_img[3*v];
		m_grad_mag_img[v] = vox_grad[0] * vox_grad[0] 
		    + vox_grad[1] * vox_grad[1] + vox_grad[2] * vox_grad[2];
	    }
	}
    }

    /* Validate filter widths */
    validate_filter_widths (fw, parms->filter_width);

    /* Create the seperable smoothing kernels for the x, y, and z directions */
    kerx = create_ker (parms->filter_std / fixed->spacing[0], fw[0]/2);
    kery = create_ker (parms->filter_std / fixed->spacing[1], fw[1]/2);
    kerz = create_ker (parms->filter_std / fixed->spacing[2], fw[2]/2);
    kernel_stats (kerx, kery, kerz, fw);

    /* Compute some variables for converting pixel sizes / origins */
    for (i = 0; i < 3; i++) {
	invmps[i] = 1 / moving->spacing[i];
	f2mo[i] = (fixed->origin[i] - moving->origin[i]) / moving->spacing[i];
	f2ms[i] = fixed->spacing[i] / moving->spacing[i];
    }

    Plm_timer* timer = new Plm_timer;
    Plm_timer* it_timer = new Plm_timer;

    timer->start ();
    it_timer->start ();

    /* Main loop through iterations */
    for (it = 0; it < parms->max_its; it++) {

	/* Estimate displacement, store into vf_est */
	inliers = 0; ssd = 0.0;
	memcpy (vf_est_img, vf_smooth_img, vf_est->npix * vf_est->pix_size);
	for (fv = 0, fk = 0, mk = f2mo[2]; fk < fixed->dim[2]; fk++, mk+=f2ms[2]) {
	    for (fj = 0, mj = f2mo[1]; fj < fixed->dim[1]; fj++, mj+=f2ms[1]) {
		for (fi = 0, mi = f2mo[0]; fi < fixed->dim[0]; fi++, mi+=f2ms[0], fv++) {

		    /* Find correspondence with nearest neighbor interpolation & boundary checking */
		    dxyz = &vf_smooth_img [3*fv];			/* mm */
		    mz = ROUND_INT(mk + invmps[2] * dxyz[2]);		/* pixels (moving) */
		    if (mz < 0 || mz >= moving->dim[2]) continue;
		    my = ROUND_INT(mj + invmps[1] * dxyz[1]);		/* pixels (moving) */
		    if (my < 0 || my >= moving->dim[1]) continue;
		    mx = ROUND_INT(mi + invmps[0] * dxyz[0]);		/* pixels (moving) */
		    if (mx < 0 || mx >= moving->dim[0]) continue;
		    mv = moving->index (mx, my, mz);

		    /* Find image difference at this correspondence */
		    diff = f_img[fv] - m_img[mv];			/* intensity */

		    /* Find spatial gradient at this correspondece */
		    vox_grad = &m_grad_img[3*mv];			/* intensity per mm */
		    vox_grad_mag = m_grad_mag_img[mv];			/* intensity^2 per mm^2 */

		    /* Compute denominator */
		    denom = vox_grad_mag + parms->homog * diff * diff;	/* intensity^2 per mm^2 */

		    /* Compute SSD for statistics */
		    inliers++;
		    ssd += diff * diff;

		    /* Threshold the denominator to stabilize estimation */
		    if (denom < parms->denominator_eps) continue;

		    /* Compute new estimate of displacement */
		    mult = parms->accel * diff / denom;			/* per intensity^2 */
		    vf_est_img[3*fv + 0] += mult * vox_grad[0];		/* mm */
		    vf_est_img[3*fv + 1] += mult * vox_grad[1];
		    vf_est_img[3*fv + 2] += mult * vox_grad[2];
		}
	    }
	}
	//vf_print_stats (vf_est);

	/* Smooth the estimate into vf_smooth.  The volumes are ping-ponged. */
	vf_convolve_x (vf_smooth, vf_est, kerx, fw[0]);
	vf_convolve_y (vf_est, vf_smooth, kery, fw[1]);
	vf_convolve_z (vf_smooth, vf_est, kerz, fw[2]);
	//vf_print_stats (vf_smooth);

	double duration = it_timer->report ();
	printf ("MSE [%4d] %.01f (%.03f) [%6.3f secs]\n", it, ssd/inliers, 
	    ((float) inliers / fixed->npix), duration);
	it_timer->start ();
    }

    free (kerx);
    free (kery);
    free (kerz);
    delete vf_est;
    delete m_grad_mag;

    diff_run = timer->report ();
    printf ("Time for %d iterations = %f (%f sec / it)\n", 
	parms->max_its, diff_run, diff_run / parms->max_its);

    delete timer;
    delete it_timer;

    return vf_smooth;
}
