/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "bspline_interpolate.h"
#include "bspline_xform.h"
#include "direction_cosines.h"
#include "interpolate_macros.h"
#include "plm_math.h"
#include "volume.h"
#include "volume_macros.h"

void
bspline_interp_pix (
    float out[3], 
    const Bspline_xform* bxf, 
    plm_long p[3], 
    plm_long qidx
) {
    int i, j, k, m;
    plm_long cidx;
    float* q_lut = &bxf->q_lut[qidx*64];

    out[0] = out[1] = out[2] = 0;
    m = 0;
    for (k = 0; k < 4; k++) {
        for (j = 0; j < 4; j++) {
            for (i = 0; i < 4; i++) {
                cidx = (p[2] + k) * bxf->cdims[1] * bxf->cdims[0]
                        + (p[1] + j) * bxf->cdims[0]
                        + (p[0] + i);
                cidx = cidx * 3;
                out[0] += q_lut[m] * bxf->coeff[cidx+0];
                out[1] += q_lut[m] * bxf->coeff[cidx+1];
                out[2] += q_lut[m] * bxf->coeff[cidx+2];
                m ++;
            }
        }
    }
}

void
bspline_interp_pix_b (
    float out[3], 
    Bspline_xform* bxf, 
    plm_long pidx, 
    plm_long qidx
)
{
    int i, j, k, m;
    plm_long cidx;
    float* q_lut = &bxf->q_lut[qidx*64];
    plm_long* c_lut = &bxf->c_lut[pidx*64];

    out[0] = out[1] = out[2] = 0;
    m = 0;
    for (k = 0; k < 4; k++) {
        for (j = 0; j < 4; j++) {
            for (i = 0; i < 4; i++) {
                cidx = 3 * c_lut[m];
                out[0] += q_lut[m] * bxf->coeff[cidx+0];
                out[1] += q_lut[m] * bxf->coeff[cidx+1];
                out[2] += q_lut[m] * bxf->coeff[cidx+2];
                m ++;
            }
        }
    }
}

void
bspline_interp_pix_c (
    float out[3],
    Bspline_xform* bxf,
    plm_long pidx,
    plm_long *q
)
{
    int i,j,k,m;
    plm_long cidx;
    float A,B,C;
    plm_long* c_lut = &bxf->c_lut[pidx*64];
    float* bx_lut = &bxf->bx_lut[q[0]*4];
    float* by_lut = &bxf->by_lut[q[1]*4];
    float* bz_lut = &bxf->bz_lut[q[2]*4];

    out[0] = out[1] = out[2] = 0;
    m=0;
    for (k=0; k<4; k++) {
        C = bz_lut[k];
        for (j=0; j<4; j++) {
            B = by_lut[j] * C;
            for (i=0; i<4; i++) {
                A = bx_lut[i] * B;

                cidx = 3*c_lut[m++];
                out[0] += A * bxf->coeff[cidx+0];
                out[1] += A * bxf->coeff[cidx+1];
                out[2] += A * bxf->coeff[cidx+2];
            }
        }
    }
}

void
bspline_interpolate_vf (Volume* interp, 
    const Bspline_xform* bxf)
{
    plm_long i, j, k, v;
    plm_long p[3];
    plm_long q[3];
    float* out;
    float* img = (float*) interp->img;
    plm_long qidx;

    memset (img, 0, interp->npix*3*sizeof(float));
    for (k = 0; k < bxf->roi_dim[2]; k++) {
        p[2] = k / bxf->vox_per_rgn[2];
        q[2] = k % bxf->vox_per_rgn[2];
        for (j = 0; j < bxf->roi_dim[1]; j++) {
            p[1] = j / bxf->vox_per_rgn[1];
            q[1] = j % bxf->vox_per_rgn[1];
            for (i = 0; i < bxf->roi_dim[0]; i++) {
                p[0] = i / bxf->vox_per_rgn[0];
                q[0] = i % bxf->vox_per_rgn[0];
                qidx = volume_index (bxf->vox_per_rgn, q);
                v = (k+bxf->roi_offset[2]) * interp->dim[0] * interp->dim[1]
                    + (j+bxf->roi_offset[1]) * interp->dim[0] 
                    + (i+bxf->roi_offset[0]);
                out = &img[3*v];
                bspline_interp_pix (out, bxf, p, qidx);
            }
        }
    }
}


