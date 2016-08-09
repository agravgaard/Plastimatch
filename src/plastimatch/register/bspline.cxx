/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
    Proposed variable naming guide:
        Fixed image voxel                (f[3]), fidx <currently (fi,fj,fk),fv>
        Moving image voxel               (m[3]), midx < ditto >
            - what about ROI's              ?
            - what about physical coords    ?
        Tile (fixed)                        (t[3]), tidx <currently p[3]>
        Offset within tile (fixed)          (o[3]), oidx <currently q[3]>
        Control point                       (c[3]), cidx <currently (i,j,k), m>
        Coefficient array                   ?                <currently cidx>
        Multiplier LUT                      qidx
        Index LUT                           pidx

    qlut = Multiplier LUT
    clut = Index LUT
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif
#if (OPENMP_FOUND)
#include <omp.h>
#endif
#if (SSE2_FOUND)
#include <xmmintrin.h>
#endif

#include "bspline.h"
#include "bspline_gm.h"
#include "bspline_interpolate.h"
#include "bspline_landmarks.h"
#include "bspline_mi.h"
#include "bspline_mi_hist.h"
#include "bspline_mse.h"
#include "bspline_optimize.h"
#include "bspline_parms.h"
#include "bspline_regularize.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "delayload.h"
#include "file_util.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "plm_math.h"
#include "plm_timer.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"
#include "volume_macros.h"

