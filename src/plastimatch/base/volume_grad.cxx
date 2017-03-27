/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "logfile.h"
#include "volume_grad.h"

static void
volume_calc_grad (Volume* vout, const Volume* vref)
{
    plm_long i, j, k;
    plm_long i_n, j_n, k_n;     /* n is next */
    int i_p, j_p, k_p;          /* p is prev */
    plm_long gi, gj, gk;
    plm_long idx_p, idx_n;
    float *out_img, *ref_img;

    out_img = (float*) vout->img;
    ref_img = (float*) vref->img;

    const float *inv_dc = vref->direction_cosines.get_inverse();

    plm_long v = 0;
    for (k = 0; k < vref->dim[2]; k++) {
	k_p = k - 1;
	k_n = k + 1;
	if (k == 0) k_p = 0;
	if (k == vref->dim[2]-1) k_n = vref->dim[2]-1;
	for (j = 0; j < vref->dim[1]; j++) {
	    j_p = j - 1;
	    j_n = j + 1;
	    if (j == 0) j_p = 0;
	    if (j == vref->dim[1]-1) j_n = vref->dim[1]-1;
	    for (i = 0; i < vref->dim[0]; i++, v++) {
		float diff;
		i_p = i - 1;
		i_n = i + 1;
		if (i == 0) i_p = 0;
		if (i == vref->dim[0]-1) i_n = vref->dim[0]-1;
		
		gi = 3 * v + 0;
		gj = 3 * v + 1;
		gk = 3 * v + 2;
		out_img[gi] = 0.f;
		out_img[gj] = 0.f;
		out_img[gk] = 0.f;

		idx_p = volume_index (vref->dim, i_p, j, k);
		idx_n = volume_index (vref->dim, i_n, j, k);
		diff = (float) (ref_img[idx_n] - ref_img[idx_p]) 
		    / 2.0 / vref->spacing[0];
		out_img[gi] += diff * inv_dc[0*3+0];
		out_img[gj] += diff * inv_dc[1*3+0];
		out_img[gk] += diff * inv_dc[2*3+0];

		idx_p = volume_index (vref->dim, i, j_p, k);
		idx_n = volume_index (vref->dim, i, j_n, k);
		diff = (float) (ref_img[idx_n] - ref_img[idx_p]) 
		    / 2.0 / vref->spacing[1];
		out_img[gi] += diff * inv_dc[0*3+1];
		out_img[gj] += diff * inv_dc[1*3+1];
		out_img[gk] += diff * inv_dc[2*3+1];

		idx_p = volume_index (vref->dim, i, j, k_p);
		idx_n = volume_index (vref->dim, i, j, k_n);
		diff = (float) (ref_img[idx_n] - ref_img[idx_p]) 
		    / 2.0 / vref->spacing[2];
		out_img[gi] += diff * inv_dc[0*3+2];
		out_img[gj] += diff * inv_dc[1*3+2];
		out_img[gk] += diff * inv_dc[2*3+2];
	    }
	}
    }
    lprintf ("volume_calc_grad complete.\n");
}

static void
volume_calc_grad_mag (Volume* vout, const Volume* vref)
{
    plm_long i, j, k;
    plm_long i_n, j_n, k_n;     /* n is next */
    int i_p, j_p, k_p;          /* p is prev */
    plm_long idx_p, idx_n;
    float *out_img, *ref_img;

    out_img = (float*) vout->img;
    ref_img = (float*) vref->img;

    plm_long v = 0;
    for (k = 0; k < vref->dim[2]; k++) {
	k_p = k - 1;
	k_n = k + 1;
	if (k == 0) k_p = 0;
	if (k == vref->dim[2]-1) k_n = vref->dim[2]-1;
	for (j = 0; j < vref->dim[1]; j++) {
	    j_p = j - 1;
	    j_n = j + 1;
	    if (j == 0) j_p = 0;
	    if (j == vref->dim[1]-1) j_n = vref->dim[1]-1;
	    for (i = 0; i < vref->dim[0]; i++, v++) {
		float diff;
		i_p = i - 1;
		i_n = i + 1;
		if (i == 0) i_p = 0;
		if (i == vref->dim[0]-1) i_n = vref->dim[0]-1;
		
                /* No need to consider direction cosines because 
                   we're only computing the magnitude */
		out_img[v] = 0.f;

		idx_p = volume_index (vref->dim, i_p, j, k);
		idx_n = volume_index (vref->dim, i_n, j, k);
		diff = (float) (ref_img[idx_n] - ref_img[idx_p]) 
		    / 2.0 / vref->spacing[0];
		out_img[v] += diff * diff;

		idx_p = volume_index (vref->dim, i, j_p, k);
		idx_n = volume_index (vref->dim, i, j_n, k);
		diff = (float) (ref_img[idx_n] - ref_img[idx_p]) 
		    / 2.0 / vref->spacing[1];
		out_img[v] += diff * diff;

		idx_p = volume_index (vref->dim, i, j, k_p);
		idx_n = volume_index (vref->dim, i, j, k_n);
		diff = (float) (ref_img[idx_n] - ref_img[idx_p]) 
		    / 2.0 / vref->spacing[2];
		out_img[v] += diff * diff;
                out_img[v] = sqrt (out_img[v]);
	    }
	}
    }
    lprintf ("volume_calc_grad_mag complete.\n");
}

Volume::Pointer
volume_gradient (const Volume::Pointer& ref)
{
    Volume::Pointer grad = Volume::New (
        ref->dim, ref->origin, ref->spacing, 
	ref->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3);
    volume_calc_grad (grad.get(), ref.get());

    return grad;
}

Volume* 
volume_make_gradient (const Volume* ref)
{
    Volume* grad = new Volume (
        ref->dim, ref->origin, ref->spacing, 
	ref->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3);
    volume_calc_grad (grad, ref);

    return grad;
}

Volume::Pointer
volume_gradient_magnitude (const Volume::Pointer& ref)
{
    Volume::Pointer grad = Volume::New (
        ref->dim, ref->origin, ref->spacing, 
	ref->direction_cosines, PT_FLOAT, 1);
    volume_calc_grad_mag (grad.get(), ref.get());

    return grad;
}
