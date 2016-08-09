/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_loop_txx_
#define _bspline_loop_txx_

#include "plmregister_config.h"
#include <string>
#if OPENMP_FOUND
#include <omp.h>
#endif

#include "bspline_macros.h"
#include "bspline_mse.h"
#include "bspline_optimize.h"
#include "bspline_parms.h"
#include "file_util.h"
#include "interpolate.h"
#include "interpolate_macros.h"
#include "plm_timer.h"
#include "string_util.h"

/* -----------------------------------------------------------------------
   B-Spline registration.
   Reference implementation, meant to be easy to understand.
   Equivalent to "MSE C" method.
   Respects direction cosines and ROI images.
   ----------------------------------------------------------------------- */
template< class Bspline_loop_user >
void
bspline_loop_voxel_serial (
    Bspline_loop_user& bspline_loop_user,
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();
    Bspline_score* ssd = &bst->ssd;

    Volume *fixed = parms->fixed;
    Volume *moving = parms->moving;
    Volume *fixed_roi  = parms->fixed_roi;
    Volume *moving_roi = parms->moving_roi;

    plm_long fijk[3], fidx;     /* Indices within fixed image (vox) */
    float mijk[3];              /* Indices within moving image (vox) */
    float fxyz[3];              /* Position within fixed image (mm) */
    float mxyz[3];              /* Position within moving image (mm) */
    plm_long mijk_f[3], midx_f; /* Floor */
    plm_long mijk_r[3];         /* Round */
    plm_long p[3], pidx;        /* Region index of fixed voxel */
    plm_long q[3], qidx;        /* Offset index of fixed voxel */

    float li_1[3];           /* Fraction of interpolant in lower index */
    float li_2[3];           /* Fraction of interpolant in upper index */
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    float dxyz[3];

    FILE* val_fp = 0;
    FILE* dc_dv_fp = 0;
    FILE* corr_fp = 0;
    if (parms->debug) {
        std::string fn;

        fn = string_format ("%s/%02d_%03d_%03d_dc_dv.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        dc_dv_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_%03d_%03d_val.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        val_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_%03d_%03d_corr.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        corr_fp = plm_fopen (fn.c_str(), "wb");
    }
    
    LOOP_Z (fijk, fxyz, fixed) {
        p[2] = REGION_INDEX_Z (fijk, bxf);
        q[2] = REGION_OFFSET_Z (fijk, bxf);
        LOOP_Y (fijk, fxyz, fixed) {
            p[1] = REGION_INDEX_Y (fijk, bxf);
            q[1] = REGION_OFFSET_Y (fijk, bxf);
            LOOP_X (fijk, fxyz, fixed) {
                p[0] = REGION_INDEX_X (fijk, bxf);
                q[0] = REGION_OFFSET_X (fijk, bxf);

                /* Discard fixed image voxels outside of roi */
                if (fixed_roi) {
                    if (!inside_roi (fxyz, fixed_roi)) continue;
                }

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                qidx = volume_index (bxf->vox_per_rgn, q);
                bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

                /* Find correspondence in moving image */
                int rc;
                rc = bspline_find_correspondence_dcos_roi (
                    mxyz, mijk, fxyz, dxyz, moving, moving_roi);

                /* If voxel is not inside moving image */
                if (!rc) continue;

                if (parms->debug) {
                    fprintf (corr_fp, 
                        "%d %d %d, %f %f %f -> %f %f %f, %f %f %f\n",
                        (unsigned int) fijk[0], 
                        (unsigned int) fijk[1], 
                        (unsigned int) fijk[2], 
                        fxyz[0], fxyz[1], fxyz[2],
                        mijk[0], mijk[1], mijk[2],
                        fxyz[0] + dxyz[0], fxyz[1] + dxyz[1], fxyz[2] + dxyz[2]
                    );
                }

                /* Compute interpolation fractions */
                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Compute linear index of fixed image voxel */
                fidx = volume_index (fixed->dim, fijk);

                /* Find linear index of "corner voxel" in moving image */
                midx_f = volume_index (moving->dim, mijk_f);

                /* Run the target function */
                bspline_loop_user.loop_function (
                    bod, bxf, bst, ssd, 
                    fixed, moving, f_img, m_img, 
                    fidx, midx_f, mijk_r, 
                    pidx, qidx, li_1, li_2);
            } /* LOOP_THRU_ROI_X */
        } /* LOOP_THRU_ROI_Y */
    } /* LOOP_THRU_ROI_Z */

    if (parms->debug) {
        fclose (val_fp);
        fclose (dc_dv_fp);
        fclose (corr_fp);
    }
}

/* -----------------------------------------------------------------------
   B-Spline registration.
   Fasted known method for single core CPU.
   Equivalent to "MSE H" method.
   Respects direction cosines and ROI images.
   ----------------------------------------------------------------------- */
template< class Bspline_loop_user >
void
bspline_loop_tile_serial (
    Bspline_loop_user& bspline_loop_user,
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();
    Bspline_score* ssd = &bst->ssd;

    Volume *fixed = parms->fixed;
    Volume *moving = parms->moving;
    Volume *fixed_roi  = parms->fixed_roi;
    Volume *moving_roi = parms->moving_roi;

    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

    // Zero out accumulators
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    FILE* val_fp = 0;
    FILE* dc_dv_fp = 0;
    FILE* corr_fp = 0;
    if (parms->debug) {
        std::string fn;

        fn = string_format ("%s/%02d_%03d_%03d_dc_dv.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        dc_dv_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_%03d_%03d_val.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        val_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_%03d_%03d_corr.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        corr_fp = plm_fopen (fn.c_str(), "wb");
    }

    // Serial across tiles
    plm_long pidx;
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;

        int ijk_tile[3];
        plm_long q[3];

        plm_long fijk[3], fidx;     /* Indices within fixed image (vox) */
        float fxyz[3];              /* Position within fixed image (mm) */
        float mijk[3];              /* Indices within moving image (vox) */
        float mxyz[3];              /* Position within moving image (mm) */
        plm_long mijk_f[3], midx_f; /* Floor */
        plm_long mijk_r[3], midx_r; /* Round */

        float dxyz[3];

        float li_1[3], li_2[3];
        float m_val, diff;
    
        float dc_dv[3];

        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));

        // Get tile coordinates from index
        COORDS_FROM_INDEX (ijk_tile, pidx, bxf->rdims); 

        // Serial through voxels in tile
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {

                    // Construct coordinates into fixed image volume
                    GET_VOL_COORDS (fijk, ijk_tile, q, bxf);

                    /* Discard fixed image voxels outside of roi */
                    if (fixed_roi) {
                        if (!inside_roi (fxyz, fixed_roi)) continue;
                    }

                    // Compute physical coordinates of fixed image voxel
                    /* To remove DCOS support, switch to 
                       GET_REAL_SPACE_COORDS (fxyz, fijk, bxf); */
                    POSITION_FROM_COORDS (fxyz, fijk, bxf->img_origin, 
                        fixed->step);

                    // Construct the image volume index
                    fidx = volume_index (fixed->dim, fijk);

                    // Calc. deformation vector (dxyz) for voxel
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    int rc;
                    rc = bspline_find_correspondence_dcos_roi (
                        mxyz, mijk, fxyz, dxyz, moving, moving_roi);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    if (parms->debug) {
                        fprintf (corr_fp, 
                            "%d %d %d, %f %f %f -> %f %f %f, %f %f %f\n",
                            (unsigned int) fijk[0], 
                            (unsigned int) fijk[1], 
                            (unsigned int) fijk[2], 
                            fxyz[0], fxyz[1], fxyz[2],
                            mijk[0], mijk[1], mijk[2],
                            fxyz[0] + dxyz[0], fxyz[1] + dxyz[1], 
                            fxyz[2] + dxyz[2]);
                    }

                    // Compute linear interpolation fractions
                    li_clamp_3d (
                        mijk,
                        mijk_f,
                        mijk_r,
                        li_1,
                        li_2,
                        moving
                    );

                    // Find linear indices for moving image
                    midx_f = volume_index (moving->dim, mijk_f);
                    midx_r = volume_index (moving->dim, mijk_r);

                    /* Run the target function */
                    bspline_loop_user.loop_function (
                        bod, bxf, bst, ssd, 
                        fixed, moving, f_img, m_img, 
                        fidx, midx_f, mijk_r, 
                        li_1, li_2, q, 
                        sets_x, sets_y, sets_z);

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */

        // The tile is now condensed.  Now we will put it in the
        // proper slot within the control point bin that it belong to.
        bspline_sort_sets (
            cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf
        );

    } /* LOOP_THRU_VOL_TILES */


    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  A single bin summation is
     * dc_dp for the single control point associated with the bin.
     * The number of total bins is equal to the number of control
     * points in the control grid.
     */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z, bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

    if (parms->debug) {
        fclose (val_fp);
        fclose (dc_dv_fp);
        fclose (corr_fp);
    }
}

