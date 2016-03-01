/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>

#include "plm_int.h"
#include "plm_math.h"
#include "vf_convolve.h"
#include "volume.h"

void
vf_convolve_x (Volume* vf_out, Volume* vf_in, float* ker, int width)
{
    plm_long v, x, y, z;
    int half_width;
    float *in_img = (float*) vf_in->img;
    float *out_img = (float*) vf_out->img;

    half_width = width / 2;

    for (v = 0, z = 0; z < vf_in->dim[2]; z++) {
	for (y = 0; y < vf_in->dim[1]; y++) {
	    for (x = 0; x < vf_in->dim[0]; x++, v++) {
		plm_long i, i1;	    /* i is the offset in the vf */
		plm_long j, j1, j2;   /* j is the index of the kernel */
		int d;		    /* d is the vector field direction */
		float *vout = &out_img[3*v];

		if (x < half_width) {
		    i1 = 0;
		    j1 = half_width - x;
		} else {
		    i1 = x - half_width;
		    j1 = 0;
		}
		if (x + half_width > vf_in->dim[0] - 1) {
		    j2 = half_width + (vf_in->dim[0] - x) - 1;
		} else {
		    j2 = 2 * half_width;
		}

		for (d = 0; d < 3; d++) {
                    float ktot = 0.0f;
		    vout[d] = (float) 0.0;
		    for (i = i1, j = j1; j <= j2; i++, j++) {
			plm_long idx = vf_in->index (i, y, z);
			vout[d] += ker[j] * in_img [idx*3+d];
                        ktot += ker[j];
		    }
                    vout[d] /= ktot;
		}
#if defined (commentout)
		printf ("%u %u %u | %u | %u %u %u\n",
		    z, y, x, v, i1, j1, j2);
#endif
	    }
	}
    }
}

void
vf_convolve_y (Volume* vf_out, Volume* vf_in, float* ker, int width)
{
    plm_long v, x, y, z;
    int half_width;
    float *in_img = (float*) vf_in->img;
    float *out_img = (float*) vf_out->img;

    half_width = width / 2;

    for (v = 0, z = 0; z < vf_in->dim[2]; z++) {
	for (y = 0; y < vf_in->dim[1]; y++) {
	    for (x = 0; x < vf_in->dim[0]; x++, v++) {
		plm_long i, i1;	    /* i is the offset in the vf */
		plm_long j, j1, j2;   /* j is the index of the kernel */
		int d;		    /* d is the vector field direction */
		float *vout = &out_img[3*v];

		if (y < half_width) {
		    i1 = 0;
		    j1 = half_width - y;
		} else {
		    i1 = y - half_width;
		    j1 = 0;
		}
		if (y + half_width > vf_in->dim[1] - 1) {
		    j2 = half_width + (vf_in->dim[1] - y) - 1;
		} else {
		    j2 = 2 * half_width;
		}

		for (d = 0; d < 3; d++) {
                    float ktot = 0.0f;
		    vout[d] = (float) 0.0;
		    for (i = i1, j = j1; j <= j2; i++, j++) {
			plm_long idx = vf_in->index (x, i, z);
			vout[d] += ker[j] * in_img [idx*3+d];
                        ktot += ker[j];
		    }
                    vout[d] /= ktot;
		}
	    }
	}
    }
}

void
vf_convolve_z (Volume* vf_out, Volume* vf_in, float* ker, int width)
{
    plm_long v, x, y, z;
    int half_width;
    float *in_img = (float*) vf_in->img;
    float *out_img = (float*) vf_out->img;

    half_width = width / 2;

    for (v = 0, z = 0; z < vf_in->dim[2]; z++) {
	for (y = 0; y < vf_in->dim[1]; y++) {
	    for (x = 0; x < vf_in->dim[0]; x++, v++) {
		plm_long i, i1;	    /* i is the offset in the vf */
		plm_long j, j1, j2;   /* j is the index of the kernel */
		int d;		    /* d is the vector field direction */
		float *vout = &out_img[3*v];

		if (z < half_width) {
		    i1 = 0;
		    j1 = half_width - z;
		} else {
		    i1 = z - half_width;
		    j1 = 0;
		}
		if (z + half_width > vf_in->dim[2] - 1) {
		    j2 = half_width + (vf_in->dim[2] - z) - 1;
		} else {
		    j2 = 2 * half_width;
		}

		for (d = 0; d < 3; d++) {
                    float ktot = 0.0f;
		    vout[d] = (float) 0.0;
		    for (i = i1, j = j1; j <= j2; i++, j++) {
			plm_long idx = vf_in->index (x, y, i);
			vout[d] += ker[j] * in_img [idx*3+d];
                        ktot += ker[j];
		    }
                    vout[d] /= ktot;
		}
	    }
	}
    }
}

