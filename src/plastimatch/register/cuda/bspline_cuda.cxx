/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#define PLM_CUDA_COMPILE 1
#include "plmregister_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined (_WIN32)
#include <windows.h>
#endif

#include "bspline.h"
#if (CUDA_FOUND)
#include "bspline_cuda.h"
#include "cuda_util.h"
#endif
#include "bspline_correspond.h"
#include "bspline_interpolate.h"
#include "bspline_optimize.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "compiler_warnings.h"
#include "interpolate.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "plm_math.h"
#include "volume.h"
#include "volume_macros.h"

/***********************************************************************
 * A few of the CPU functions are reproduced here for testing purposes.
 * Once the CPU code is removed from the functions below, these
 * functions can be deleted.
 ***********************************************************************/
// #define ROUND_INT(x) ((x)>=0?(long)((x)+0.5):(long)(-(-(x)+0.5)))

// JAS 2010.11.23
// Sorry about this... these functions are reproductions of stuff that lives in
// bspline.c  These common functions will need to be eventually moved to their
// own object in order for linking to work nicely for shared libs...
// (like the CUDA plugin)
#if defined (PLM_USE_GPU_PLUGINS)
void
clamp_linear_interpolate (
    float ma,           /*  Input: (Unrounded) pixel coordinate (in vox) */
    int dmax,		/*  Input: Maximum coordinate in this dimension */
    int* maf,		/* Output: x, y, or z coord of "floor" pixel in moving img */
    int* mar,		/* Output: x, y, or z coord of "round" pixel in moving img */
    float* fa1,		/* Output: Fraction of interpolant for lower index voxel */
    float* fa2		/* Output: Fraction of interpolant for upper index voxel */
)
{
    float maff = floor(ma);
    *maf = (int) maff;
    *mar = ROUND_INT (ma);
    *fa2 = ma - maff;
    if (*maf < 0) {
	*maf = 0;
	*mar = 0;
	*fa2 = 0.0f;
    } else if (*maf >= dmax) {
	*maf = dmax - 1;
	*mar = dmax;
	*fa2 = 1.0f;
    }
    *fa1 = 1.0f - *fa2;
}