////////////////////////////////////////////////////////////////////////////////
// FUNCTION: calc_offsets()
//
// This function accepts the number or voxels per control region
// and the dimensions of the control grid to generate where the linear
// memory offsets lie for the beginning of each tile in a 32-byte
// aligned tile-major data organization scheme (such as that which
// is produced by kernel_row_to_tile_major().
//
// Author: James Shackleford
// Data: July 30th, 2009
////////////////////////////////////////////////////////////////////////////////
int* calc_offsets(int* tile_dims, int* cdims)
{
    int vox_per_tile = (tile_dims[0] * tile_dims[1] * tile_dims[2]);
    int pad = 32 - (vox_per_tile % 32);
    int num_tiles = (cdims[0]-3)*(cdims[1]-3)*(cdims[2]-3);

    int* output = (int*)malloc(num_tiles*sizeof(int));

    int i;
    for(i = 0; i < num_tiles; i++)
        output[i] = (vox_per_tile + pad) * i;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION: find_knots()
//
// This function takes a tile index as an input and generates
// the indicies of the 64 control knots that it affects.
//
// Returns:
//    knots[idx] - idx is [0,63] and knots[idx] = the linear knot index
//                 of affected control knot # idx within the entire control
//                 knot grid.
//
//    tile_pos[
//
// Author: James Shackleford
// Data: July 13th, 2009
////////////////////////////////////////////////////////////////////////////////
void find_knots (
    plm_long* knots, 
    plm_long tile_num, 
    plm_long* rdims, 
    plm_long* cdims
) {
    int tile_loc[3];
    int i, j, k;
    int idx = 0;
    int num_tiles_x = cdims[0] - 3;
    int num_tiles_y = cdims[1] - 3;
    int num_tiles_z = cdims[2] - 3;
        
    // First get the [x,y,z] coordinate of
    // the tile in the control grid.
    tile_loc[0] = tile_num % num_tiles_x;
    tile_loc[1] = ((tile_num - tile_loc[0]) / num_tiles_x) % num_tiles_y;
    tile_loc[2] = ((((tile_num - tile_loc[0]) / num_tiles_x) / num_tiles_y) % num_tiles_z);
    /*
      tile_loc[0] = tile_num % rdims[0];
      tile_loc[1] = ((tile_num - tile_loc[0]) / rdims[0]) % rdims[1];
      tile_loc[2] = ((((tile_num - tile_loc[0]) / rdims[0]) / rdims[1]) % rdims[2]);
    */

    // Tiles do not start on the edges of the grid, so we
    // push them to the center of the control grid.
    tile_loc[0]++;
    tile_loc[1]++;
    tile_loc[2]++;

    // Find 64 knots' [x,y,z] coordinates
    // and convert into a linear knot index
    for (k = -1; k < 3; k++)
        for (j = -1; j < 3; j++)
            for (i = -1; i < 3; i++)
            {
                knots[idx++] = (cdims[0]*cdims[1]*(tile_loc[2]+k)) + (cdims[0]*(tile_loc[1]+j)) + (tile_loc[0]+i);
            }

}



/* -----------------------------------------------------------------------
   Debugging routines
   ----------------------------------------------------------------------- */
void
dump_total_gradient (Bspline_xform* bxf, Bspline_score* ssd, const char* fn)
{
    int i;
    FILE* fp;

    make_parent_directories (fn);
    fp = fopen (fn, "wb");
    for (i = 0; i < bxf->num_coeff; i++) {
        fprintf (fp, "%20.20f\n", ssd->total_grad[i]);
    }
    fclose (fp);
}

void
bspline_display_coeff_stats (Bspline_xform* bxf)
{
    float cf_min, cf_avg, cf_max;
    int i;

    cf_avg = 0.0;
    cf_min = cf_max = bxf->coeff[0];
    for (i = 0; i < bxf->num_coeff; i++) {
        cf_avg += bxf->coeff[i];
        if (cf_min > bxf->coeff[i]) cf_min = bxf->coeff[i];
        if (cf_max < bxf->coeff[i]) cf_max = bxf->coeff[i];
    }
    logfile_printf ("         "
                    "CMIN %6.2f CAVG %6.2f CMAX %6.2f\n", 
                    cf_min, cf_avg / bxf->num_coeff, cf_max);
}

void
bspline_save_debug_state (
    Bspline_parms *parms, 
    Bspline_state *bst, 
    Bspline_xform *bxf
)
{
    if (parms->debug) {
        std::string fn;
        char buf[1024];

        sprintf (buf, "%02d_grad_%03d_%03d.txt", 
            parms->debug_stage, bst->it, bst->feval);
        fn = parms->debug_dir + "/" + buf;
        dump_total_gradient (bxf, &bst->ssd, fn.c_str());

        sprintf (buf, "%02d_coeff_%03d_%03d.txt", 
            parms->debug_stage, bst->it, bst->feval);
        fn = parms->debug_dir + "/" + buf;
        bxf->save (fn.c_str());

        if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
            sprintf (buf, "%02d_", parms->debug_stage);
            fn = parms->debug_dir + "/" + buf;
            bst->mi_hist->dump_hist (bst->feval, fn);
        }
    }
}

Volume*
bspline_compute_vf (const Bspline_xform* bxf)
{
    Volume* vf = new Volume (
        bxf->img_dim, bxf->img_origin, 
        bxf->img_spacing, 0, 
        PT_VF_FLOAT_INTERLEAVED, 3
    );
    bspline_interpolate_vf (vf, bxf);

    return vf;
}

void
bspline_update_sets (float* sets_x, float* sets_y, float* sets_z,
    int qidx, float* dc_dv, Bspline_xform* bxf)
{
    int sidx;   // set index

    /* Initialize q_lut */
    float* q_lut = &bxf->q_lut[64*qidx];

    /* Condense dc_dv & commit to sets for tile */
    for (sidx=0; sidx<64; sidx++) {
        sets_x[sidx] += dc_dv[0] * q_lut[sidx];
        sets_y[sidx] += dc_dv[1] * q_lut[sidx];
        sets_z[sidx] += dc_dv[2] * q_lut[sidx];
    }
}

