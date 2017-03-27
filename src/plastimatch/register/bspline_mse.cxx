/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif
#if (SSE2_FOUND)
#include <xmmintrin.h>
#endif

#include "bspline.h"
#include "bspline_correspond.h"
#include "bspline_cuda.h"
#include "bspline_interpolate.h"
#include "bspline_loop.txx"
#include "bspline_macros.h"
#include "bspline_mse.h"
#include "bspline_mse.txx"
#include "bspline_optimize.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "file_util.h"
#include "interpolate.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "mha_io.h"
#include "plm_math.h"
#include "plm_timer.h"
#include "string_util.h"
#include "volume.h"
#include "volume_macros.h"

void
bspline_score_normalize (
    Bspline_optimize *bod,
    double raw_score
)
{
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();
    Bspline_score *ssd = &bst->ssd;

    const int MIN_VOX = 1;

    /* GCS FIX: This is partly correct.  It would be better if 
       I could set the smetric to slightly more than the previous 
       best score.  By setting to FLT_MAX, it causes the optimizer 
       to exit prematurely.  
       However, the best score is not currently stored in the state.  
    */
    if (ssd->curr_num_vox < MIN_VOX) {
        ssd->curr_smetric = FLT_MAX;
        for (int i = 0; i < bxf->num_coeff; i++) {
            ssd->curr_smetric_grad[i] = 0;
        }
    } else {
        ssd->curr_smetric = raw_score / ssd->curr_num_vox;
        for (int i = 0; i < bxf->num_coeff; i++) {
            ssd->curr_smetric_grad[i] 
                = 2 * ssd->curr_smetric_grad[i] / ssd->curr_num_vox;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION: bspline_score_i_mse()
//
// This is a multi-CPU implementation using OpenMP, using the tile 
// "condense" method.  It is similar to flavor "g", but respects 
// image rois.
///////////////////////////////////////////////////////////////////////////////
void
bspline_score_i_mse (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;
    Volume *moving_grad = bst->moving_grad;

    Bspline_score* ssd = &bst->ssd;
    double score_tile;

    float* f_img = (float*)fixed->img;
    float* m_img = (float*)moving->img;
    float* m_grad = (float*)moving_grad->img;

    int idx_tile;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

    Volume* fixed_roi  = bst->fixed_roi;
    Volume* moving_roi = bst->moving_roi;

    static int it = 0;

    FILE* corr_fp = 0;

    if (parms->debug) {
        std::string fn = string_format ("%s/%02d_corr_mse_%03d_%03d.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        corr_fp = plm_fopen (fn.c_str(), "wb");
        it ++;
    }

    // Zero out accumulators
    int num_vox = 0;
    score_tile = 0;
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    // Parallel across tiles
#pragma omp parallel for reduction (+:num_vox,score_tile)
    LOOP_THRU_VOL_TILES (idx_tile, bxf) {
        int rc;

        plm_long ijk_tile[3];
        plm_long ijk_local[3];

        float fxyz[3];
        plm_long fijk[3];
        plm_long idx_fixed;

        float dxyz[3];

        float xyz_moving[3];
        float ijk_moving[3];
        plm_long ijk_moving_floor[3];
        plm_long ijk_moving_round[3];
        plm_long idx_moving_floor;
        plm_long idx_moving_round;

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
        COORDS_FROM_INDEX (ijk_tile, idx_tile, bxf->rdims); 

        // Serial through voxels in tile
        LOOP_THRU_TILE_Z (ijk_local, bxf) {
            LOOP_THRU_TILE_Y (ijk_local, bxf) {
                LOOP_THRU_TILE_X (ijk_local, bxf) {

                    // Construct coordinates into fixed image volume
                    GET_VOL_COORDS (fijk, ijk_tile, ijk_local, bxf);

                    // Make sure we are inside the image volume
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0])
                        continue;
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1])
                        continue;
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2])
                        continue;

                    // Compute physical coordinates of fixed image voxel
                    POSITION_FROM_COORDS (fxyz, fijk, bxf->img_origin, 
                        fixed->step);

                    /* JAS 2012.03.26: Tends to break the optimizer (PGTOL)   */
                    /* Check to make sure the indices are valid (inside roi) */
                    if (fixed_roi) {
                        if (!inside_roi (fxyz, fixed_roi)) continue;
                    }

                    // Construct the image volume index
                    idx_fixed = volume_index (fixed->dim, fijk);

                    // Calc. deformation vector (dxyz) for voxel
                    bspline_interp_pix_c (dxyz, bxf, idx_tile, ijk_local);

                    // Calc. moving image coordinate from the deformation 
                    // vector
                    rc = bspline_find_correspondence_dcos_roi (
                        xyz_moving, ijk_moving, fxyz, dxyz, moving,
                        moving_roi);

                    if (parms->debug) {
                        fprintf (corr_fp, 
                            "%d %d %d %f %f %f\n",
                            (unsigned int) fijk[0], 
                            (unsigned int) fijk[1], 
                            (unsigned int) fijk[2], 
                            ijk_moving[0], ijk_moving[1], ijk_moving[2]);
                    }

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    // Compute linear interpolation fractions
                    li_clamp_3d (
                        ijk_moving,
                        ijk_moving_floor,
                        ijk_moving_round,
                        li_1,
                        li_2,
                        moving
                    );

                    // Find linear indices for moving image
                    idx_moving_floor = volume_index (
                        moving->dim, ijk_moving_floor);
                    idx_moving_round = volume_index (
                        moving->dim, ijk_moving_round);

                    // Calc. moving voxel intensity via linear interpolation
                    LI_VALUE (
                        m_val, 
                        li_1[0], li_2[0],
                        li_1[1], li_2[1],
                        li_1[2], li_2[2],
                        idx_moving_floor,
                        m_img, moving
                    );

                    // Compute intensity difference
                    diff = m_val - f_img[idx_fixed];

                    // Store the score!
                    score_tile += diff * diff;
                    num_vox++;

                    // Compute dc_dv
                    dc_dv[0] = diff * m_grad[3 * idx_moving_round + 0];
                    dc_dv[1] = diff * m_grad[3 * idx_moving_round + 1];
                    dc_dv[2] = diff * m_grad[3 * idx_moving_round + 2];

                    /* Generate condensed tile */
                    bspline_update_sets_b (
                        sets_x, sets_y, sets_z,
                        ijk_local, dc_dv, bxf
                    );

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */


        // The tile is now condensed.  Now we will put it in the
        // proper slot within the control point bin that it belong to.
        bspline_sort_sets (
            cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            idx_tile, bxf
        );

    } /* LOOP_THRU_VOL_TILES */

    ssd->curr_num_vox = num_vox;

    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result with be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z, bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

    /* Normalize score for MSE */
    bspline_score_normalize (bod, score_tile);

    if (parms->debug) {
        fclose (corr_fp);
    }
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION: bspline_score_h_mse()
//
// This is a single core CPU implementation of CUDA implementation J.
// The tile "condense" method is demonstrated.
//
// ** This is the fastest know CPU implmentation for single core **
//
// See also:
//   OpenMP implementation of CUDA J: bspline_score_g_mse()
//
// AUTHOR: James A. Shackleford
// DATE: 11.22.2009
///////////////////////////////////////////////////////////////////////////////
void
bspline_score_h_mse (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;
    Volume *moving_grad = bst->moving_grad;

    Bspline_score* ssd = &bst->ssd;
    double score_tile;

    float* f_img = (float*)fixed->img;
    float* m_img = (float*)moving->img;
    float* m_grad = (float*)moving_grad->img;

    plm_long idx_tile;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

    static int it = 0;

    // Zero out accumulators
    score_tile = 0;
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    FILE* corr_fp = 0;

    if (parms->debug) {
        std::string fn = string_format ("%s/%02d_corr_mse_%03d_%03d.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        corr_fp = plm_fopen (fn.c_str(), "wb");
        it ++;
    }

    // Serial across tiles
    LOOP_THRU_VOL_TILES (idx_tile, bxf) {
        int rc;

        int ijk_tile[3];
        plm_long ijk_local[3];

        float fxyz[3];
        plm_long fijk[3];
        plm_long idx_fixed;

        float dxyz[3];

        float xyz_moving[3];
        float ijk_moving[3];
        plm_long ijk_moving_floor[3];
        plm_long ijk_moving_round[3];
        plm_long idx_moving_floor;
        plm_long idx_moving_round;

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
        COORDS_FROM_INDEX (ijk_tile, idx_tile, bxf->rdims); 

        // Serial through voxels in tile
        LOOP_THRU_TILE_Z (ijk_local, bxf) {
            LOOP_THRU_TILE_Y (ijk_local, bxf) {
                LOOP_THRU_TILE_X (ijk_local, bxf) {

                    // Construct coordinates into fixed image volume
                    GET_VOL_COORDS (fijk, ijk_tile, ijk_local, bxf);

                    // Make sure we are inside the region of interest
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0])
                        continue;
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1])
                        continue;
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2])
                        continue;

                    // Compute physical coordinates of fixed image voxel
                    POSITION_FROM_COORDS (fxyz, fijk, bxf->img_origin, 
                        fixed->step);

                    // Construct the image volume index
                    idx_fixed = volume_index (fixed->dim, fijk);

                    // Calc. deformation vector (dxyz) for voxel
                    bspline_interp_pix_c (dxyz, bxf, idx_tile, ijk_local);

                    // Calc. moving image coordinate from the deformation vector
                    /* To remove DCOS support, change function call to 
                       bspline_find_correspondence() */
                    rc = bspline_find_correspondence_dcos (
                        xyz_moving, ijk_moving, fxyz, dxyz, moving);

                    // Return code is 0 if voxel is pushed outside of moving image
                    if (!rc) continue;

                    if (parms->debug) {
                        fprintf (corr_fp, 
                            "%d %d %d %f %f %f\n",
                            (unsigned int) fijk[0], 
                            (unsigned int) fijk[1], 
                            (unsigned int) fijk[2], 
                            ijk_moving[0], ijk_moving[1], ijk_moving[2]);
                    }

                    // Compute linear interpolation fractions
                    li_clamp_3d (
                        ijk_moving,
                        ijk_moving_floor,
                        ijk_moving_round,
                        li_1,
                        li_2,
                        moving
                    );

                    // Find linear indices for moving image
                    idx_moving_floor = volume_index (moving->dim, ijk_moving_floor);
                    idx_moving_round = volume_index (moving->dim, ijk_moving_round);

                    // Calc. moving voxel intensity via linear interpolation
                    LI_VALUE (
                        m_val, 
                        li_1[0], li_2[0],
                        li_1[1], li_2[1],
                        li_1[2], li_2[2],
                        idx_moving_floor,
                        m_img, moving
                    );

                    // Compute intensity difference
                    diff = m_val - f_img[idx_fixed];

                    // Store the score!
                    score_tile += diff * diff;
                    ssd->curr_num_vox++;

                    // Compute dc_dv
                    dc_dv[0] = diff * m_grad[3 * idx_moving_round + 0];
                    dc_dv[1] = diff * m_grad[3 * idx_moving_round + 1];
                    dc_dv[2] = diff * m_grad[3 * idx_moving_round + 2];

                    /* Generate condensed tile */
                    bspline_update_sets_b (sets_x, sets_y, sets_z,
                        ijk_local, dc_dv, bxf);

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */

        // The tile is now condensed.  Now we will put it in the
        // proper slot within the control point bin that it belong to.
        bspline_sort_sets (
            cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            idx_tile, bxf
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

    /* Normalize score for MSE */
    bspline_score_normalize (bod, score_tile);

    if (parms->debug) {
        fclose (corr_fp);
    }
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION: bspline_score_g_mse()
//
// This is a multi-CPU implementation of CUDA implementation J.  OpenMP is
// used.  The tile "condense" method is demonstrated.
//
// ** This is the fastest know CPU implmentation for multi core **
//    (That does not require SSE)
//
// AUTHOR: James A. Shackleford
// DATE: 11.22.2009
//
// 2012-06-10 (GCS): Updated to DCOS, only 0.15% increase in runtime, 
//   judged not worth maintaining separate code.
///////////////////////////////////////////////////////////////////////////////
void
bspline_score_g_mse (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;
    Volume *moving_grad = bst->moving_grad;

    Bspline_score* ssd = &bst->ssd;
    double score_tile;

    float* f_img = (float*)fixed->img;
    float* m_img = (float*)moving->img;
    float* m_grad = (float*)moving_grad->img;

    int idx_tile;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

    static int it = 0;

    FILE* corr_fp = 0;

    if (parms->debug) {
        std::string fn = string_format ("%s/%02d_corr_mse_%03d_%03d.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        corr_fp = plm_fopen (fn.c_str(), "wb");
        it ++;
    }

    // Zero out accumulators
    int num_vox = 0;
    score_tile = 0;
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    // Parallel across tiles
#pragma omp parallel for reduction (+:num_vox,score_tile)
    LOOP_THRU_VOL_TILES (idx_tile, bxf) {
        int rc;

        plm_long ijk_tile[3];
        plm_long ijk_local[3];

        float fxyz[3];
        plm_long fijk[3];
        plm_long idx_fixed;

        float dxyz[3];

        float xyz_moving[3];
        float ijk_moving[3];
        plm_long ijk_moving_floor[3];
        plm_long ijk_moving_round[3];
        plm_long idx_moving_floor;
        plm_long idx_moving_round;

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
        COORDS_FROM_INDEX (ijk_tile, idx_tile, bxf->rdims); 

        // Serial through voxels in tile
        LOOP_THRU_TILE_Z (ijk_local, bxf) {
            LOOP_THRU_TILE_Y (ijk_local, bxf) {
                LOOP_THRU_TILE_X (ijk_local, bxf) {

                    // Construct coordinates into fixed image volume
                    GET_VOL_COORDS (fijk, ijk_tile, ijk_local, bxf);

                    // Make sure we are inside the image volume
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0])
                        continue;
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1])
                        continue;
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2])
                        continue;

                    // Compute physical coordinates of fixed image voxel
                    POSITION_FROM_COORDS (fxyz, fijk, bxf->img_origin, 
                        fixed->step);
                    
                    // Construct the image volume index
                    idx_fixed = volume_index (fixed->dim, fijk);

                    // Calc. deformation vector (dxyz) for voxel
                    bspline_interp_pix_c (dxyz, bxf, idx_tile, ijk_local);

                    // Calc. moving image coordinate from the deformation 
                    // vector
                    /* To remove DCOS support, change function call to 
                       bspline_find_correspondence() */
                    rc = bspline_find_correspondence_dcos (
                        xyz_moving, ijk_moving, fxyz, dxyz, moving);

                    if (parms->debug) {
                        fprintf (corr_fp, 
                            "%d %d %d %f %f %f\n",
                            (unsigned int) fijk[0], 
                            (unsigned int) fijk[1], 
                            (unsigned int) fijk[2], 
                            ijk_moving[0], ijk_moving[1], ijk_moving[2]);
                    }

                    // Return code is 0 if voxel is pushed outside of 
                    // moving image
                    if (!rc) continue;

                    // Compute linear interpolation fractions
                    li_clamp_3d (
                        ijk_moving,
                        ijk_moving_floor,
                        ijk_moving_round,
                        li_1,
                        li_2,
                        moving
                    );

                    // Find linear indices for moving image
                    idx_moving_floor = volume_index (
                        moving->dim, ijk_moving_floor);
                    idx_moving_round = volume_index (
                        moving->dim, ijk_moving_round);

                    // Calc. moving voxel intensity via linear interpolation
                    LI_VALUE (
                        m_val, 
                        li_1[0], li_2[0],
                        li_1[1], li_2[1],
                        li_1[2], li_2[2],
                        idx_moving_floor,
                        m_img, moving
                    );

                    // Compute intensity difference
                    diff = m_val - f_img[idx_fixed];

                    // Store the score!
                    score_tile += diff * diff;
                    num_vox++;

                    // Compute dc_dv
                    dc_dv[0] = diff * m_grad[3 * idx_moving_round + 0];
                    dc_dv[1] = diff * m_grad[3 * idx_moving_round + 1];
                    dc_dv[2] = diff * m_grad[3 * idx_moving_round + 2];

                    /* Generate condensed tile */
                    bspline_update_sets_b (
                        sets_x, sets_y, sets_z,
                        ijk_local, dc_dv, bxf
                    );

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */


        // The tile is now condensed.  Now we will put it in the
        // proper slot within the control point bin that it belong to.
        bspline_sort_sets (
            cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            idx_tile, bxf
        );

    } /* LOOP_THRU_VOL_TILES */

    ssd->curr_num_vox = num_vox;

    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result with be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z, bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

    /* Normalize score for MSE */
    bspline_score_normalize (bod, score_tile);

    if (parms->debug) {
        fclose (corr_fp);
    }
}

/* -----------------------------------------------------------------------
   FUNCTION: bspline_score_c_mse()

   This is the older "fast" single-threaded MSE implementation, modified 
   to respect direction cosines (and ROI support removed).
   ----------------------------------------------------------------------- */
void
bspline_score_c_mse (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;
    Volume *moving_grad = bst->moving_grad;

    Bspline_score* ssd = &bst->ssd;
    plm_long fijk[3], fv;         /* Indices within fixed image (vox) */
    float mijk[3];              /* Indices within moving image (vox) */
    float fxyz[3];              /* Position within fixed image (mm) */
    float mxyz[3];              /* Position within moving image (mm) */
    plm_long mijk_f[3], mvf;      /* Floor */
    plm_long mijk_r[3], mvr;      /* Round */
    plm_long p[3], pidx;          /* Region index of fixed voxel */
    plm_long q[3], qidx;          /* Offset index of fixed voxel */

    float dc_dv[3];
    float li_1[3];           /* Fraction of interpolant in lower index */
    float li_2[3];           /* Fraction of interpolant in upper index */
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    float* m_grad = (float*) moving_grad->img;
    float dxyz[3];
    float m_val;

    /* GCS: Oct 5, 2009.  We have determined that sequential accumulation
       of the score requires double precision.  However, reduction 
       accumulation does not. */
    double score_acc = 0.;

    static int it = 0;
    FILE* val_fp = 0;
    FILE* dc_dv_fp = 0;
    FILE* corr_fp = 0;

    if (parms->debug) {
        std::string fn;

        fn = string_format ("%s/%02d_dc_dv_mse_%03d_%03d.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        dc_dv_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_val_mse_%03d_%03d.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        val_fp = plm_fopen (fn.c_str(), "wb");

        fn = string_format ("%s/%02d_corr_mse_%03d_%03d.csv",
            parms->debug_dir.c_str(), parms->debug_stage, bst->it, 
            bst->feval);
        corr_fp = plm_fopen (fn.c_str(), "wb");
        it ++;
    }

    /* GCS FIX: region of interest is not used */
    LOOP_Z (fijk, fxyz, fixed) {
        p[2] = REGION_INDEX_Z (fijk, bxf);
        q[2] = REGION_OFFSET_Z (fijk, bxf);
        LOOP_Y (fijk, fxyz, fixed) {
            p[1] = REGION_INDEX_Y (fijk, bxf);
            q[1] = REGION_OFFSET_Y (fijk, bxf);
            LOOP_X (fijk, fxyz, fixed) {
                p[0] = REGION_INDEX_X (fijk, bxf);
                q[0] = REGION_OFFSET_X (fijk, bxf);

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                qidx = volume_index (bxf->vox_per_rgn, q);
                bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

                /* Compute moving image coordinate of fixed image voxel */
                mxyz[2] = fxyz[2] + dxyz[2] - moving->origin[2];
                mxyz[1] = fxyz[1] + dxyz[1] - moving->origin[1];
                mxyz[0] = fxyz[0] + dxyz[0] - moving->origin[0];
                mijk[2] = PROJECT_Z (mxyz, moving->proj);
                mijk[1] = PROJECT_Y (mxyz, moving->proj);
                mijk[0] = PROJECT_X (mxyz, moving->proj);

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

                if (!moving->is_inside (mijk)) continue;

                /* Compute interpolation fractions */
                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of "corner voxel" in moving image */
                mvf = volume_index (moving->dim, mijk_f);

                /* Compute moving image intensity using linear interpolation */
                /* Macro is slightly faster than function */
                LI_VALUE (m_val, 
                    li_1[0], li_2[0],
                    li_1[1], li_2[1],
                    li_1[2], li_2[2],
                    mvf, m_img, moving);

                /* Compute linear index of fixed image voxel */
                fv = volume_index (fixed->dim, fijk);

                /* Compute intensity difference */
                float diff = m_val - f_img[fv];

                /* Compute spatial gradient using nearest neighbors */
                mvr = volume_index (moving->dim, mijk_r);
                dc_dv[0] = diff * m_grad[3*mvr+0];  /* x component */
                dc_dv[1] = diff * m_grad[3*mvr+1];  /* y component */
                dc_dv[2] = diff * m_grad[3*mvr+2];  /* z component */
                bst->ssd.update_smetric_grad_b (bxf, pidx, qidx, dc_dv);
        
                if (parms->debug) {
                    fprintf (val_fp, 
                        "%u %u %u %g %g %g\n", 
                        (unsigned int) fijk[0], 
                        (unsigned int) fijk[1], 
                        (unsigned int) fijk[2], 
                        f_img[fv], m_val, diff);
                    fprintf (dc_dv_fp, 
                        "%u %u %u %g %g %g %g\n", 
                        (unsigned int) fijk[0], 
                        (unsigned int) fijk[1], 
                        (unsigned int) fijk[2], 
                        diff, 
                        dc_dv[0], dc_dv[1], dc_dv[2]);
                }

                score_acc += diff * diff;
                ssd->curr_num_vox++;

            } /* LOOP_THRU_ROI_X */
        } /* LOOP_THRU_ROI_Y */
    } /* LOOP_THRU_ROI_Z */

    if (parms->debug) {
        fclose (val_fp);
        fclose (dc_dv_fp);
        fclose (corr_fp);
    }

    /* Normalize score for MSE */
    bspline_score_normalize (bod, score_acc);
}

/* -----------------------------------------------------------------------
   FUNCTION: bspline_score_k_mse(), bspline_score_l_mse()

   This is the same as 'c', except using templates.

   This is the older "fast" single-threaded MSE implementation, modified 
   to respect direction cosines (and ROI support removed).
   ----------------------------------------------------------------------- */
void
bspline_score_k_mse (
    Bspline_optimize *bod
)
{
    /* Create/initialize bspline_loop_user */
    Bspline_mse_k blu (bod);

    /* Run the loop */
    bspline_loop_voxel_serial (blu, bod);

    /* Normalize score for MSE */
    bspline_score_normalize (bod, blu.score_acc);
}

void
bspline_score_l_mse (
    Bspline_optimize *bod
)
{
    /* Create/initialize bspline_loop_user */
    Bspline_mse_l blu (bod);

    /* Run the loop */
    bspline_loop_tile_serial (blu, bod);

    /* Normalize score for MSE */
    bspline_score_normalize (bod, blu.score_acc);
}

void
bspline_score_m_mse (
    Bspline_optimize *bod
)
{
}

void
bspline_score_n_mse (
    Bspline_optimize *bod
)
{
}

void
bspline_score_mse (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();

    Volume* fixed_roi  = bst->fixed_roi;
    Volume* moving_roi = bst->moving_roi;
    bool have_roi = fixed_roi || moving_roi;

    /* CPU Implementations */
    if (parms->threading == BTHR_CPU)
    {
        if (have_roi) {
            switch (parms->implementation) {
            case 'c':
            case 'k':
                bspline_score_k_mse (bod);
            break;
            default:
                bspline_score_i_mse (bod);
                break;
            }
        } else {
            switch (parms->implementation) {
            case 'c':
                bspline_score_c_mse (bod);
                break;
            case 'g':
                bspline_score_g_mse (bod);
                break;
            case 'h':
                bspline_score_h_mse (bod);
                break;
            case 'i':
                bspline_score_i_mse (bod);
                break;
            case 'k':
                bspline_score_k_mse (bod);
                break;
            case 'l':
                bspline_score_l_mse (bod);
                break;
            case 'm':
                bspline_score_m_mse (bod);
                break;
            case 'n':
                bspline_score_n_mse (bod);
                break;
            default:
#if (OPENMP_FOUND)
                bspline_score_g_mse (bod);
#else
                bspline_score_h_mse (bod);
#endif
                break;
            }
        }
    }

#if (CUDA_FOUND)
    /* CUDA Implementations */
    else if (parms->threading == BTHR_CUDA)
    {
        /* Be sure we loaded the CUDA plugin */
        LOAD_LIBRARY_SAFE (libplmregistercuda);
        LOAD_SYMBOL (CUDA_bspline_mse_j, libplmregistercuda);

        switch (parms->implementation) {
        case 'j':
            CUDA_bspline_mse_j (
                bod->get_bspline_parms(),
                bod->get_bspline_state(),
                bod->get_bspline_xform());
            break;
        default:
            CUDA_bspline_mse_j (
                bod->get_bspline_parms(),
                bod->get_bspline_state(),
                bod->get_bspline_xform());
            break;
        }

        /* Unload plugin when done */
        UNLOAD_LIBRARY (libplmregistercuda);
    }
#endif
}