void
bspline_interp_pix_b (
    float out[3], 
    Bspline_xform* bxf, 
    plm_long pidx, 
    plm_long qidx
)
{
    plm_long i, j, k, m;
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

int
bspline_find_correspondence 
(
 float *mxyz,             /* Output: xyz coordinates in moving image (mm) */
 float *mijk,             /* Output: ijk indices in moving image (vox) */
 const float *fxyz,       /* Input:  xyz coordinates in fixed image (mm) */
 const float *dxyz,       /* Input:  displacement from fixed to moving (mm) */
 const Volume *moving     /* Input:  moving image */
 )
{
    mxyz[0] = fxyz[0] + dxyz[0];
    mijk[0] = (mxyz[0] - moving->origin[0]) / moving->spacing[0];
    if (mijk[0] < -0.5 || mijk[0] > moving->dim[0] - 0.5) return 0;

    mxyz[1] = fxyz[1] + dxyz[1];
    mijk[1] = (mxyz[1] - moving->origin[1]) / moving->spacing[1];
    if (mijk[1] < -0.5 || mijk[1] > moving->dim[1] - 0.5) return 0;

    mxyz[2] = fxyz[2] + dxyz[2];
    mijk[2] = (mxyz[2] - moving->origin[2]) / moving->spacing[2];
    if (mijk[2] < -0.5 || mijk[2] > moving->dim[2] - 0.5) return 0;

    return 1;
}
#endif

static inline void
bspline_mi_hist_add_pvi_8 (
    Joint_histogram* mi_hist, 
    Volume *fixed, 
    Volume *moving, 
    int fv, 
    int mvf, 
    float li_1[3],           /* Fraction of interpolant in lower index */
    float li_2[3])           /* Fraction of interpolant in upper index */
{
    float w1, w2, w3, w4, w5, w6, w7, w8;
    int   n1, n2, n3, n4, n5, n6, n7, n8;
    int idx_fbin, idx_mbin, idx_jbin;
    int offset_fbin;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    double *f_hist = mi_hist->f_hist;
    double *m_hist = mi_hist->m_hist;
    double *j_hist = mi_hist->j_hist;

    w1 = li_1[0] * li_1[1] * li_1[2];   // Partial Volume w1
    w2 = li_2[0] * li_1[1] * li_1[2];   // Partial Volume w2
    w3 = li_1[0] * li_2[1] * li_1[2];   // Partial Volume w3
    w4 = li_2[0] * li_2[1] * li_1[2];   // Partial Volume w4
    w5 = li_1[0] * li_1[1] * li_2[2];   // Partial Volume w5
    w6 = li_2[0] * li_1[1] * li_2[2];   // Partial Volume w6
    w7 = li_1[0] * li_2[1] * li_2[2];   // Partial Volume w7
    w8 = li_2[0] * li_2[1] * li_2[2];   // Partial Volume w8

    // Note that Sum(wN) for N within [1,8] should = 1 (checked OK)

    // Calculate Point Indices for 8 neighborhood
    n1 = mvf;
    n2 = n1 + 1;
    n3 = n1 + moving->dim[0];
    n4 = n1 + moving->dim[0] + 1;
    n5 = n1 + moving->dim[0]*moving->dim[1];
    n6 = n1 + moving->dim[0]*moving->dim[1] + 1;
    n7 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0];
    n8 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0] + 1;

    // Calculate fixed histogram bin and increment it
    idx_fbin = floor ((f_img[fv] - mi_hist->fixed.offset) / mi_hist->fixed.delta);
    f_hist[idx_fbin]++;

    offset_fbin = idx_fbin * mi_hist->moving.bins;

    // Add PV w1 to moving & joint histograms   
    idx_mbin = floor ((m_img[n1] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w1;
    j_hist[idx_jbin] += w1;

    // Add PV w2 to moving & joint histograms   
    idx_mbin = floor ((m_img[n2] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w2;
    j_hist[idx_jbin] += w2;

    // Add PV w3 to moving & joint histograms   
    idx_mbin = floor ((m_img[n3] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w3;
    j_hist[idx_jbin] += w3;

    // Add PV w4 to moving & joint histograms   
    idx_mbin = floor ((m_img[n4] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w4;
    j_hist[idx_jbin] += w4;

    // Add PV w5 to moving & joint histograms   
    idx_mbin = floor ((m_img[n5] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w5;
    j_hist[idx_jbin] += w5;
    
    // Add PV w6 to moving & joint histograms   
    idx_mbin = floor ((m_img[n6] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w6;
    j_hist[idx_jbin] += w6;

    // Add PV w7 to moving & joint histograms   
    idx_mbin = floor ((m_img[n7] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w7;
    j_hist[idx_jbin] += w7;
    
    // Add PV w8 to moving & joint histograms   
    idx_mbin = floor ((m_img[n8] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    m_hist[idx_mbin] += w8;
    j_hist[idx_jbin] += w8;

}

static inline void
bspline_mi_pvi_8_dc_dv (
    float dc_dv[3],                /* Output */
    Joint_histogram* mi_hist,      /* Input */
    Bspline_state *bst,            /* Input */
    Volume *fixed,                 /* Input */
    Volume *moving,                /* Input */
    int fv,                        /* Input */
    int mvf,                       /* Input */
    float mijk[3],                 /* Input */
    float num_vox_f,               /* Input */
    float li_1[3],                 /* Input */
    float li_2[3]                  /* Input */
)
{
    float dS_dP;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    Bspline_score* ssd = &bst->ssd;
    int n1, n2, n3, n4, n5, n6, n7, n8;
    int idx_fbin, idx_mbin, idx_jbin;
    int offset_fbin;
    float dw1[3], dw2[3], dw3[3], dw4[3], dw5[3], dw6[3], dw7[3], dw8[3];

    dc_dv[0] = dc_dv[1] = dc_dv[2] = 0.0f;

    // Calculate Point Indices for 8 neighborhood
    n1 = mvf;
    n2 = n1 + 1;
    n3 = n1 + moving->dim[0];
    n4 = n1 + moving->dim[0] + 1;
    n5 = n1 + moving->dim[0]*moving->dim[1];
    n6 = n1 + moving->dim[0]*moving->dim[1] + 1;
    n7 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0];
    n8 = n1 + moving->dim[0]*moving->dim[1] + moving->dim[0] + 1;

    // Pre-compute differential PV slices
    dw1[0] = (  -1 ) * li_1[1] * li_1[2];
    dw1[1] = li_1[0] * (  -1 ) * li_1[2];
    dw1[2] = li_1[0] * li_1[1] * (  -1 );

    dw2[0] = (  +1 ) * li_1[1] * li_1[2];
    dw2[1] = li_2[0] * (  -1 ) * li_1[2];
    dw2[2] = li_2[0] * li_1[1] * (  -1 );

    dw3[0] = (  -1 ) * li_2[1] * li_1[2];
    dw3[1] = li_1[0] * (  +1 ) * li_1[2];
    dw3[2] = li_1[0] * li_2[1] * (  -1 );

    dw4[0] = (  +1 ) * li_2[1] * li_1[2];
    dw4[1] = li_2[0] * (  +1 ) * li_1[2];
    dw4[2] = li_2[0] * li_2[1] * (  -1 );

    dw5[0] = (  -1 ) * li_1[1] * li_2[2];
    dw5[1] = li_1[0] * (  -1 ) * li_2[2];
    dw5[2] = li_1[0] * li_1[1] * (  +1 );

    dw6[0] = (  +1 ) * li_1[1] * li_2[2];
    dw6[1] = li_2[0] * (  -1 ) * li_2[2];
    dw6[2] = li_2[0] * li_1[1] * (  +1 );

    dw7[0] = (  -1 ) * li_2[1] * li_2[2];
    dw7[1] = li_1[0] * (  +1 ) * li_2[2];
    dw7[2] = li_1[0] * li_2[1] * (  +1 );

    dw8[0] = (  +1 ) * li_2[1] * li_2[2];
    dw8[1] = li_2[0] * (  +1 ) * li_2[2];
    dw8[2] = li_2[0] * li_2[1] * (  +1 );

    // Fixed image voxel's histogram index
    idx_fbin = floor ((f_img[fv] - mi_hist->fixed.offset) / mi_hist->fixed.delta);
    offset_fbin = idx_fbin * mi_hist->moving.bins;

    // Partial Volume w1's Contribution
    idx_mbin = floor ((m_img[n1] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw1[0] * dS_dP;
        dc_dv[1] -= dw1[1] * dS_dP;
        dc_dv[2] -= dw1[2] * dS_dP;
    }

    // Partial Volume w2
    idx_mbin = floor ((m_img[n2] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw2[0] * dS_dP;
        dc_dv[1] -= dw2[1] * dS_dP;
        dc_dv[2] -= dw2[2] * dS_dP;
    }

    // Partial Volume w3
    idx_mbin = floor ((m_img[n3] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw3[0] * dS_dP;
        dc_dv[1] -= dw3[1] * dS_dP;
        dc_dv[2] -= dw3[2] * dS_dP;
    }

    // Partial Volume w4
    idx_mbin = floor ((m_img[n4] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw4[0] * dS_dP;
        dc_dv[1] -= dw4[1] * dS_dP;
        dc_dv[2] -= dw4[2] * dS_dP;
    }

    // Partial Volume w5
    idx_mbin = floor ((m_img[n5] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw5[0] * dS_dP;
        dc_dv[1] -= dw5[1] * dS_dP;
        dc_dv[2] -= dw5[2] * dS_dP;
    }

    // Partial Volume w6
    idx_mbin = floor ((m_img[n6] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw6[0] * dS_dP;
        dc_dv[1] -= dw6[1] * dS_dP;
        dc_dv[2] -= dw6[2] * dS_dP;
    }

    // Partial Volume w7
    idx_mbin = floor ((m_img[n7] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw7[0] * dS_dP;
        dc_dv[1] -= dw7[1] * dS_dP;
        dc_dv[2] -= dw7[2] * dS_dP;
    }

    // Partial Volume w8
    idx_mbin = floor ((m_img[n8] - mi_hist->moving.offset) / mi_hist->moving.delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > 0.0001) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
        dc_dv[0] -= dw8[0] * dS_dP;
        dc_dv[1] -= dw8[1] * dS_dP;
        dc_dv[2] -= dw8[2] * dS_dP;
    }

    dc_dv[0] = dc_dv[0] / num_vox_f / moving->spacing[0];
    dc_dv[1] = dc_dv[1] / num_vox_f / moving->spacing[1];
    dc_dv[2] = dc_dv[2] / num_vox_f / moving->spacing[2];
}

#if defined (MI_GRAD_CPU)
inline void
bspline_update_grad_b_inline (Bspline_state* bst, Bspline_xform* bxf, 
             int pidx, int qidx, float dc_dv[3])
{
    Bspline_score* ssd = &bst->ssd;
    int i, j, k, m;
    int cidx;
    float* q_lut = &bxf->q_lut[qidx*64];
    int* c_lut = &bxf->c_lut[pidx*64];

    m = 0;
    for (k = 0; k < 4; k++) {
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
        cidx = 3 * c_lut[m];
        ssd->grad[cidx+0] += dc_dv[0] * q_lut[m];
        ssd->grad[cidx+1] += dc_dv[1] * q_lut[m];
        ssd->grad[cidx+2] += dc_dv[2] * q_lut[m];
        m ++;
        }
    }
    }
}
#endif

#if defined (commentout)
static void display_hist_totals (Joint_histogram *mi_hist)
{
    plm_long i;
    double tmp = 0;

    for (i=0, tmp=0; i < mi_hist->fixed.bins; i++) {
        tmp += mi_hist->f_hist[i];
    }
    printf ("f_hist total: %f\n", tmp);

    for (i=0, tmp=0; i < mi_hist->moving.bins; i++) {
        tmp += mi_hist->m_hist[i];
    }
    printf ("m_hist total: %f\n", tmp);

    for (i=0, tmp=0; i < mi_hist->moving.bins * mi_hist->fixed.bins; i++) {
        tmp += mi_hist->j_hist[i];
    }
    printf ("j_hist total: %f\n", tmp);
}
#endif


////////////////////////////////////////////////////////////////////////////////
size_t
CPU_MI_Hist (Joint_histogram *mi_hist,  // OUTPUT: Histograms
    Bspline_xform *bxf,                 //  INPUT: Bspline X-Form
    Volume* fixed,                      //  INPUT: Fixed Image
    Volume* moving)                     //  INPUT: Moving Image
{
    plm_long rijk[3];
    plm_long fijk[3];
    plm_long fv;
    plm_long p[3];
    plm_long q[3];
    float fxyz[3];
    plm_long pidx, qidx;
    float dxyz[3];
    float mxyz[3];
    float mijk[3];
    plm_long mijk_f[3];  // floor: mijk
    plm_long mijk_r[3];  // round: mijk
    plm_long mvf;        // floor: mv
    float li_1[3];
    float li_2[3];
    plm_long num_vox = 0;

    for (rijk[2] = 0, fijk[2] = bxf->roi_offset[2]; rijk[2] < bxf->roi_dim[2]; rijk[2]++, fijk[2]++) {
        p[2] = rijk[2] / bxf->vox_per_rgn[2];
        q[2] = rijk[2] % bxf->vox_per_rgn[2];
        fxyz[2] = bxf->img_origin[2] + bxf->img_spacing[2] * fijk[2];
        for (rijk[1] = 0, fijk[1] = bxf->roi_offset[1]; rijk[1] < bxf->roi_dim[1]; rijk[1]++, fijk[1]++) {
            p[1] = rijk[1] / bxf->vox_per_rgn[1];
            q[1] = rijk[1] % bxf->vox_per_rgn[1];
            fxyz[1] = bxf->img_origin[1] + bxf->img_spacing[1] * fijk[1];
            for (rijk[0] = 0, fijk[0] = bxf->roi_offset[0]; rijk[0] < bxf->roi_dim[0]; rijk[0]++, fijk[0]++) {
                int rc;
                p[0] = rijk[0] / bxf->vox_per_rgn[0];
                q[0] = rijk[0] % bxf->vox_per_rgn[0];
                fxyz[0] = bxf->img_origin[0] + bxf->img_spacing[0] * fijk[0];

                // Get B-spline deformation vector
                pidx = volume_index (bxf->rdims, p);
                qidx = volume_index (bxf->vox_per_rgn, q);
                bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

                // Find correspondence in moving image
                rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                // If voxel is not inside moving image
                if (!rc) continue;

                // Compute tri-linear interpolation weights
                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                // Find linear index of fixed image voxel
                fv = volume_index (fixed->dim, fijk);

                // Find linear index of "corner voxel" in moving image
                mvf = volume_index (moving->dim, mijk_f);

                // PARTIAL VALUE INTERPOLATION - 8 neighborhood
                bspline_mi_hist_add_pvi_8 (mi_hist, fixed, moving, fv, mvf, li_1, li_2);

                // Increment the voxel count
                num_vox ++;
            }
        }
    }

    return num_vox;
}
////////////////////////////////////////////////////////////////////////////////


static float
CPU_MI_Score (Joint_histogram* mi_hist, int num_vox)
{
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

    plm_long i, j, v;
    double fnv = (double) num_vox;
    double score = 0;
    float hist_thresh = 0.001 / (mi_hist->moving.bins * mi_hist->fixed.bins);

    /* Compute cost */
    for (i = 0, v = 0; i < mi_hist->fixed.bins; i++) {
        for (j = 0; j < mi_hist->moving.bins; j++, v++) {
            if (j_hist[v] > hist_thresh) {
                score -= j_hist[v] * logf (fnv * j_hist[v] / (m_hist[j] * f_hist[i]));
            }
        }
    }

    score = score / fnv;
    return (float) score;
}

#if defined (MI_GRAD_CPU)
void
CPU_MI_Grad (Joint_histogram *mi_hist, // OUTPUT: Histograms
        Bspline_state *bst,     //  INPUT: Bspline State
        Bspline_xform *bxf,     //  INPUT: Bspline X-Form
        Volume* fixed,          //  INPUT: Fixed Image
        Volume* moving,         //  INPUT: Moving Image
        float num_vox_f)        //  INPUT: Number of voxels
{
    int rijk[3];
    int fijk[3];
    int fv;
    int p[3];
    int q[3];
    float fxyz[3];
    int pidx, qidx;
    float dxyz[3];
    float mxyz[3];
    float mijk[3];
    int mijk_f[3];  // floor: mijk
    int mijk_r[3];  // round: mijk
    int mvf;    // floor: mv
    float li_1[3];
    float li_2[3];
    float dc_dv[3];

    for (rijk[2] = 0, fijk[2] = bxf->roi_offset[2]; rijk[2] < bxf->roi_dim[2]; rijk[2]++, fijk[2]++) {
        p[2] = rijk[2] / bxf->vox_per_rgn[2];
        q[2] = rijk[2] % bxf->vox_per_rgn[2];
        fxyz[2] = bxf->img_origin[2] + bxf->img_spacing[2] * fijk[2];
        for (rijk[1] = 0, fijk[1] = bxf->roi_offset[1]; rijk[1] < bxf->roi_dim[1]; rijk[1]++, fijk[1]++) {
            p[1] = rijk[1] / bxf->vox_per_rgn[1];
            q[1] = rijk[1] % bxf->vox_per_rgn[1];
            fxyz[1] = bxf->img_origin[1] + bxf->img_spacing[1] * fijk[1];
            for (rijk[0] = 0, fijk[0] = bxf->roi_offset[0]; rijk[0] < bxf->roi_dim[0]; rijk[0]++, fijk[0]++) {
                int rc;

                p[0] = rijk[0] / bxf->vox_per_rgn[0];
                q[0] = rijk[0] % bxf->vox_per_rgn[0];
                fxyz[0] = bxf->img_origin[0] + bxf->img_spacing[0] * fijk[0];

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                qidx = volume_index (bxf->vox_per_rgn, q);
                bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

                /* Find linear index of fixed image voxel */
                fv = volume_index (fixed->dim, fijk);

                /* Find correspondence in moving image */
                rc = bspline_find_correspondence (mxyz, mijk, fxyz, dxyz, moving);

                /* If voxel is not inside moving image */
                if (!rc) continue;

                /* PARTIAL VALUE INTERPOLATION - 8 neighborhood */
                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of fixed image voxel */
                fv = volume_index (fixed->dim, fijk);

                /* Find linear index of "corner voxel" in moving image */
                mvf = volume_index (moving->dim, mijk_f);

                // Partial Volume Interpolation 8-neighbor Gradient 
                bspline_mi_pvi_8_dc_dv (dc_dv, mi_hist, bst, fixed, moving, 
                            fv, mvf, mijk, num_vox_f, li_1, li_2);

                // B-Spline parameterization
                bspline_update_grad_b_inline (bst, bxf, pidx, qidx, dc_dv);
            }
        }
    }
}
#endif

void
CUDA_bspline_mi_a (
    Bspline_parms *parms,
    Bspline_state *bst,
    Bspline_xform *bxf
)
{
    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;
    Volume *moving_grad = bst->moving_grad;
    UNUSED_VARIABLE (moving_grad);

    Dev_Pointers_Bspline* dev_ptrs = (Dev_Pointers_Bspline*)bst->dev_ptrs;

    // --- DECLARE LOCAL VARIABLES ------------------------------
    Bspline_score* ssd; // Holds the SSD "Score" information
    Joint_histogram* mi_hist = bst->get_mi_hist();
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    static int it=0;        // Holds Iteration Number
    char debug_fn[1024];    // Debug message buffer
    FILE* fp = NULL;        // File Pointer to Debug File
    //int i;                  // Good ol' i
    // ----------------------------------------------------------


    // --- CHECK COMPUTE CAPABILITY (NEED SHARED/GLOBAL ATOMICS)-
    if (CUDA_getarch(parms->gpuid) < 120) {

        printf ("\n*******************   NOTICE  *******************\n");
        printf ("     A GPU of Compute Capability 1.2 or greater\n");
        printf ("     is required to for GPU accelerated MI\n");
        printf ("\n");
        printf ("     Unfortunately, your current GPU does not\n");
        printf ("     satisfy this requirement.  Sorry.\n");
        printf ("***************************************************\n\n");
        exit(0);
    }
    // ----------------------------------------------------------


    // --- INITIALIZE LOCAL VARIABLES ---------------------------
    ssd = &bst->ssd;
    
    if (parms->debug) {
        sprintf (debug_fn, "dump_mse_%02d.txt", it++);
        fp = fopen (debug_fn, "w");
    }
    // ----------------------------------------------------------


#if defined (MI_HISTS_CPU) || defined (MI_SCORE_CPU)
    // --- INITIALIZE CPU MEMORY --------------------------------
    memset(f_hist, 0, mi_hist->fixed.bins * sizeof (double));
    memset(m_hist, 0, mi_hist->moving.bins * sizeof (double));
    memset(j_hist, 0, mi_hist->moving.bins * mi_hist->fixed.bins * sizeof (double));
    // ----------------------------------------------------------
#endif

    // --- INITIALIZE GPU MEMORY --------------------------------
    CUDA_bspline_push_coeff (dev_ptrs, bxf);
    CUDA_bspline_zero_score (dev_ptrs);
    CUDA_bspline_zero_grad  (dev_ptrs);
    // ----------------------------------------------------------

    // --- GENERATE HISTOGRMS -----------------------------------
    if ((mi_hist->fixed.bins > GPU_MAX_BINS) ||
        (mi_hist->moving.bins > GPU_MAX_BINS)) {

        ssd->curr_num_vox = CPU_MI_Hist (mi_hist, bxf, fixed, moving);
    } else {
        ssd->curr_num_vox = CUDA_bspline_mi_hist (dev_ptrs, mi_hist, fixed, moving, bxf);
    }
    // ----------------------------------------------------------

    // dump histogram images?
#if !defined (PLM_USE_GPU_PLUGINS)
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }
#endif

    if (parms->debug) {
//        dump_hist (mi_hist, bst->it);
    }

#if defined (commentout)
    display_hist_totals (mi_hist);
#endif


    // --- COMPUTE SCORE ----------------------------------------
#if defined (MI_SCORE_CPU)
    ssd->curr_smetric = CPU_MI_Score(mi_hist, ssd->curr_num_vox);
#else
    // Doing this on the GPU may be silly.
    // The CPU generally completes this computation extremely quickly
#endif
    // ----------------------------------------------------------

    // --- COMPUTE GRADIENT -------------------------------------
#if defined (MI_GRAD_CPU)
    CPU_MI_Grad(mi_hist, bst, bxf, fixed, moving, (float)ssd->num_vox);
#else
    float score = ssd->curr_smetric;
    CUDA_bspline_mi_grad (
        bst,
        bxf,
        fixed,
        moving,
        (float) ssd->curr_num_vox,
        score,
        dev_ptrs
    );
#endif
    // ----------------------------------------------------------

    if (parms->debug) {
        fclose (fp);
    }
}

void
CUDA_bspline_mse_j (
    Bspline_parms *parms,
    Bspline_state *bst,
    Bspline_xform *bxf
)
{
    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;
    Volume *moving_grad = bst->moving_grad;

    Dev_Pointers_Bspline* dev_ptrs = (Dev_Pointers_Bspline*)bst->dev_ptrs;

    // --- DECLARE LOCAL VARIABLES ------------------------------
    Bspline_score* ssd;     // Holds the SSD "Score" information
    float ssd_grad_norm;    // Holds the SSD Gradient's Norm
    float ssd_grad_mean;    // Holds the SSD Gradient's Mean

    static int it=0;        // Holds Iteration Number
    char debug_fn[1024];    // Debug message buffer
    FILE* fp = NULL;        // File Pointer to Debug File
    // ----------------------------------------------------------
    
    // --- INITIALIZE LOCAL VARIABLES ---------------------------
    ssd = &bst->ssd;
    
    if (parms->debug) {
        sprintf (debug_fn, "dump_mse_%02d.txt", it++);
        fp = fopen (debug_fn, "w");
    }
    // ----------------------------------------------------------

    // --- INITIALIZE GPU MEMORY --------------------------------
    CUDA_bspline_push_coeff (dev_ptrs, bxf);
    CUDA_bspline_zero_score (dev_ptrs);
    CUDA_bspline_zero_grad  (dev_ptrs);
    // ----------------------------------------------------------


    // --- LAUNCH STUB FUNCTIONS --------------------------------

    // Populate the score, dc_dv, and gradient
    CUDA_bspline_mse_pt1 (
        fixed,
        moving,
        moving_grad,
        bxf,
        parms,
        dev_ptrs
    );


    // Calculate the score and gradient
    // via sum reduction
    CUDA_bspline_mse_pt2 (
        parms,
        bxf,
        fixed,
        bxf->vox_per_rgn,
        fixed->dim,
        &(ssd->curr_smetric),
        bst->ssd.curr_smetric_grad,
        &ssd_grad_mean,
        &ssd_grad_norm,
        dev_ptrs,
        &(ssd->curr_num_vox)
    );

    if (parms->debug) {
        fclose (fp);
    }
}

////////////////////////////////////////////////////////////////////////////////