void
bspline_update_sets_b (float* sets_x, float* sets_y, float* sets_z,
    plm_long *q, float* dc_dv, Bspline_xform* bxf)
{
    int i,j,k,m;
    float A,B,C;

    /* Initialize b_luts */
    float* bx_lut = &bxf->bx_lut[q[0]*4];
    float* by_lut = &bxf->by_lut[q[1]*4];
    float* bz_lut = &bxf->bz_lut[q[2]*4];

    /* Condense dc_dv & commit to sets for tile */
    m=0;
    for (k=0; k<4; k++) {
        C = bz_lut[k];
        for (j=0; j<4; j++) {
            B = by_lut[j] * C;
            for (i=0; i<4; i++) {
                A = bx_lut[i] * B;
                sets_x[m] += dc_dv[0] * A;
                sets_y[m] += dc_dv[1] * A;
                sets_z[m] += dc_dv[2] * A;
                m++;
            }
        }
    }
}


void
bspline_sort_sets (float* cond_x, float* cond_y, float* cond_z,
    float* sets_x, float* sets_y, float* sets_z,
    plm_long pidx, Bspline_xform* bxf)
{
    int sidx, kidx;
    plm_long* k_lut = (plm_long*) malloc (64*sizeof(plm_long));

    /* Generate the knot index lut */
    find_knots (k_lut, pidx, bxf->rdims, bxf->cdims);

    /* Rackem' Up */
    for (sidx=0; sidx<64; sidx++) {
        kidx = k_lut[sidx];

        cond_x[ (64*kidx) + sidx ] = sets_x[sidx];
        cond_y[ (64*kidx) + sidx ] = sets_y[sidx];
        cond_z[ (64*kidx) + sidx ] = sets_z[sidx];
    }

    free (k_lut);
}

void
bspline_condense_smetric_grad (float* cond_x, float* cond_y, float* cond_z,
    Bspline_xform* bxf, Bspline_score* ssd)
{
    plm_long kidx, sidx;

    for (kidx=0; kidx < bxf->num_knots; kidx++) {
        for (sidx=0; sidx<64; sidx++) {
            ssd->smetric_grad[3*kidx + 0] += cond_x[64*kidx + sidx];
            ssd->smetric_grad[3*kidx + 1] += cond_y[64*kidx + sidx];
            ssd->smetric_grad[3*kidx + 2] += cond_z[64*kidx + sidx];
        }
    }
}

static void
logfile_print_score (float score)
{
    if (score < 10. && score > -10.) {
        logfile_printf (" %1.8f ", score);
    } else {
        logfile_printf (" %9.3f ", score);
    }
}