/* This function uses the B-Spline coefficients to transform a point.  
   The point need not lie exactly on a voxel, so we do not use the 
   lookup table. */
void
bspline_transform_point (
    float point_out[3], /* Output coordinate of point */
    Bspline_xform* bxf, /* Bspline transform coefficients */
    float point_in[3],  /* Input coordinate of point */
    int linear_interp   /* 1 = trilinear, 0 = nearest neighbors */
)
{
    plm_long d, i, j, k;
    plm_long p[3];                    /* Index of tile */
    float q[3];                  /* Fractional offset within tile */
    float q_mini[3][4];          /* "miniature" q-lut, just for this point */

    /* Default value is untransformed point */
    for (d = 0; d < 3; d++) {
        point_out[d] = point_in[d];
    }

    /* Compute tile and offset within tile */
    for (d = 0; d < 3; d++) {
        float img_ijk[3];         /* Voxel coordinate of point_in */
        img_ijk[d] = (point_in[d] - bxf->img_origin[d]) / bxf->img_spacing[d];
        p[d] = (int) floorf (
            (img_ijk[d] - bxf->roi_offset[d]) / bxf->vox_per_rgn[d]);
        /* If point lies outside of B-spline domain, return point_in */
        if (p[d] < 0 || p[d] >= bxf->rdims[d]) {
            printf ("Unwarped point, outside roi: %f %f %f\n", 
                point_out[0], point_out[1], point_out[2]);
            return;
        }
        q[d] = ((img_ijk[d] - bxf->roi_offset[d])
            - p[d] * bxf->vox_per_rgn[d]) / bxf->vox_per_rgn[d];
    }

#if defined (commentout)
    printf ("p = [%d, %d, %d], q = [%f, %f, %f]\n", 
        p[0], p[1], p[2], q[0], q[1], q[2]);
#endif

    /* Compute basis function values for this offset */
    for (d = 0; d < 3; d++) {
        float t3 = q[d]*q[d]*q[d];
        float t2 = q[d]*q[d];
        float t1 = q[d];
        q_mini[d][0] = (1.0/6.0) * (- 1.0 * t3 + 3.0 * t2 - 3.0 * t1 + 1.0);
        q_mini[d][1] = (1.0/6.0) * (+ 3.0 * t3 - 6.0 * t2            + 4.0);
        q_mini[d][2] = (1.0/6.0) * (- 3.0 * t3 + 3.0 * t2 + 3.0 * t1 + 1.0);
        q_mini[d][3] = (1.0/6.0) * (+ 1.0 * t3);
    }

    /* Compute displacement vector and add to point_out */
#if defined (commentout)
    printf ("---\n");
#endif
    for (k = 0; k < 4; k++) {
        for (j = 0; j < 4; j++) {
            for (i = 0; i < 4; i++) {
                float ql;
                int cidx;

                cidx = (p[2] + k) * bxf->cdims[1] * bxf->cdims[0]
                    + (p[1] + j) * bxf->cdims[0]
                    + (p[0] + i);
                cidx = cidx * 3;
                ql = q_mini[0][i] * q_mini[1][j] * q_mini[2][k];

#if defined (commentout)
                printf ("(%f) + [%f] + [%f] = ", point_out[0],
                    ql, bxf->coeff[cidx+0]);
#endif

                point_out[0] += ql * bxf->coeff[cidx+0];
                point_out[1] += ql * bxf->coeff[cidx+1];
                point_out[2] += ql * bxf->coeff[cidx+2];

#if defined (commentout)
                printf (" = (%f)\n", point_out[0]);
#endif
            }
        }
    }
}
