/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif
#if (SSE2_FOUND)
#include <xmmintrin.h>
#endif

#include "bspline_correspond.h"
#include "bspline_interpolate.h"
#include "bspline_macros.h"
#include "bspline_warp.h"
#include "interpolate.h"
#include "interpolate_macros.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "volume_macros.h"
#include "volume.h"

/* This only warps voxels within the ROI.  If you need the whole 
   image, call bspline_xform_extend. */
template <class T, int PIX_TYPE>
void
bspline_warp_dcos (
    Volume *vout,       /* Output image (sized and allocated) */
    Volume *vf_out,     /* Output vf (sized and allocated, can be null) */
    Bspline_xform* bxf, /* Bspline transform coefficients */
    Volume *moving,     /* Input image */
    int linear_interp,  /* 1 = trilinear, 0 = nearest neighbors */
    T default_val       /* Fill in this value outside of image */
)
{
    T* vout_img = (T*) vout->img;
    T* m_img = (T*) moving->img;

    /* A few sanity checks */
    if (vout->pix_type != PIX_TYPE) {
        print_and_exit ("Error: bspline_warp pix type mismatch\n");
        return;
    }
    for (int d = 0; d < 3; d++) {
        if (vout->dim[d] != bxf->img_dim[d]) {
            print_and_exit ("Error: bspline_warp dim mismatch\n");
            return;
        }
        if (vout->origin[d] != bxf->img_origin[d]) {
            print_and_exit ("Error: bspline_warp origin mismatch\n");
            return;
        }
        if (vout->spacing[d] != bxf->img_spacing[d]) {
            print_and_exit ("Error: bspline_warp pix spacing mismatch\n");
            return;
        }
    }
    if (vf_out && vf_out->pix_type != PT_VF_FLOAT_INTERLEAVED) {
        print_and_exit ("Error: bspline_warp requires interleaved vf\n");
        return;
    }

    /* Set default */
    for (plm_long vidx = 0; vidx < vout->npix; vidx++) {
        vout_img[vidx] = default_val;
    }
    if (vf_out) {
        memset (vf_out->img, 0, vf_out->pix_size * vf_out->npix);
    }

#pragma omp parallel for 
    LOOP_Z_OMP (k, vout) {
        plm_long fijk[3];      /* Index within fixed image (vox) */
        float fxyz[3];         /* Position within fixed image (mm) */
        plm_long p[3];
        plm_long q[3];
        plm_long pidx, qidx;
        float dxyz[3];

        fijk[2] = k;
        fxyz[2] = vout->origin[2] + fijk[2] * vout->step[2*3+2];
        p[2] = REGION_INDEX_Z (fijk, bxf);
        q[2] = REGION_OFFSET_Z (fijk, bxf);
        LOOP_Y (fijk, fxyz, vout) {
            p[1] = REGION_INDEX_Y (fijk, bxf);
            q[1] = REGION_OFFSET_Y (fijk, bxf);
            LOOP_X (fijk, fxyz, vout) {
                plm_long fv;     /* Linear index within fixed image (vox) */
                float mxyz[3];   /* Position within moving image (mm) */
                float mijk[3];   /* Index within moving image (vox) */
                plm_long mijk_f[3]; /* Floor index within moving image (vox) */
                plm_long mijk_r[3]; /* Round index within moving image (vox) */
                plm_long mvf;    /* Floor linear index within moving image */
                float li_1[3];   /* Fraction of interpolant in lower index */
                float li_2[3];   /* Fraction of interpolant in upper index */
                p[0] = REGION_INDEX_X (fijk, bxf);
                q[0] = REGION_OFFSET_X (fijk, bxf);

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                qidx = volume_index (bxf->vox_per_rgn, q);
                bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

                /* Compute linear index of fixed image voxel */
                fv = volume_index (vout->dim, fijk);

                /* Assign deformation */
                if (vf_out) {
                    float *vf_out_img = (float*) vf_out->img;
                    vf_out_img[3*fv+0] = dxyz[0];
                    vf_out_img[3*fv+1] = dxyz[1];
                    vf_out_img[3*fv+2] = dxyz[2];
                }

                /* Compute moving image coordinate of fixed image voxel */
                mxyz[2] = fxyz[2] + dxyz[2] - moving->origin[2];
                mxyz[1] = fxyz[1] + dxyz[1] - moving->origin[1];
                mxyz[0] = fxyz[0] + dxyz[0] - moving->origin[0];
                mijk[2] = PROJECT_Z (mxyz, moving->proj);
                mijk[1] = PROJECT_Y (mxyz, moving->proj);
                mijk[0] = PROJECT_X (mxyz, moving->proj);

                if (!moving->is_inside (mijk)) continue;

                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                if (linear_interp) {
                    /* Find linear index of "corner voxel" in moving image */
                    mvf = volume_index (moving->dim, mijk_f);

                    /* Macro is slightly faster than function */
                    /* Compute moving image intensity using linear 
                       interpolation */
                    T m_val;
                    LI_VALUE (m_val, 
                        li_1[0], li_2[0],
                        li_1[1], li_2[1],
                        li_1[2], li_2[2],
                        mvf, m_img, moving);

                    /* Assign warped value to output image */
                    vout_img[fv] = m_val;

                } else {
                    /* Find linear index of "nearest voxel" in moving image */
                    mvf = volume_index (moving->dim, mijk_r);

                    /* Loop through planes */
                    /* Note: We omit looping through planes when linear 
                       interpolation is enabled, with the understanding 
                       that this is only used for warping structure sets */
                    for (int plane = 0; plane < moving->vox_planes; plane++)
                    {
                        /* Get moving image value */
                        T m_val;
                        m_val = m_img[mvf*moving->vox_planes+plane];

                        /* Assign to output image */
                        vout_img[fv*moving->vox_planes+plane] = m_val;
                    }
                }
            }
        }
    }
    printf ("bspline_warp complete.\n");
}

