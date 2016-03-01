/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "clamp.h"
#include "logfile.h"
#include "plm_int.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "volume.h"
#include "volume_header.h"
#include "volume_conv.h"

static void
pixel_conv (
    float* img_out,
    const float *img_in,
    const float *img_ker,
    const plm_long *dim_in,
    const plm_long *dim_ker,
    const plm_long *ker_hw,
    const plm_long *ijk_out)
{
    /* GCS FIX: This could be made faster by adding preamble and 
       postable loops rather than clamping */
    plm_long ijk_ker[3];  /* kernel ijk of kernel */
    plm_long ijk_in[3];      /* image ijk of kernel overlaid */
    plm_long out_v = volume_index (dim_in, ijk_out);

    for (ijk_ker[2] = 0; ijk_ker[2] < dim_ker[2]; ijk_ker[2]++) {
        ijk_in[2] = ijk_out[2] + ijk_ker[2] - ker_hw[2];
        CLAMP (ijk_in[2], 0, dim_in[2]-1);
        for (ijk_ker[1] = 0; ijk_ker[1] < dim_ker[1]; ijk_ker[1]++) {
            ijk_in[1] = ijk_out[1] + ijk_ker[1] - ker_hw[1];
            CLAMP (ijk_in[1], 0, dim_in[1]-1);
            for (ijk_ker[0] = 0; ijk_ker[0] < dim_ker[0]; ijk_ker[0]++) {
                ijk_in[0] = ijk_out[0] + ijk_ker[0] - ker_hw[0];
                CLAMP (ijk_in[0], 0, dim_in[0]-1);
                plm_long ker_v = volume_index (dim_ker, ijk_ker);
                plm_long in_v = volume_index (dim_in, ijk_in);
                img_out[out_v] += img_ker[ker_v] * img_in[in_v];
            }
        }
    }
}

Volume::Pointer
volume_conv (
    const Volume::Pointer& vol_in,
    const Volume::Pointer& ker_in)
{
    Volume::Pointer vol_out = vol_in->clone_empty();
    const float *img_in = vol_in->get_raw<float> ();
    const float *img_ker = ker_in->get_raw<float> ();
    float *img_out = vol_out->get_raw<float> ();
    const plm_long* dim_in = vol_in->dim;
    const plm_long* dim_ker = ker_in->dim;

    /* Compute kernel half-width */
    plm_long ker_hw[3];
    for (int d = 0; d < 3; d++) {
        ker_hw[d] = dim_ker[d] / 2;
    }

#pragma omp parallel for 
    LOOP_Z_OMP (k, vol_in) {
        plm_long ijk_out[3];
        ijk_out[2] = k;
        for (ijk_out[1] = 0; ijk_out[1] < vol_in->dim[1]; ijk_out[1]++) {
            for (ijk_out[0] = 0; ijk_out[0] < vol_in->dim[0]; ijk_out[0]++) {
                pixel_conv (img_out, img_in, img_ker, dim_in, dim_ker, 
                    ker_hw, ijk_out);
            }
        }
    }

    return vol_out;
}

void
volume_convolve_x (
    Volume::Pointer& vol_out,
    const Volume::Pointer& vol_in,
    float *ker,
    int width
)
{
    const float *img_in = vol_in->get_raw<float> ();
    float *img_out = vol_out->get_raw<float> ();
    const plm_long* dim_in = vol_in->dim;
    int half_width = width / 2;

#pragma omp parallel for 
    LOOP_Z_OMP (k, vol_in) {
        plm_long ijk[3];
        ijk[2] = k;
	for (ijk[1] = 0; ijk[1] < dim_in[1]; ijk[1]++) {
	    for (ijk[0] = 0; ijk[0] < dim_in[0]; ijk[0]++) {
		plm_long i, i1;	    /* i is the offset in the vol */
		plm_long j, j1, j2;   /* j is the index of the kernel */
                plm_long v = volume_index (dim_in, ijk);

		if (ijk[0] < half_width) {
		    i1 = 0;
		    j1 = half_width - ijk[0];
		} else {
		    i1 = ijk[0] - half_width;
		    j1 = 0;
		}
		if (ijk[0] + half_width > dim_in[0] - 1) {
		    j2 = half_width + (dim_in[0] - ijk[0]) - 1;
		} else {
		    j2 = 2 * half_width;
		}

                float ktot = 0.0f;
                img_out[v] = (float) 0.0;
                for (i = i1, j = j1; j <= j2; i++, j++) {
                    plm_long idx = vol_in->index (i, ijk[1], ijk[2]);
                    img_out[v] += ker[j] * img_in [idx];
                    ktot += ker[j];
                }
                img_out[v] /= ktot;
#if defined (commentout)
		printf ("%u %u %u | %u | %u %u %u\n",
		    ijk[2], ijk[1], ijk[0], v, i1, j1, j2);
#endif
	    }
	}
    }
}