/* -----------------------------------------------------------------------
   B-Spline registration.
   Fasted known method for single core CPU.
   Equivalent to "MSE I" method.
   Respects direction cosines and ROI images.
   ----------------------------------------------------------------------- */
#if OPENMP_FOUND
template< class Bspline_loop_user >
void
bspline_loop_tile_parallel (
    Bspline_loop_user& bspline_loop_user,
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();
    Bspline_score* ssd = &bst->ssd;

    Volume *fixed = parms->fixed;
    Volume *moving = parms->moving;
    Volume *fixed_roi  = parms->fixed_roi;
    Volume *moving_roi = parms->moving_roi;

    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

    // Zero out accumulators
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    FILE* val_fp = 0;
    FILE* dc_dv_fp = 0;
    FILE* corr_fp = 0;
    if (parms->debug) {
        std::string fn;

        fn = string_format ("%s/%02d_%03d_%03d_dc_dv.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        dc_dv_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_%03d_%03d_val.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        val_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_%03d_%03d_corr.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        corr_fp = plm_fopen (fn.c_str(), "wb");
    }

    /* Allocate and initialize private thread data */
    int num_threads = 1;
#pragma omp parallel
#pragma omp master
    {
        num_threads = omp_get_num_threads ();
    }
    typename Bspline_loop_user::Thread_data thread_data_array 
        = new typename Bspline_loop_user::Thread_data[num_threads];

    // Parallel across tiles
    plm_long pidx;
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {

        /* Get pointer to private data for this thread */
        int thread_num = omp_get_thread_num ();
        typename Bspline_loop_user::Thread_data *thread_data 
            = thread_data_array[thread_num];

        int rc;
        int ijk_tile[3];
        plm_long q[3];

        plm_long fijk[3], fidx;     /* Indices within fixed image (vox) */
        float fxyz[3];              /* Position within fixed image (mm) */
        float mijk[3];              /* Indices within moving image (vox) */
        float mxyz[3];              /* Position within moving image (mm) */
        plm_long mijk_f[3], midx_f; /* Floor */
        plm_long mijk_r[3], midx_r; /* Round */

        float dxyz[3];

        float li_1[3], li_2[3];
        float m_val, diff;
    
        float dc_dv[3];

        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));

        // Get tile coordinates from index
        COORDS_FROM_INDEX (ijk_tile, pidx, bxf->rdims); 

        // Serial through voxels in tile
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {

                    // Construct coordinates into fixed image volume
                    GET_VOL_COORDS (fijk, ijk_tile, q, bxf);

                    /* Discard fixed image voxels outside of roi */
                    if (fixed_roi) {
                        if (!inside_roi (fxyz, fixed_roi)) continue;
                    }

                    // Compute physical coordinates of fixed image voxel
                    /* To remove DCOS support, switch to 
                       GET_REAL_SPACE_COORDS (fxyz, fijk, bxf); */
                    POSITION_FROM_COORDS (fxyz, fijk, bxf->img_origin, 
                        fixed->step);

                    // Construct the image volume index
                    fidx = volume_index (fixed->dim, fijk);

                    // Calc. deformation vector (dxyz) for voxel
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    int rc;
                    rc = bspline_find_correspondence_dcos_roi (
                        mxyz, mijk, fxyz, dxyz, moving, moving_roi);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    if (parms->debug) {
                        fprintf (corr_fp, 
                            "%d %d %d, %f %f %f -> %f %f %f, %f %f %f\n",
                            (unsigned int) fijk[0], 
                            (unsigned int) fijk[1], 
                            (unsigned int) fijk[2], 
                            fxyz[0], fxyz[1], fxyz[2],
                            mijk[0], mijk[1], mijk[2],
                            fxyz[0] + dxyz[0], fxyz[1] + dxyz[1], 
                            fxyz[2] + dxyz[2]);
                    }

                    // Compute linear interpolation fractions
                    li_clamp_3d (
                        mijk,
                        mijk_f,
                        mijk_r,
                        li_1,
                        li_2,
                        moving
                    );

                    // Find linear indices for moving image
                    midx_f = volume_index (moving->dim, mijk_f);
                    midx_r = volume_index (moving->dim, mijk_r);

                    /* Run the target function */
                    bspline_loop_user.loop_function (
                        bod, bxf, bst, ssd, 
                        fixed, moving, f_img, m_img, 
                        fidx, midx_f, mijk_r, 
                        li_1, li_2, q, 
                        sets_x, sets_y, sets_z);

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */

        // The tile is now condensed.  Now we will put it in the
        // proper slot within the control point bin that it belong to.
        bspline_sort_sets (
            cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf
        );

    } /* LOOP_THRU_VOL_TILES */


    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  A single bin summation is
     * dc_dp for the single control point associated with the bin.
     * The number of total bins is equal to the number of control
     * points in the control grid.
     */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z, bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

    if (parms->debug) {
        fclose (val_fp);
        fclose (dc_dv_fp);
        fclose (corr_fp);
    }
}
#endif /* OPENMP_FOUND */

#endif