void
bspline_warp (
    Volume *vout,       /* Output image (sized and allocated) */
    Volume *vf_out,     /* Output vf (sized and allocated, can be null) */
    Bspline_xform* bxf, /* Bspline transform coefficients */
    Volume *moving,     /* Input image */
    int linear_interp,  /* 1 = trilinear, 0 = nearest neighbors */
    float default_val   /* Fill in this value outside of image */
)
{
    switch (moving->pix_type)
    {
    case PT_UCHAR:
        bspline_warp_dcos<unsigned char, PT_UCHAR> (
            vout, vf_out, bxf, moving, linear_interp, default_val);
        break;
    case PT_SHORT:
        bspline_warp_dcos<short, PT_SHORT> (
            vout, vf_out, bxf, moving, linear_interp, default_val);
        break;
    case PT_UINT16:
        bspline_warp_dcos<uint16_t, PT_UINT16> (
            vout, vf_out, bxf, moving, linear_interp, default_val);
        break;
    case PT_UINT32:
        bspline_warp_dcos<uint32_t, PT_UINT32> (
            vout, vf_out, bxf, moving, linear_interp, default_val);
        break;
    case PT_FLOAT:
        bspline_warp_dcos<float, PT_FLOAT> (
            vout, vf_out, bxf, moving, linear_interp, default_val);
        break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
        print_and_exit ("bspline_warp: sorry, this is not supported.\n");
        break;
    case PT_UCHAR_VEC_INTERLEAVED:
        bspline_warp_dcos<unsigned char, PT_UCHAR_VEC_INTERLEAVED> (
            vout, vf_out, bxf, moving, linear_interp, default_val);
        break;
    default:
        print_and_exit ("bspline_warp: sorry, this is not supported.\n");
        break;
    }
}

void
bspline_warp (
    Volume *vout,       /* Output image (sized and allocated) */
    Volume *vf_out,     /* Output vf (sized and allocated, can be null) */
    Bspline_xform* bxf, /* Bspline transform coefficients */
    const Volume::Pointer& moving,       /* Input image */
    int linear_interp,  /* 1 = trilinear, 0 = nearest neighbors */
    float default_val   /* Fill in this value outside of image */
)
{
    bspline_warp (vout, vf_out, bxf, moving.get(), linear_interp, default_val);
}