void
report_score (
    Bspline_parms *parms,
    Bspline_xform *bxf, 
    Bspline_state *bst
) 
{
    Bspline_score* ssd = &bst->ssd;
    Regularization_parms* reg_parms = parms->reg_parms;
    Bspline_landmarks* blm = parms->blm;

    int i;
    double ssd_grad_norm, ssd_grad_mean;

    /* Compute gradient statistics */
    ssd_grad_norm = 0;
    ssd_grad_mean = 0;
    for (i = 0; i < bxf->num_coeff; i++) {
        ssd_grad_mean += bst->ssd.total_grad[i];
        ssd_grad_norm += (double) bst->ssd.total_grad[i]
            * (double) bst->ssd.total_grad[i];
    }

    /* Compute total time */
    double total_time = 0;
    std::vector<double>::const_iterator it_time = ssd->time_smetric.begin();
    while (it_time != ssd->time_smetric.end()) {
        total_time += *it_time;
        ++it_time;
    }
    total_time += ssd->time_rmetric;
    
    /* First line, iterations, score, misc stats */
    logfile_printf ("[%2d,%3d] ", bst->it, bst->feval);
    if (reg_parms->lambda > 0 || blm->num_landmarks > 0
        || parms->metric_type.size() > 1)
    {
        logfile_printf ("SCORE ");
    } else {
        logfile_printf ("%-6s", registration_metric_type_string (
                parms->metric_type[0]));
    }
    logfile_print_score (ssd->score);
    logfile_printf (
        "NV %6d GM %9.3f GN %9.3g [ %9.3f s ]\n",
        ssd->num_vox, ssd_grad_mean, sqrt (ssd_grad_norm), total_time);
    
    /* Second line */
    if (reg_parms->lambda > 0 || blm->num_landmarks > 0
        || parms->metric_type.size() > 1)
    {
        logfile_printf ("         ");
        /* Part 1 - smetric(s) */   
        std::vector<float>::const_iterator it_sm = ssd->smetric.begin();
        std::vector<Registration_metric_type>::const_iterator it_st
            = parms->metric_type.begin();
        while (it_sm != ssd->smetric.end()) {
            logfile_printf ("%-6s",
                registration_metric_type_string (*it_st));
            logfile_print_score (*it_sm);
            ++it_sm, ++it_st;
        }
        if (ssd->smetric.size() > 1
            && (reg_parms->lambda > 0 || blm->num_landmarks > 0))
        {
            logfile_printf ("\n");
            logfile_printf ("         ");
        }
        if (reg_parms->lambda > 0 || blm->num_landmarks > 0) {
            /* Part 2 - regularization metric */
            if (reg_parms->lambda > 0) {
                logfile_printf ("RM %9.3f ", 
                    reg_parms->lambda * bst->ssd.rmetric);
            }
            /* Part 3 - landmark metric */
            if (blm->num_landmarks > 0) {
                logfile_printf ("LM %9.3f ", 
                    blm->landmark_stiffness * bst->ssd.lmetric);
            }
            /* Part 4 - timing */
            if (reg_parms->lambda > 0) {
                logfile_printf ("[ %9.3f | %9.3f ]", 
                    ssd->time_smetric[0], ssd->time_rmetric);
            }
        }
        logfile_printf ("\n");
    }
}

void
bspline_score (Bspline_optimize *bod)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Regularization_parms* reg_parms = parms->reg_parms;
    Bspline_landmarks* blm = parms->blm;

    /* Zero out the score for this iteration */
    bst->ssd.reset_score ();

    /* Compute similarity metrics */
    std::vector<Registration_metric_type>::const_iterator it_metric
        = parms->metric_type.begin();
    std::vector<float>::const_iterator it_lambda
        = parms->metric_lambda.begin();
    bst->sm = 0;
    while (it_metric != parms->metric_type.end()
        && it_lambda != parms->metric_lambda.end())
    {
        Plm_timer timer;
        timer.start ();
        bst->ssd.smetric.push_back (0.f);
        if (*it_metric == REGISTRATION_METRIC_MSE) {
            bspline_score_mse (bod);
        }
        else if (*it_metric == REGISTRATION_METRIC_MI_MATTES) {
            bspline_score_mi (bod);
        }
        else if (*it_metric == REGISTRATION_METRIC_GM) {
            bspline_score_gm (bod);
        }
        else {
            print_and_exit ("Unknown similarity metric in bspline_score()\n");
        }

        bst->ssd.accumulate_grad (*it_lambda);

        bst->ssd.time_smetric.push_back (timer.report ());
        bst->sm ++;
        ++it_metric;
        ++it_lambda;
    }

    /* Compute regularization */
    if (reg_parms->lambda > 0.0f) {
        bst->rst.compute_score (&bst->ssd, reg_parms, bxf);
    }

    /* Compute landmark score/gradient to image score/gradient */
    if (blm->num_landmarks > 0) {
        bspline_landmarks_score (parms, bst, bxf);
    }

    /* Compute total score */
    bst->ssd.score = bst->ssd.smetric[0] + reg_parms->lambda * bst->ssd.rmetric;
    if (blm->num_landmarks > 0) {
        bst->ssd.score += blm->landmark_stiffness * bst->ssd.lmetric;
    }

    /* Report results of this iteration */
    report_score (parms, bxf, bst);
}