void
volume_convolve_y (
    Volume::Pointer& vol_out,
    const Volume::Pointer& vol_in,
    float *ker,
    int width
)
{
    const float *img_in = vol_in->get_raw<float> ();
    float *img_out = vol_out->get_raw<float> ();
    const plm_long* dim_in = vol_in->dim;
    int half_width = width / 2;

#pragma omp parallel for 
    LOOP_Z_OMP (k, vol_in) {
        plm_long ijk[3];
        ijk[2] = k;
	for (ijk[1] = 0; ijk[1] < vol_in->dim[1]; ijk[1]++) {
	    for (ijk[0] = 0; ijk[0] < vol_in->dim[0]; ijk[0]++) {
		plm_long i, i1;	    /* i is the offset in the vf */
		plm_long j, j1, j2;   /* j is the index of the kernel */
                plm_long v = volume_index (dim_in, ijk);

		if (ijk[1] < half_width) {
		    i1 = 0;
		    j1 = half_width - ijk[1];
		} else {
		    i1 = ijk[1] - half_width;
		    j1 = 0;
		}
		if (ijk[1] + half_width > vol_in->dim[1] - 1) {
		    j2 = half_width + (vol_in->dim[1] - ijk[1]) - 1;
		} else {
		    j2 = 2 * half_width;
		}

                float ktot = 0.0f;
                img_out[v] = (float) 0.0;
                for (i = i1, j = j1; j <= j2; i++, j++) {
                    plm_long idx = vol_in->index (ijk[0], i, ijk[2]);
                    img_out[v] += ker[j] * img_in [idx];
                    ktot += ker[j];
                }
                img_out[v] /= ktot;
            }
	}
    }

}

void
volume_convolve_z (
    Volume::Pointer& vol_out,
    const Volume::Pointer& vol_in,
    float *ker,
    int width
)
{
    const float *img_in = vol_in->get_raw<float> ();
    float *img_out = vol_out->get_raw<float> ();
    const plm_long* dim_in = vol_in->dim;
    int half_width = width / 2;

#pragma omp parallel for 
    LOOP_Z_OMP (k, vol_in) {
        plm_long ijk[3];
        ijk[2] = k;
	for (ijk[1] = 0; ijk[1] < vol_in->dim[1]; ijk[1]++) {
	    for (ijk[0] = 0; ijk[0] < vol_in->dim[0]; ijk[0]++) {
		plm_long i, i1;	    /* i is the offset in the vf */
		plm_long j, j1, j2;   /* j is the index of the kernel */
                plm_long v = volume_index (dim_in, ijk);

		if (ijk[2] < half_width) {
		    i1 = 0;
		    j1 = half_width - ijk[2];
		} else {
		    i1 = ijk[2] - half_width;
		    j1 = 0;
		}
		if (ijk[2] + half_width > vol_in->dim[2] - 1) {
		    j2 = half_width + (vol_in->dim[2] - ijk[2]) - 1;
		} else {
		    j2 = 2 * half_width;
		}

                float ktot = 0.0f;
                img_out[v] = (float) 0.0;
                for (i = i1, j = j1; j <= j2; i++, j++) {
                    plm_long idx = vol_in->index (ijk[0], ijk[1], i);
                    img_out[v] += ker[j] * img_in [idx];
                    ktot += ker[j];
                }
                img_out[v] /= ktot;
	    }
	}
    }
}

Volume::Pointer
volume_convolve_separable
(
    const Volume::Pointer& vol_in,
    float *ker_i,
    int width_i,
    float *ker_j,
    int width_j,
    float *ker_k,
    int width_k
)
{
    Volume::Pointer vol_1 = vol_in->clone_empty();
    Volume::Pointer vol_2 = vol_in->clone_empty();

    volume_convolve_x (vol_1, vol_in, ker_i, width_i);
    volume_convolve_y (vol_2, vol_1, ker_j, width_j);
    volume_convolve_z (vol_1, vol_2, ker_k, width_k);

    return vol_1;

}
