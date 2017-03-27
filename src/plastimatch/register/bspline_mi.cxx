/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
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
#include "bspline_mi.h"
#include "bspline_mi.txx"
#include "bspline_optimize.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "file_util.h"
#include "interpolate.h"
#include "interpolate_macros.h"
#include "joint_histogram.h"
#include "logfile.h"
#include "mha_io.h"
#include "plm_math.h"
#include "volume_macros.h"
#include "volume.h"

/* Some neat debug facilities
 *   Will probably move these somewhere more appropriate soon
 */
Volume*
volume_clip_intensity (Volume* vin, float bot, float top)
{
    plm_long i, j, N;
    int* marks;
    float* imgin;
    float* imgout;
    float min;
    Volume* vout;

    vout = volume_clone (vin);
        imgin  = (float*) vin->img;
        imgout = (float*) vout->img;

    N=0;
    min = FLT_MAX;
    for (i=0; i<vin->npix; i++) {
        if ( (imgin[i] >= bot) && (imgin[i] <= top) ) {
            N++;
        }
        if (imgin[i] < min) {
            min = imgin[i];
        }
    }
    if (N == 0) { return NULL; }
    marks = (int*) malloc (N * sizeof (int));

    j=0;
    for (i=0; i<vin->npix; i++) {
        if ( (imgin[i] >= bot) && (imgin[i] <= top) ) {
            marks[j++] = i;
        }
        imgout[i] = min;
    }

    for (i=0; i<j; i++) {
        imgout[marks[i]] = imgin[marks[i]];
    }

    free (marks);

    return vout;
}

#if defined (commentout)
static void
dump_vol_clipped (char* fn, Volume* vin, float bot, float top)
{
    Volume* vout;

    vout = volume_clip_intensity (vin, bot, top);
    if (vout) {
        write_mha (fn, vout);
        delete vout;
    }
}
#endif

/* -----------------------------------------------------------------------
   MI computation functions
   ----------------------------------------------------------------------- */
inline void
clamp_quadratic_interpolate_inline (
    float ma,           /* Input: (Unrounded) pixel coordinate (in vox units) */
    long dmax,          /* Input: Maximum coordinate in this dimension */
    long maqs[3],       /* Output: x, y, or z coord of 3 pixels in moving img */
    float faqs[3]       /* Output: Fraction of interpolant for 3 voxels */
)
{
    float marf = floorf (ma + 0.5); /* marf = ma, rounded, floating */
    long mari = (long) marf;        /* mari = ma, rounded, integer */

    float t = ma - marf + 0.5;
    float t2 = t * t;
    float t22 = 0.5 * t2;

    faqs[2] = t22;
    faqs[1] = - t2 + t + 0.5;
    faqs[0] = t22 - t + 0.5;

    maqs[0] = mari - 1;
    maqs[1] = mari;
    maqs[2] = mari + 1;

    if (maqs[0] < 0) {
        maqs[0] = 0;
        if (maqs[1] < 0) {
            maqs[1] = 0;
            if (maqs[2] < 0) {
                maqs[2] = 0;
            }
        }
    } else if (maqs[2] >= dmax) {
        maqs[2] = dmax - 1;
        if (maqs[1] >= dmax) {
            maqs[1] = dmax - 1;
            if (maqs[0] >= dmax) {
                maqs[0] = dmax - 1;
            }
        }
    }
}

inline void
clamp_quadratic_interpolate_grad_inline (
    float ma,          /* Input: (Unrounded) pixel coordinate (in vox units) */
    long dmax,         /* Input: Maximum coordinate in this dimension */
    long maqs[3],      /* Output: x, y, or z coord of 3 pixels in moving img */
    float faqs[3]      /* Output: Gradient interpolant for 3 voxels */
)
{
    float marf = floorf (ma + 0.5); /* marf = ma, rounded, floating */
    long mari = (long) marf;        /* mari = ma, rounded, integer */

    float t = ma - marf + 0.5;

    faqs[0] = -1.0f + t;
    faqs[1] = -2.0f * t + 1.0f;
    faqs[2] = t;

    maqs[0] = mari - 1;
    maqs[1] = mari;
    maqs[2] = mari + 1;

    if (maqs[0] < 0) {
        faqs[0] = faqs[1] = faqs[2] = 0.0f; /* No gradient at image boundary */
        maqs[0] = 0;
        if (maqs[1] < 0) {
            maqs[1] = 0;
            if (maqs[2] < 0) {
                maqs[2] = 0;
            }
        }
    } else if (maqs[2] >= dmax) {
        faqs[0] = faqs[1] = faqs[2] = 0.0f; /* No gradient at image boundary */
        maqs[2] = dmax - 1;
        if (maqs[1] >= dmax) {
            maqs[1] = dmax - 1;
            if (maqs[0] >= dmax) {
                maqs[0] = dmax - 1;
            }
        }
    }
}

/* This function will split the amout to add between two bins (linear interp) 
    based on m_val, but one bin based on f_val. */
inline void
bspline_mi_hist_lookup (
    long j_idxs[2],     /* Output: Joint histogram indices */
    long m_idxs[2],     /* Output: Moving marginal indices */
    long f_idxs[1],     /* Output: Fixed marginal indices */
    float fxs[2],       /* Output: Fraction contribution at indices */
    Joint_histogram* mi_hist,   /* Input:  The histogram */
    float f_val,        /* Input:  Intensity of fixed image */
    float m_val             /* Input:  Intensity of moving image */
)
{
    plm_long fl;
    float midx, midx_trunc;
    plm_long ml_1, ml_2;      /* 1-d index of bin 1, bin 2 */
    float mf_1, mf_2;            /* fraction to bin 1, bin 2 */
    long f_idx;                  /* Index into 2-d histogram */

    /* Fixed image is binned */
    fl = (long) floor ((f_val - mi_hist->fixed.offset) / mi_hist->fixed.delta);
    f_idx = fl * mi_hist->moving.bins;

    /* This had better not happen! */
    if (fl < 0 || fl >= mi_hist->fixed.bins) {
        fprintf (stderr, "Error: fixed image binning problem.\n"
            "Bin %ld from val %g parms [off=%g, delt=%g, (%ld bins)]\n",
            fl, f_val, mi_hist->fixed.offset, mi_hist->fixed.delta,
            mi_hist->fixed.bins);
        exit (-1);
    }
    
    /* Moving image binning is interpolated (linear, not b-spline) */
    midx = ((m_val - mi_hist->moving.offset) / mi_hist->moving.delta);
    midx_trunc = floorf (midx);
    ml_1 = (long) midx_trunc;
    mf_1 = midx - midx_trunc;    // Always between 0 and 1
    ml_2 = ml_1 + 1;
    mf_2 = 1.0 - mf_1;

    if (ml_1 < 0) {
        /* This had better not happen! */
        fprintf (stderr, "Error: moving image binning problem\n");
        exit (-1);
    } else if (ml_2 >= mi_hist->moving.bins) {
        /* This could happen due to rounding */
        ml_1 = mi_hist->moving.bins - 2;
        ml_2 = mi_hist->moving.bins - 1;
        mf_1 = 0.0;
        mf_2 = 1.0;
    }

    if (mf_1 < 0.0 || mf_1 > 1.0 || mf_2 < 0.0 || mf_2 > 1.0) {
        fprintf (stderr, "Error: MI interpolation problem\n");
        exit (-1);
    }

    j_idxs[0] = f_idx + ml_1;
    j_idxs[1] = f_idx + ml_2;
    fxs[0] = mf_1;
    fxs[1] = mf_2;
    f_idxs[0] = fl;
    m_idxs[0] = ml_1;
    m_idxs[1] = ml_2;
}

/* This function will split the amout to add between two bins (linear interp) 
    based on m_val, but one bin based on f_val. */
inline void
bspline_mi_hist_add (
    Joint_histogram* mi_hist,   /* The histogram */
    float f_val,        /* Intensity of fixed image */
    float m_val,        /* Intensity of moving image */
    float amt               /* How much to add to histogram */
)
{
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    long j_idxs[2];
    long m_idxs[2];
    long f_idxs[1];
    float fxs[2];

    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, mi_hist, 
        f_val, m_val);

    fxs[0] *= amt;
    fxs[1] *= amt;

    f_hist[f_idxs[0]] += amt;       /* This is inefficient */
    m_hist[m_idxs[0]] += fxs[0];
    m_hist[m_idxs[1]] += fxs[1];
    j_hist[j_idxs[0]] += fxs[0];
    j_hist[j_idxs[1]] += fxs[1];
}

/* This algorithm uses a un-normalized score. */
static float
mi_hist_score_omp (Joint_histogram* mi_hist, int num_vox)
{
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

    int f_bin, m_bin, j_bin;
    double fnv = (double) num_vox;
    double score = 0;
    double hist_thresh = 0.001 / (mi_hist->moving.bins * mi_hist->fixed.bins);

    /* Compute cost */
#pragma omp parallel for reduction(-:score)
    for (j_bin=0; j_bin < (mi_hist->fixed.bins * mi_hist->moving.bins); j_bin++) {
        m_bin = j_bin % mi_hist->moving.bins;
        f_bin = j_bin / mi_hist->moving.bins;
        
        if (j_hist[j_bin] > hist_thresh) {
            score -= j_hist[j_bin] * logf(fnv * j_hist[j_bin] / (m_hist[m_bin] * f_hist[f_bin]));
        }
    }

    score = score / fnv;
    return (float) score;
}

inline float
compute_dS_dP (
    double* j_hist, 
    double* f_hist, 
    double* m_hist, 
    long* j_idxs, 
    long* f_idxs, 
    long* m_idxs, 
    float num_vox_f, 
    float* fxs, 
    float score, 
    int debug
)
{
    float dS_dP_0, dS_dP_1, dS_dP;
    const float j_hist_thresh = 0.0001f;

    if (debug) {
        fprintf (stderr, "j=[%ld %ld] (%g %g), "
            "f=[%ld] (%g), "
            "m=[%ld %ld] (%g %g), "
            "fxs = (%g %g)\n",
            j_idxs[0], j_idxs[1], j_hist[j_idxs[0]], j_hist[j_idxs[1]],
            f_idxs[0], f_hist[f_idxs[0]],
            m_idxs[0], m_idxs[1], m_hist[m_idxs[0]], m_hist[m_idxs[1]],
            fxs[0], fxs[1]);
    }

    if (j_hist[j_idxs[0]] < j_hist_thresh) {
        dS_dP_0 = 0.0f;
    } else {
        dS_dP_0 = fxs[0] * (logf((num_vox_f * j_hist[j_idxs[0]]) / (f_hist[f_idxs[0]] * m_hist[m_idxs[0]])) - score);
    }
    if (j_hist[j_idxs[1]] < j_hist_thresh) {
        dS_dP_1 = 0.0f;
    } else {
        dS_dP_1 = fxs[1] * (logf((num_vox_f * j_hist[j_idxs[1]]) / (f_hist[f_idxs[0]] * m_hist[m_idxs[1]])) - score);
    }

    dS_dP = dS_dP_0 + dS_dP_1;
    if (debug) {
        fprintf (stderr, "dS_dP %g = %g %g\n", dS_dP, dS_dP_0, dS_dP_1);
    }

    return dS_dP;
}

/* Use 1 histogram per thread so we are
 * thread safe when employing multi-core */
#if (OPENMP_FOUND)
static inline void
bspline_mi_hist_add_pvi_8_omp_v2 (
    Joint_histogram* mi_hist, 
    double* f_hist_omp,
    double* m_hist_omp,
    double* j_hist_omp,
    Volume *fixed, 
    Volume *moving, 
    int fidx, 
    int midx_f, 
    float li_1[3],           /* Fraction of interpolant in lower index */
    float li_2[3],           /* Fraction of interpolant in upper index */
    int thread_num)
{
    float w[8];
    int n[8];
    plm_long idx_fbin, idx_mbin, idx_jbin, idx_pv;
    int offset_fbin;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;


    /* Compute partial volumes from trilinear interpolation weights */
    w[0] = li_1[0] * li_1[1] * li_1[2]; // Partial Volume w0
    w[1] = li_2[0] * li_1[1] * li_1[2]; // Partial Volume w1
    w[2] = li_1[0] * li_2[1] * li_1[2]; // Partial Volume w2
    w[3] = li_2[0] * li_2[1] * li_1[2]; // Partial Volume w3
    w[4] = li_1[0] * li_1[1] * li_2[2]; // Partial Volume w4
    w[5] = li_2[0] * li_1[1] * li_2[2]; // Partial Volume w5
    w[6] = li_1[0] * li_2[1] * li_2[2]; // Partial Volume w6
    w[7] = li_2[0] * li_2[1] * li_2[2]; // Partial Volume w7

    /* Note that Sum(wN) for N within [0,7] should = 1 */

    // Calculate Point Indices for 8 neighborhood
    n[0] = midx_f;
    n[1] = n[0] + 1;
    n[2] = n[0] + moving->dim[0];
    n[3] = n[2] + 1;
    n[4] = n[0] + moving->dim[0]*moving->dim[1];
    n[5] = n[4] + 1;
    n[6] = n[4] + moving->dim[0];
    n[7] = n[6] + 1;

    // Calculate fixed histogram bin and increment it
    idx_fbin = floor ((f_img[fidx] - mi_hist->fixed.offset) / mi_hist->fixed.delta);

    f_hist_omp[mi_hist->fixed.bins*thread_num + idx_fbin]++;

    offset_fbin = idx_fbin * mi_hist->moving.bins;

    // Add PV weights to moving & joint histograms   
    for (idx_pv=0; idx_pv<8; idx_pv++) {

        idx_mbin = floor ((m_img[n[idx_pv]] - mi_hist->moving.offset) / mi_hist->moving.delta);
        idx_jbin = offset_fbin + idx_mbin;

        if (idx_mbin != mi_hist->moving.big_bin) {
            m_hist_omp[mi_hist->moving.bins*thread_num + idx_mbin] += w[idx_pv];
        }

        if (idx_jbin != mi_hist->joint.big_bin) {
            j_hist_omp[mi_hist->moving.bins*mi_hist->fixed.bins*thread_num + idx_jbin] += w[idx_pv];
        }
    }
}
#endif

/* JAS 2010.11.30
 * This is an intentionally bad idea and will be removed as soon the paper I'm
 * writing sees some ink.
 *
 * Uses CRITICAL SECTIONS instead of locks to make histogram writes thread
 * safe when employing multi-core */
#if (OPENMP_FOUND)
static inline void
bspline_mi_hist_add_pvi_8_omp_crits (
    Joint_histogram* mi_hist, 
    Volume *fixed, 
    Volume *moving, 
    int fidx, 
    int midx_f, 
    float li_1[3],           /* Fraction of interpolant in lower index */
    float li_2[3])           /* Fraction of interpolant in upper index */
{
    float w[8];
    int n[8];
    plm_long idx_fbin, idx_mbin, idx_jbin, idx_pv;
    int offset_fbin;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    double *f_hist = mi_hist->f_hist;
    double *m_hist = mi_hist->m_hist;
    double *j_hist = mi_hist->j_hist;


    /* Compute partial volumes from trilinear interpolation weights */
    w[0] = li_1[0] * li_1[1] * li_1[2]; // Partial Volume w0
    w[1] = li_2[0] * li_1[1] * li_1[2]; // Partial Volume w1
    w[2] = li_1[0] * li_2[1] * li_1[2]; // Partial Volume w2
    w[3] = li_2[0] * li_2[1] * li_1[2]; // Partial Volume w3
    w[4] = li_1[0] * li_1[1] * li_2[2]; // Partial Volume w4
    w[5] = li_2[0] * li_1[1] * li_2[2]; // Partial Volume w5
    w[6] = li_1[0] * li_2[1] * li_2[2]; // Partial Volume w6
    w[7] = li_2[0] * li_2[1] * li_2[2]; // Partial Volume w7

    /* Note that Sum(wN) for N within [0,7] should = 1 */

    // Calculate Point Indices for 8 neighborhood
    n[0] = midx_f;
    n[1] = n[0] + 1;
    n[2] = n[0] + moving->dim[0];
    n[3] = n[2] + 1;
    n[4] = n[0] + moving->dim[0]*moving->dim[1];
    n[5] = n[4] + 1;
    n[6] = n[4] + moving->dim[0];
    n[7] = n[6] + 1;

    // Calculate fixed histogram bin and increment it
    idx_fbin = floor ((f_img[fidx] - mi_hist->fixed.offset) / mi_hist->fixed.delta);

#pragma omp critical (fixed_histogram)
    {
        f_hist[idx_fbin]++;
    }

    offset_fbin = idx_fbin * mi_hist->moving.bins;

    // Add PV weights to moving & joint histograms   
    for (idx_pv=0; idx_pv<8; idx_pv++) {

        idx_mbin = floor ((m_img[n[idx_pv]] 
                - mi_hist->moving.offset) / mi_hist->moving.delta);
        idx_jbin = offset_fbin + idx_mbin;

        if (idx_mbin != mi_hist->moving.big_bin) {
#pragma omp critical (moving_histogram)
            {
                m_hist[idx_mbin] += w[idx_pv];
            }
        }

        if (idx_jbin != mi_hist->joint.big_bin) {
#pragma omp critical (joint_histogram)
            {
                j_hist[idx_jbin] += w[idx_pv];
            }
        }
    }
}
#endif

/* Used locks to make histogram writes
 * thread safe when employing multi-core */
#if (OPENMP_FOUND)
static inline void
bspline_mi_hist_add_pvi_8_omp (
    Joint_histogram* mi_hist, 
    Volume *fixed, 
    Volume *moving, 
    int fidx, 
    int midx_f, 
    float li_1[3],           /* Fraction of interpolant in lower index */
    float li_2[3],           /* Fraction of interpolant in upper index */
    omp_lock_t* f_locks,
    omp_lock_t* m_locks,
    omp_lock_t* j_locks)
{
    float w[8];
    int n[8];
    plm_long idx_fbin, idx_mbin, idx_jbin, idx_pv;
    int offset_fbin;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    double *f_hist = mi_hist->f_hist;
    double *m_hist = mi_hist->m_hist;
    double *j_hist = mi_hist->j_hist;


    /* Compute partial volumes from trilinear interpolation weights */
    w[0] = li_1[0] * li_1[1] * li_1[2]; // Partial Volume w0
    w[1] = li_2[0] * li_1[1] * li_1[2]; // Partial Volume w1
    w[2] = li_1[0] * li_2[1] * li_1[2]; // Partial Volume w2
    w[3] = li_2[0] * li_2[1] * li_1[2]; // Partial Volume w3
    w[4] = li_1[0] * li_1[1] * li_2[2]; // Partial Volume w4
    w[5] = li_2[0] * li_1[1] * li_2[2]; // Partial Volume w5
    w[6] = li_1[0] * li_2[1] * li_2[2]; // Partial Volume w6
    w[7] = li_2[0] * li_2[1] * li_2[2]; // Partial Volume w7

    /* Note that Sum(wN) for N within [0,7] should = 1 */

    // Calculate Point Indices for 8 neighborhood
    n[0] = midx_f;
    n[1] = n[0] + 1;
    n[2] = n[0] + moving->dim[0];
    n[3] = n[2] + 1;
    n[4] = n[0] + moving->dim[0]*moving->dim[1];
    n[5] = n[4] + 1;
    n[6] = n[4] + moving->dim[0];
    n[7] = n[6] + 1;

    // Calculate fixed histogram bin and increment it
    idx_fbin = floor ((f_img[fidx] - mi_hist->fixed.offset) / mi_hist->fixed.delta);
    if (mi_hist->fixed.type == HIST_VOPT) {
        idx_fbin = mi_hist->fixed.key_lut[idx_fbin];
    }

    omp_set_lock(&f_locks[idx_fbin]);
    f_hist[idx_fbin]++;
    omp_unset_lock(&f_locks[idx_fbin]);

    offset_fbin = idx_fbin * mi_hist->moving.bins;

    // Add PV weights to moving & joint histograms   
    for (idx_pv=0; idx_pv<8; idx_pv++) {

        idx_mbin = floor ((m_img[n[idx_pv]] - mi_hist->moving.offset) / mi_hist->moving.delta);
        if (mi_hist->moving.type == HIST_VOPT) {
            idx_mbin = mi_hist->moving.key_lut[idx_mbin];
        }
        idx_jbin = offset_fbin + idx_mbin;

        if (idx_mbin != mi_hist->moving.big_bin) {
            omp_set_lock(&m_locks[idx_mbin]);
            m_hist[idx_mbin] += w[idx_pv];
            omp_unset_lock(&m_locks[idx_mbin]);
        }

        if (idx_jbin != mi_hist->joint.big_bin) {
            omp_set_lock(&j_locks[idx_jbin]);
            j_hist[idx_jbin] += w[idx_pv];
            omp_unset_lock(&j_locks[idx_jbin]);
        }
    }
}
#endif

#if defined (commentout)
static inline void
bspline_mi_hist_add_pvi_6 (
    Joint_histogram* mi_hist, 
    Volume *fixed, 
    Volume *moving, 
    int fidx, 
    int midx_f, 
    float mijk[3]
)
{
    long miqs[3], mjqs[3], mkqs[3]; /* Rounded indices */
    float fxqs[3], fyqs[3], fzqs[3];    /* Fractional values */
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    const float ONE_THIRD = 1.0f / 3.0f;

    /* Compute quadratic interpolation fractions */
    clamp_quadratic_interpolate_inline (mijk[0], moving->dim[0], miqs, fxqs);
    clamp_quadratic_interpolate_inline (mijk[1], moving->dim[1], mjqs, fyqs);
    clamp_quadratic_interpolate_inline (mijk[2], moving->dim[2], mkqs, fzqs);

#if 0
    printf ("[%d %d %d], [%d %d %d], [%d %d %d]\n",
    miqs[0], miqs[1], miqs[2],
    mjqs[0], mjqs[1], mjqs[2],
    mkqs[0], mkqs[1], mkqs[2]
    );
    printf ("[%f %f %f], [%f %f %f], [%f %f %f]\n",
    fxqs[0], fxqs[1], fxqs[2], 
    fyqs[0], fyqs[1], fyqs[2], 
    fzqs[0], fzqs[1], fzqs[2]
    );
#endif

    /* PARTIAL VALUE INTERPOLATION - 6 neighborhood */
    midx_f = (mkqs[1] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_add (mi_hist, f_img[fidx], m_img[midx_f], ONE_THIRD * (fxqs[1] + fyqs[1] + fzqs[1]));
    midx_f = (mkqs[1] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[0];
    bspline_mi_hist_add (mi_hist, f_img[fidx], m_img[midx_f], ONE_THIRD * fxqs[0]);
    midx_f = (mkqs[1] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[2];
    bspline_mi_hist_add (mi_hist, f_img[fidx], m_img[midx_f], ONE_THIRD * fxqs[2]);
    midx_f = (mkqs[1] * moving->dim[1] + mjqs[0]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_add (mi_hist, f_img[fidx], m_img[midx_f], ONE_THIRD * fyqs[0]);
    midx_f = (mkqs[1] * moving->dim[1] + mjqs[2]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_add (mi_hist, f_img[fidx], m_img[midx_f], ONE_THIRD * fyqs[2]);
    midx_f = (mkqs[0] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_add (mi_hist, f_img[fidx], m_img[midx_f], ONE_THIRD * fzqs[0]);
    midx_f = (mkqs[2] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_add (mi_hist, f_img[fidx], m_img[midx_f], ONE_THIRD * fzqs[2]);
}
#endif

/* -----------------------------------------------------------------------
   bspline_mi_pvi_8_dc_dv
   bspline_mi_pvi_6_dc_dv

   Compute pixel contribution to gradient based on histogram change

   There are 6 or 8 correspondences between fixed and moving.  
   Each of these correspondences will update 2 or 3 histogram bins
   (other options are possible, but this is my current strategy).

   First, figure out which histogram bins this correspondence influences
   by calling bspline_mi_hist_lookup().

   Next, compute dS/dP * dP/dx. 
   dP/dx is zero outside of the 8 neighborhood.  Otherwise it is +/- 1/pixel size.
   dS/dP is 1/N (ln (N P / Pf Pm) - I)
   dS/dP is weighted by the relative contribution of the 2 histogram bins.

   dS_dx and dc_dv are really the same thing.  Just different notation.

   For dc_dv:
   We do -= instead of += because our optimizer wants to minimize 
   instead of maximize.
   The right hand side is - for "left" voxels, and + for "right" voxels.

   Use a hard threshold on j_hist[j_idxs] to prevent overflow.  This test 
   should be reconsidered, because it is theoretically unsound.

   Some trivial speedups for the future:
   The pixel size component is constant, so we can post-multiply.
   1/N is constant, so post-multiply.
   ----------------------------------------------------------------------- */
static inline void
bspline_mi_pvi_8_dc_dv (
    float dc_dv[3],                /* Output */
    Joint_histogram* mi_hist,      /* Input */
    Bspline_state *bst,            /* Input */
    Volume *fixed,                 /* Input */
    Volume *moving,                /* Input */
    int fidx,                        /* Input */
    int midx_f,                       /* Input */
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
    int idx_fbin, idx_mbin, idx_jbin, idx_pv;
    int offset_fbin;
    int n[8];
    float dw[24];

    dc_dv[0] = dc_dv[1] = dc_dv[2] = 0.0f;

    /* Calculate Point Indices for 8 neighborhood */
    n[0] = midx_f;
    n[1] = n[0] + 1;
    n[2] = n[0] + moving->dim[0];
    n[3] = n[2] + 1;
    n[4] = n[0] + moving->dim[0]*moving->dim[1];
    n[5] = n[4] + 1;
    n[6] = n[4] + moving->dim[0];
    n[7] = n[6] + 1;

    /* Pre-compute differential PV slices */
    dw[3*0+0] = (  -1 ) * li_1[1] * li_1[2];    // dw0
    dw[3*0+1] = li_1[0] * (  -1 ) * li_1[2];
    dw[3*0+2] = li_1[0] * li_1[1] * (  -1 );

    dw[3*1+0] = (  +1 ) * li_1[1] * li_1[2];    // dw1
    dw[3*1+1] = li_2[0] * (  -1 ) * li_1[2];
    dw[3*1+2] = li_2[0] * li_1[1] * (  -1 );

    dw[3*2+0] = (  -1 ) * li_2[1] * li_1[2];    // dw2
    dw[3*2+1] = li_1[0] * (  +1 ) * li_1[2];
    dw[3*2+2] = li_1[0] * li_2[1] * (  -1 );

    dw[3*3+0] = (  +1 ) * li_2[1] * li_1[2];    // dw3
    dw[3*3+1] = li_2[0] * (  +1 ) * li_1[2];
    dw[3*3+2] = li_2[0] * li_2[1] * (  -1 );

    dw[3*4+0] = (  -1 ) * li_1[1] * li_2[2];    // dw4
    dw[3*4+1] = li_1[0] * (  -1 ) * li_2[2];
    dw[3*4+2] = li_1[0] * li_1[1] * (  +1 );

    dw[3*5+0] = (  +1 ) * li_1[1] * li_2[2];    // dw5
    dw[3*5+1] = li_2[0] * (  -1 ) * li_2[2];
    dw[3*5+2] = li_2[0] * li_1[1] * (  +1 );

    dw[3*6+0] = (  -1 ) * li_2[1] * li_2[2];    // dw6
    dw[3*6+1] = li_1[0] * (  +1 ) * li_2[2];
    dw[3*6+2] = li_1[0] * li_2[1] * (  +1 );

    dw[3*7+0] = (  +1 ) * li_2[1] * li_2[2];    // dw7
    dw[3*7+1] = li_2[0] * (  +1 ) * li_2[2];
    dw[3*7+2] = li_2[0] * li_2[1] * (  +1 );


    /* Fixed image voxel's histogram index */
    idx_fbin = floor ((f_img[fidx] - mi_hist->fixed.offset) 
        / mi_hist->fixed.delta);
    if (mi_hist->fixed.type == HIST_VOPT) {
        idx_fbin = mi_hist->fixed.key_lut[idx_fbin];
    }
    offset_fbin = idx_fbin * mi_hist->moving.bins;

    /* Partial Volume Contributions */
    for (idx_pv=0; idx_pv<8; idx_pv++) {
        idx_mbin = floor ((m_img[n[idx_pv]] - mi_hist->moving.offset) 
            / mi_hist->moving.delta);
        if (mi_hist->moving.type == HIST_VOPT) {
            idx_mbin = mi_hist->moving.key_lut[idx_mbin];
        }
        idx_jbin = offset_fbin + idx_mbin;
        if (j_hist[idx_jbin] > 0.0001) {
            dS_dP = logf((num_vox_f * j_hist[idx_jbin]) 
                / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
            dc_dv[0] -= dw[3*idx_pv+0] * dS_dP;
            dc_dv[1] -= dw[3*idx_pv+1] * dS_dP;
            dc_dv[2] -= dw[3*idx_pv+2] * dS_dP;
        }
    }

    dc_dv[0] = dc_dv[0] / num_vox_f / moving->spacing[0];
    dc_dv[1] = dc_dv[1] / num_vox_f / moving->spacing[1];
    dc_dv[2] = dc_dv[2] / num_vox_f / moving->spacing[2];

#if defined (commentout)
    for (idx_pv=0; idx_pv<8; idx_pv++) {
        printf ("dw%i [ %2.5f %2.5f %2.5f ]\n", idx_pv, dw[3*idx_pv+0], dw[3*idx_pv+1], dw[3*idx_pv+2]);
    }

    printf ("S [ %2.5f %2.5f %2.5f ]\n\n\n", dc_dv[0], dc_dv[1], dc_dv[2]);
    exit(0);
#endif
}



static inline void
bspline_mi_pvi_6_dc_dv (
    float dc_dv[3],                /* Output */
    Joint_histogram* mi_hist,      /* Input */
    Bspline_state *bst,            /* Input */
    Volume *fixed,                 /* Input */
    Volume *moving,                /* Input */
    int fidx,                        /* Input: Index into fixed  image */
    int midx_f,                       /* Input: Index into moving image (unnecessary) */
    float mijk[3],                 /* Input: ijk indices in moving image (vox) */
    float num_vox_f                /* Input: Number of voxels falling into the moving image */
)
{
    long miqs[3], mjqs[3], mkqs[3]; /* Rounded indices */
    float fxqs[3], fyqs[3], fzqs[3];    /* Fractional values */
    long j_idxs[2];
    long m_idxs[2];
    long f_idxs[1];
    float fxs[2];
    float dS_dP;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    Bspline_score* ssd = &bst->ssd;
    int debug = 0;

    dc_dv[0] = dc_dv[1] = dc_dv[2] = 0.0f;

    /* Compute quadratic interpolation fractions */
    clamp_quadratic_interpolate_grad_inline (mijk[0], moving->dim[0], 
        miqs, fxqs);
    clamp_quadratic_interpolate_grad_inline (mijk[1], moving->dim[1], 
        mjqs, fyqs);
    clamp_quadratic_interpolate_grad_inline (mijk[2], moving->dim[2], 
        mkqs, fzqs);

    /* PARTIAL VALUE INTERPOLATION - 6 neighborhood */
    float smetric = ssd->curr_smetric;
    midx_f = (mkqs[1] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, 
        mi_hist, f_img[fidx], m_img[midx_f]);
    dS_dP = compute_dS_dP (j_hist, f_hist, m_hist, j_idxs, f_idxs, m_idxs, 
        num_vox_f, fxs, smetric, debug);
    dc_dv[0] += - fxqs[1] * dS_dP;
    dc_dv[1] += - fyqs[1] * dS_dP;
    dc_dv[2] += - fzqs[1] * dS_dP;

    midx_f = (mkqs[1] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[0];
    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, 
        mi_hist, f_img[fidx], m_img[midx_f]);
    dS_dP = compute_dS_dP (j_hist, f_hist, m_hist, j_idxs, f_idxs, m_idxs, 
        num_vox_f, fxs, smetric, debug);
    dc_dv[0] += - fxqs[0] * dS_dP;

    midx_f = (mkqs[1] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[2];
    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, 
        mi_hist, f_img[fidx], m_img[midx_f]);
    dS_dP = compute_dS_dP (j_hist, f_hist, m_hist, j_idxs, f_idxs, m_idxs, 
        num_vox_f, fxs, smetric, debug);
    dc_dv[0] += - fxqs[2] * dS_dP;

    midx_f = (mkqs[1] * moving->dim[1] + mjqs[0]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, 
        mi_hist, f_img[fidx], m_img[midx_f]);
    dS_dP = compute_dS_dP (j_hist, f_hist, m_hist, j_idxs, f_idxs, m_idxs, 
        num_vox_f, fxs, smetric, debug);
    dc_dv[1] += - fyqs[0] * dS_dP;

    midx_f = (mkqs[1] * moving->dim[1] + mjqs[2]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, 
        mi_hist, f_img[fidx], m_img[midx_f]);
    dS_dP = compute_dS_dP (j_hist, f_hist, m_hist, j_idxs, f_idxs, m_idxs, 
        num_vox_f, fxs, smetric, debug);
    dc_dv[1] += - fyqs[2] * dS_dP;

    midx_f = (mkqs[0] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, 
        mi_hist, f_img[fidx], m_img[midx_f]);
    dS_dP = compute_dS_dP (j_hist, f_hist, m_hist, j_idxs, f_idxs, m_idxs, 
        num_vox_f, fxs, smetric, debug);
    dc_dv[2] += - fzqs[0] * dS_dP;

    midx_f = (mkqs[2] * moving->dim[1] + mjqs[1]) * moving->dim[0] + miqs[1];
    bspline_mi_hist_lookup (j_idxs, m_idxs, f_idxs, fxs, 
        mi_hist, f_img[fidx], m_img[midx_f]);
    dS_dP = compute_dS_dP (j_hist, f_hist, m_hist, j_idxs, f_idxs, m_idxs, 
        num_vox_f, fxs, smetric, debug);
    dc_dv[2] += - fzqs[2] * dS_dP;

    dc_dv[0] = dc_dv[0] / moving->spacing[0] / num_vox_f;
    dc_dv[1] = dc_dv[1] / moving->spacing[1] / num_vox_f;
    dc_dv[2] = dc_dv[2] / moving->spacing[2] / num_vox_f;
}
    

/* -----------------------------------------------------------------------
   Scoring functions
  ----------------------------------------------------------------------- */

/* B-Spline Registration using Mutual Information
 * Implementation I
 *   -- Histograms are OpenMP accelerated
 *   -- Uses OpenMP for Cost & dc_dv computation
 *   -- Uses methods introduced in bspline_score_g_mse
 *        to compute dc_dp more rapidly.
 */
#if (OPENMP_FOUND)
void
bspline_score_i_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;

    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();
    long pidx;
    float num_vox_f;

    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    double mhis = 0.0f;      /* Moving histogram incomplete sum */
    double jhis = 0.0f;      /* Joint  histogram incomplete sum */

    int num_threads;
    double* f_hist_omp = NULL;
    double* m_hist_omp = NULL;
    double* j_hist_omp = NULL;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

#pragma omp parallel
#pragma omp master
    {
        num_threads = omp_get_num_threads ();
        f_hist_omp = (double*) malloc (num_threads * sizeof (double) * mi_hist->fixed.bins);
        m_hist_omp = (double*) malloc (num_threads * sizeof (double) * mi_hist->moving.bins);
        j_hist_omp = (double*) malloc (num_threads * sizeof (double) * mi_hist->fixed.bins * mi_hist->moving.bins);
        memset (f_hist_omp, 0, num_threads * sizeof (double) * mi_hist->fixed.bins);
        memset (m_hist_omp, 0, num_threads * sizeof (double) * mi_hist->moving.bins);
        memset (j_hist_omp, 0, num_threads * sizeof (double) * mi_hist->fixed.bins * mi_hist->moving.bins);
    }

    /* PASS 1 - Accumulate histogram */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */

        int thread_num = omp_get_thread_num ();

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct coordinates into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);
                    
                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    GET_REAL_SPACE_COORDS (fxyz, fijk, bxf);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Add to histogram */

                    bspline_mi_hist_add_pvi_8_omp_v2 (
                        mi_hist, f_hist_omp, m_hist_omp, j_hist_omp,
                        fixed, moving, fidx, midx_f, li_1, li_2, thread_num);
                }
            }
        }   // tile
    }   // openmp

    /* Merge the OpenMP histogram copies */
    for (plm_long b=0; b<mi_hist->fixed.bins; b++) {
        for (int c=0; c<num_threads; c++) {
            f_hist[b] += f_hist_omp[c*mi_hist->fixed.bins + b];
        }
    }
    for (plm_long b=0; b<mi_hist->moving.bins; b++) {
        for (int c=0; c<num_threads; c++) {
            m_hist[b] += m_hist_omp[c*mi_hist->moving.bins + b];
        }
    }
    for (plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for (plm_long i=0; i<mi_hist->moving.bins; i++) {
            for (int c=0; c<num_threads; c++) {
                j_hist[j*mi_hist->moving.bins+i] += j_hist_omp[c*mi_hist->moving.bins*mi_hist->fixed.bins + j*mi_hist->moving.bins + i];
            }
        }
    }

    /* Compute num_vox and find fullest fixed hist bin */
    for (plm_long i=0; i<mi_hist->fixed.bins; i++) {
        if (f_hist[i] > f_hist[mi_hist->fixed.big_bin]) {
            mi_hist->fixed.big_bin = i;
        }
        ssd->curr_num_vox += f_hist[i];
    }

    /* Fill in the missing histogram bin */
    for (plm_long i=0; i<mi_hist->moving.bins; i++) {
        mhis += m_hist[i];
    }
    m_hist[mi_hist->moving.big_bin] = (double) ssd->curr_num_vox - mhis;


    /* Look for the biggest moving histogram bin */
//    printf ("moving.big_bin [%i -> ", mi_hist->moving.big_bin);
    for (plm_long i=0; i<mi_hist->moving.bins; i++) {
        if (m_hist[i] > m_hist[mi_hist->moving.big_bin]) {
            mi_hist->moving.big_bin = i;
        }
    }
//    printf ("%i]\n", mi_hist->moving.big_bin);


    /* Fill in the missing jnt hist bin */
    for (plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for (plm_long i=0; i<mi_hist->moving.bins; i++) {
            jhis += j_hist[j*mi_hist->moving.bins + i];
        }
    }
    j_hist[mi_hist->joint.big_bin] = (double) ssd->curr_num_vox - jhis;

    
    /* Look for the biggest joint histogram bin */
//    printf ("joint.big_bin [%i -> ", mi_hist->joint.big_bin);
    for (plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for (plm_long i=0; i<mi_hist->moving.bins; i++) {
            if (j_hist[j*mi_hist->moving.bins + i] > j_hist[mi_hist->joint.big_bin]) {
                mi_hist->joint.big_bin = j*mi_hist->moving.bins + i;
            }
        }
    }
//    printf ("%i]\n", mi_hist->joint.big_bin);
    


    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Display histrogram stats in debug mode */
    if (parms->debug) {
        double tmp;
        long zz;
        for (zz=0,tmp=0; zz < mi_hist->fixed.bins; zz++) {
            tmp += f_hist[zz];
        }
        printf ("f_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins; zz++) {
            tmp += m_hist[zz];
        }
        printf ("m_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins * mi_hist->fixed.bins; zz++) {
            tmp += j_hist[zz];
        }
        printf ("j_hist total: %f\n", tmp);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist_score_omp (mi_hist, ssd->curr_num_vox);
    num_vox_f = (float) ssd->curr_num_vox;

    /* PASS 2 - Compute Gradient (Parallel across tiles) */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */
        float dc_dv[3];
        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct coordinates into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);
                    
                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    GET_REAL_SPACE_COORDS (fxyz, fijk, bxf);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Compute dc_dv */
                    bspline_mi_pvi_8_dc_dv (dc_dv, mi_hist, bst, fixed, moving, 
                        fidx, midx_f, mijk, num_vox_f, li_1, li_2);

                    /* Update condensed tile sets */
                    bspline_update_sets_b (sets_x, sets_y, sets_z,
                        q, dc_dv, bxf);
                }
            }
        }   // tile

        /* We now have a tile of condensed dc_dv values (64 values).
         * Let's put each one in the proper slot within the control
         * point bin its belogs to */
        bspline_sort_sets (cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf);
    }   // openmp

    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result with be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z,
        bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

#if 0
    if (parms->debug) {
        fclose (fp);
    }
#endif

    mse_score = mse_score / ssd->curr_num_vox;
}
#endif



/* B-Spline Registration using Mutual Information
 * Implementation H  (D w/ direction cosines & true roi support)
 *   -- Uses OpenMP for Cost & dc_dv computation
 *   -- Uses methods introduced in bspline_score_g_mse
 *        to compute dc_dp more rapidly.
 */
void
bspline_score_h_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;

    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();
    float diff;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    float num_vox_f;
    long pidx;
    float m_val;

    plm_long fijk[3] = {0};
    plm_long fidx;
    float mijk[3];
    float fxyz[3];
    float mxyz[3];
    plm_long mijk_f[3], midx_f;      /* Floor */
    plm_long mijk_r[3];           /* Round */
    plm_long p[3];
    plm_long q[3];
    float dxyz[3];
    float li_1[3];           /* Fraction of interpolant in lower index */
    float li_2[3];           /* Fraction of interpolant in upper index */

    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

    Volume* fixed_roi  = bst->fixed_roi;
    Volume* moving_roi = bst->moving_roi;

#if 0
    Volume* dbg_vol = volume_clone_empty (moving);
    float* dbg_img = (float*) dbg_vol->img;
#endif

    FILE* fp = 0;
    static int it = 0;
    if (parms->debug) {
        char buf[1024];
        sprintf (buf, "dump_mi_%02d.txt", it);
        std::string fn = parms->debug_dir + "/" + buf;
        fp = plm_fopen (fn.c_str(), "wb");
        it ++;
    }

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    /* PASS 1 - Accumulate histogram */
    LOOP_Z (fijk, fxyz, fixed) {
        p[2] = REGION_INDEX_Z (fijk, bxf);
        q[2] = REGION_OFFSET_Z (fijk, bxf);
        LOOP_Y (fijk, fxyz, fixed) {
            p[1] = REGION_INDEX_Y (fijk, bxf);
            q[1] = REGION_OFFSET_Y (fijk, bxf);
            LOOP_X (fijk, fxyz, fixed) {
                int rc;
                p[0] = REGION_INDEX_X (fijk, bxf);
                q[0] = REGION_OFFSET_X (fijk, bxf);

                /* JAS 2012.03.26: Tends to break the optimizer (PGTOL)   */
                /* Check to make sure the indices are valid (inside roi) */
                if (fixed_roi) {
                    if (!inside_roi (fxyz, fixed_roi)) continue;
                }

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                bspline_interp_pix_c (dxyz, bxf, pidx, q);

                rc = bspline_find_correspondence_dcos_roi (
                    mxyz, mijk, fxyz, dxyz, moving, moving_roi);

                /* If voxel is not inside moving image */
                if (!rc) continue;

                if (parms->debug) {
                    fprintf (fp, "[%i %i %i]\t->[%f %f %f]\n",
                        (int) fijk[0], (int) fijk[1], (int) fijk[2],
                        mijk[0], mijk[1], mijk[2]);
                }

                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of fixed image voxel */
                fidx = volume_index (fixed->dim, fijk);

                /* Find linear index of "corner voxel" in moving image */
                midx_f = volume_index (moving->dim, mijk_f);

                /* Compute moving image intensity using linear interpolation */
                // NOTE: Not used by MI PVI8
                LI_VALUE (m_val, 
                    li_1[0], li_2[0],
                    li_1[1], li_2[1],
                    li_1[2], li_2[2],
                    midx_f, m_img, moving
                );

                /* PARTIAL VALUE INTERPOLATION - 8 neighborhood */
                mi_hist->add_pvi_8 (
                    fixed, moving, fidx, midx_f, li_1, li_2);

                /* Compute intensity difference */
                diff = m_val - f_img[fidx];
                mse_score += diff * diff;
                ssd->curr_num_vox ++;

            } /* LOOP_THRU_ROI_X */
        } /* LOOP_THRU_ROI_Y */
    } /* LOOP_TRHU_ROI_Z */

#if 0
    write_mha ("dbg_vol.mha", dbg_vol);
    exit (0);
#endif

    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist_score_omp (mi_hist, ssd->curr_num_vox);
    num_vox_f = (float) ssd->curr_num_vox;

    /* PASS 2 - Compute Gradient (Parallel across tiles) */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */
        float dc_dv[3];
        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct ijk indices into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);

                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    POSITION_FROM_COORDS (fxyz, fijk, bxf->img_origin, 
                        fixed->step);

                    /* JAS 2012.03.26: Tends to break the optimizer (PGTOL)   */
                    /* Check to make sure the indices are valid (inside roi) */
                    if (fixed_roi) {
                        if (!inside_roi (fxyz, fixed_roi)) continue;
                    }

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos_roi (
                        mxyz, mijk, fxyz, dxyz, moving, moving_roi);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Construct the fixed image linear index within 
                       volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Compute dc_dv */
                    bspline_mi_pvi_8_dc_dv_dcos (
                        dc_dv, mi_hist, bst,
                        fixed, moving, 
                        fidx, midx_f, 
                        num_vox_f, li_1, li_2
                    );

                    /* Update condensed tile sets */
                    bspline_update_sets_b (
                        sets_x, sets_y, sets_z,
                        q, dc_dv, bxf
                    );

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */


        /* We now have a tile of condensed dc_dv values (64 values).
         * Let's put each one in the proper slot within the control
         * point bin its belogs to */
        bspline_sort_sets (cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf);

    } /* LOOP_THRU_VOL_TILES */


    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result will be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z, bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

    if (parms->debug) {
        fclose (fp);
    }

    mse_score = mse_score / ssd->curr_num_vox;
    if (parms->debug) {
        printf ("<< MSE %3.3f >>\n", mse_score);
    }
}


/* B-Spline Registration using Mutual Information
 * Implementation G  (D w/ direction cosines)
 *   -- Uses OpenMP for Cost & dc_dv computation
 *   -- Uses methods introduced in bspline_score_g_mse
 *        to compute dc_dp more rapidly.
 */
void
bspline_score_g_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;

    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();
    float diff;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    float num_vox_f;
    long pidx;
    float m_val;

    plm_long fijk[3] = {0};
    plm_long fidx;
    float mijk[3];
    float fxyz[3];
    float mxyz[3];
    plm_long mijk_f[3], midx_f;      /* Floor */
    plm_long mijk_r[3];           /* Round */
    plm_long p[3];
    plm_long q[3];
    float dxyz[3];
    float li_1[3];           /* Fraction of interpolant in lower index */
    float li_2[3];           /* Fraction of interpolant in upper index */

    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

#if 0
    FILE* fp = 0;
    char debug_fn[1024];
    static int it = 0;
    if (parms->debug) {
        sprintf (debug_fn, "dump_mi_%02d.txt", it++);
        fp = fopen (debug_fn, "w");
    }
#endif

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    /* PASS 1 - Accumulate histogram */
    LOOP_Z (fijk, fxyz, fixed) {
        p[2] = REGION_INDEX_Z (fijk, bxf);
        q[2] = REGION_OFFSET_Z (fijk, bxf);
        LOOP_Y (fijk, fxyz, fixed) {
            p[1] = REGION_INDEX_Y (fijk, bxf);
            q[1] = REGION_OFFSET_Y (fijk, bxf);
            LOOP_X (fijk, fxyz, fixed) {
                int rc;
                p[0] = REGION_INDEX_X (fijk, bxf);
                q[0] = REGION_OFFSET_X (fijk, bxf);

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                bspline_interp_pix_c (dxyz, bxf, pidx, q);

                rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                /* If voxel is not inside moving image */
                if (!rc) continue;

                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of fixed image voxel */
                fidx = volume_index (fixed->dim, fijk);

                /* Find linear index of "corner voxel" in moving image */
                midx_f = volume_index (moving->dim, mijk_f);

                /* Compute moving image intensity using linear interpolation */
                // NOTE: Not used by MI PVI8
                LI_VALUE (m_val, 
                    li_1[0], li_2[0],
                    li_1[1], li_2[1],
                    li_1[2], li_2[2],
                    midx_f, m_img, moving
                );

                /* PARTIAL VALUE INTERPOLATION - 8 neighborhood */
                mi_hist->add_pvi_8 (
                    fixed, moving, fidx, midx_f, li_1, li_2);

                /* Compute intensity difference */
                diff = m_val - f_img[fidx];
                mse_score += diff * diff;
                ssd->curr_num_vox ++;

            } /* LOOP_THRU_ROI_X */
        } /* LOOP_THRU_ROI_Y */
    } /* LOOP_TRHU_ROI_Z */

    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist_score_omp (mi_hist, ssd->curr_num_vox);
    num_vox_f = (float) ssd->curr_num_vox;

    /* PASS 2 - Compute Gradient (Parallel across tiles) */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */
        float dc_dv[3];
        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct ijk indices into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);

                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    POSITION_FROM_COORDS (fxyz, fijk, bxf->img_origin, 
                        fixed->step);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Compute dc_dv */
                    bspline_mi_pvi_8_dc_dv_dcos (
                        dc_dv, mi_hist, bst,
                        fixed, moving, 
                        fidx, midx_f, 
                        num_vox_f, li_1, li_2
                    );

                    /* Update condensed tile sets */
                    bspline_update_sets_b (
                        sets_x, sets_y, sets_z,
                        q, dc_dv, bxf
                    );

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */


        /* We now have a tile of condensed dc_dv values (64 values).
         * Let's put each one in the proper slot within the control
         * point bin its belogs to */
        bspline_sort_sets (cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf);

    } /* LOOP_THRU_VOL_TILES */


    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result will be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z, bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

#if 0
    if (parms->debug) {
        fclose (fp);
    }
#endif

    mse_score = mse_score / ssd->curr_num_vox;
    if (parms->debug) {
        printf ("<< MSE %3.3f >>\n", mse_score);
    }
}



/* JAS 2010.11.30
 * This is an intentionally bad idea -- proof of concept code
 * 
 * B-Spline Registration using Mutual Information
 * Implementation F (not good... only for comparison to E)
 *   -- Histograms are OpenMP accelerated
 *      (using CRITICAL SECTIONS... just to show better performance with locks)
 *   -- Uses OpenMP for Cost & dc_dv computation
 *   -- Uses methods introduced in bspline_score_g_mse
 *        to compute dc_dp more rapidly.
 */
#if (OPENMP_FOUND)
void
bspline_score_f_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;

    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();
    int pidx;
    float num_vox_f;

    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    double mhis = 0.0f;      /* Moving histogram incomplete sum */
    double jhis = 0.0f;      /* Joint  histogram incomplete sum */

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

#if 0
    FILE* fp = 0;
    static int it = 0;
    char debug_fn[1024];
    if (parms->debug) {
        sprintf (debug_fn, "dump_mi_%02d.txt", it++);
        fp = fopen (debug_fn, "w");
    }
#endif

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    /* PASS 1 - Accumulate histogram */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct coordinates into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);
                    
                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    GET_REAL_SPACE_COORDS (fxyz, fijk, bxf);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Add to histogram */

                    bspline_mi_hist_add_pvi_8_omp_crits (mi_hist, fixed, moving, 
                        fidx, midx_f, li_1, li_2);
                }
            }
        }   // tile
    }   // openmp

    /* Compute num_vox and find fullest fixed hist bin */
    for (plm_long i=0; i<mi_hist->fixed.bins; i++) {
        if (f_hist[i] > f_hist[mi_hist->fixed.big_bin]) {
            mi_hist->fixed.big_bin = i;
        }
        ssd->curr_num_vox += f_hist[i];
    }

    /* Fill in the missing histogram bin */
    for(plm_long i=0; i<mi_hist->moving.bins; i++) {
        mhis += m_hist[i];
    }
    m_hist[mi_hist->moving.big_bin] = (double) ssd->curr_num_vox - mhis;


    /* Look for the biggest moving histogram bin */
    for(plm_long i=0; i<mi_hist->moving.bins; i++) {
        if (m_hist[i] > m_hist[mi_hist->moving.big_bin]) {
            mi_hist->moving.big_bin = i;
        }
    }


    /* Fill in the missing jnt hist bin */
    for(plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for(plm_long i=0; i<mi_hist->moving.bins; i++) {
            jhis += j_hist[j*mi_hist->moving.bins + i];
        }
    }
    j_hist[mi_hist->joint.big_bin] = (double) ssd->curr_num_vox - jhis;

    
    /* Look for the biggest joint histogram bin */
    for(plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for(plm_long i=0; i<mi_hist->moving.bins; i++) {
            if (j_hist[j*mi_hist->moving.bins + i] > j_hist[mi_hist->joint.big_bin]) {
                mi_hist->joint.big_bin = j*mi_hist->moving.bins + i;
            }
        }
    }

    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Display histrogram stats in debug mode */
    if (parms->debug) {
        plm_long zz;
        double tmp;
        for (zz=0,tmp=0; zz < mi_hist->fixed.bins; zz++) {
            tmp += f_hist[zz];
        }
        printf ("f_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins; zz++) {
            tmp += m_hist[zz];
        }
        printf ("m_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins * mi_hist->fixed.bins; zz++) {
            tmp += j_hist[zz];
        }
        printf ("j_hist total: %f\n", tmp);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist_score_omp (mi_hist, ssd->curr_num_vox);
    num_vox_f = (float) ssd->curr_num_vox;

    /* PASS 2 - Compute Gradient (Parallel across tiles) */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */
        float dc_dv[3];
        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct coordinates into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);
                    
                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    GET_REAL_SPACE_COORDS (fxyz, fijk, bxf);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Compute dc_dv */
                    bspline_mi_pvi_8_dc_dv (dc_dv, mi_hist, bst, fixed, moving, 
                        fidx, midx_f, mijk, num_vox_f, li_1, li_2);

                    /* Update condensed tile sets */
                    bspline_update_sets_b (sets_x, sets_y, sets_z,
                        q, dc_dv, bxf);
                }
            }
        }   // tile

        /* We now have a tile of condensed dc_dv values (64 values).
         * Let's put each one in the proper slot within the control
         * point bin its belogs to */
        bspline_sort_sets (cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf);
    }   // openmp

    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result with be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z,
        bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

#if 0
    if (parms->debug) {
        fclose (fp);
    }
#endif

    mse_score = mse_score / ssd->curr_num_vox;
}
#endif



/* B-Spline Registration using Mutual Information
 * Implementation E (D is still faster)
 *   -- Histograms are OpenMP accelerated
 *      (only good on i7 core? really bad otherwise it seems...)
 *   -- Uses OpenMP for Cost & dc_dv computation
 *   -- Uses methods introduced in bspline_score_g_mse
 *        to compute dc_dp more rapidly.
 */
#if (OPENMP_FOUND)
void
bspline_score_e_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;

    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();
    long pidx;
    float num_vox_f;

    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    double mhis = 0.0f;      /* Moving histogram incomplete sum */
    double jhis = 0.0f;      /* Joint  histogram incomplete sum */
    omp_lock_t *f_locks, *m_locks, *j_locks;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);

#if 0
    FILE* fp = 0;
    char debug_fn[1024];
    static int it = 0;
    if (parms->debug) {
        sprintf (debug_fn, "dump_mi_%02d.txt", it++);
        fp = fopen (debug_fn, "w");
    }
#endif

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    /* -- OpenMP locks for histograms --------------------- */
    f_locks = (omp_lock_t*) malloc (mi_hist->fixed.bins * sizeof(omp_lock_t));
    m_locks = (omp_lock_t*) malloc (mi_hist->moving.bins * sizeof(omp_lock_t));
    j_locks = (omp_lock_t*) malloc (mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(omp_lock_t));

#pragma omp parallel for
    for (plm_long i=0; i < mi_hist->fixed.bins; i++) {
        omp_init_lock(&f_locks[i]);
    }

#pragma omp parallel for
    for (plm_long i=0; i < mi_hist->moving.bins; i++) {
        omp_init_lock(&m_locks[i]);
    }

#pragma omp parallel for
    for (plm_long i=0; i < mi_hist->fixed.bins * mi_hist->moving.bins; i++) {
        omp_init_lock(&j_locks[i]);
    }
    /* ---------------------------------------------------- */

    /* PASS 1 - Accumulate histogram */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct coordinates into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);
                    
                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    GET_REAL_SPACE_COORDS (fxyz, fijk, bxf);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Add to histogram */

                    bspline_mi_hist_add_pvi_8_omp (mi_hist, fixed, moving, 
                        fidx, midx_f, li_1, li_2,
                        f_locks, m_locks, j_locks);
#if defined (commentout)
#endif
                }
            }
        }   // tile
    }   // openmp

    /* Compute num_vox and find fullest fixed hist bin */
    for (plm_long i=0; i<mi_hist->fixed.bins; i++) {
        if (f_hist[i] > f_hist[mi_hist->fixed.big_bin]) {
            mi_hist->fixed.big_bin = i;
        }
        ssd->curr_num_vox += f_hist[i];
    }

    /* Fill in the missing histogram bin */
    for (plm_long i=0; i<mi_hist->moving.bins; i++) {
        mhis += m_hist[i];
    }
    m_hist[mi_hist->moving.big_bin] = (double) ssd->curr_num_vox - mhis;


    /* Look for the biggest moving histogram bin */
//    printf ("moving.big_bin [%i -> ", mi_hist->moving.big_bin);
    for (plm_long i=0; i<mi_hist->moving.bins; i++) {
        if (m_hist[i] > m_hist[mi_hist->moving.big_bin]) {
            mi_hist->moving.big_bin = i;
        }
    }
//    printf ("%i]\n", mi_hist->moving.big_bin);


    /* Fill in the missing jnt hist bin */
    for (plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for (plm_long i=0; i<mi_hist->moving.bins; i++) {
            jhis += j_hist[j*mi_hist->moving.bins + i];
        }
    }
    j_hist[mi_hist->joint.big_bin] = (double) ssd->curr_num_vox - jhis;

    
    /* Look for the biggest joint histogram bin */
//    printf ("joint.big_bin [%i -> ", mi_hist->joint.big_bin);
    for (plm_long j=0; j<mi_hist->fixed.bins; j++) {
        for (plm_long i=0; i<mi_hist->moving.bins; i++) {
            if (j_hist[j*mi_hist->moving.bins + i] > j_hist[mi_hist->joint.big_bin]) {
                mi_hist->joint.big_bin = j*mi_hist->moving.bins + i;
            }
        }
    }
//    printf ("%i]\n", mi_hist->joint.big_bin);
    


    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Display histrogram stats in debug mode */
    if (parms->debug) {
        double tmp;
        long zz;
        for (zz=0,tmp=0; zz < mi_hist->fixed.bins; zz++) {
            tmp += f_hist[zz];
        }
        printf ("f_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins; zz++) {
            tmp += m_hist[zz];
        }
        printf ("m_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins * mi_hist->fixed.bins; zz++) {
            tmp += j_hist[zz];
        }
        printf ("j_hist total: %f\n", tmp);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist_score_omp (mi_hist, ssd->curr_num_vox);
    num_vox_f = (float) ssd->curr_num_vox;

    /* PASS 2 - Compute Gradient (Parallel across tiles) */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */
        float dc_dv[3];
        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));

        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct coordinates into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);
                    
                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    GET_REAL_SPACE_COORDS (fxyz, fijk, bxf);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Compute dc_dv */
                    bspline_mi_pvi_8_dc_dv (dc_dv, mi_hist, bst, fixed, moving, 
                        fidx, midx_f, mijk, num_vox_f, li_1, li_2);

                    /* Update condensed tile sets */
                    bspline_update_sets_b (sets_x, sets_y, sets_z,
                        q, dc_dv, bxf);
                }
            }
        }   // tile

        /* We now have a tile of condensed dc_dv values (64 values).
         * Let's put each one in the proper slot within the control
         * point bin its belogs to */
        bspline_sort_sets (cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf);
    }   // openmp

    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result with be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z,
        bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);


#pragma omp parallel for
    for (long i=0; i < mi_hist->fixed.bins; i++) {
        omp_destroy_lock(&f_locks[i]);
    }

#pragma omp parallel for
    for (long i=0; i < mi_hist->moving.bins; i++) {
        omp_destroy_lock(&m_locks[i]);
    }

#pragma omp parallel for
    for (long i=0; i < mi_hist->fixed.bins * mi_hist->moving.bins; i++) {
        omp_destroy_lock(&j_locks[i]);
    }

#if 0
    if (parms->debug) {
        fclose (fp);
    }
#endif

    mse_score = mse_score / ssd->curr_num_vox;
}
#endif


/* B-Spline Registration using Mutual Information
 * Implementation D
 *   -- Uses OpenMP for Cost & dc_dv computation
 *   -- Uses methods introduced in bspline_score_g_mse
 *        to compute dc_dp more rapidly.
 */
void
bspline_score_d_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;

    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();
    plm_long rijk[3];
    float diff;
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    float num_vox_f;
    long pidx;
    float m_val;

    plm_long fijk[3];
    plm_long fidx;
    float mijk[3];
    float fxyz[3];
    float mxyz[3];
    plm_long mijk_f[3], midx_f;      /* Floor */
    plm_long mijk_r[3];           /* Round */
    plm_long p[3];
    plm_long q[3];
    float dxyz[3];
    float li_1[3];           /* Fraction of interpolant in lower index */
    float li_2[3];           /* Fraction of interpolant in upper index */

    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

    plm_long cond_size = 64*bxf->num_knots*sizeof(float);
    float* cond_x = (float*)malloc(cond_size);
    float* cond_y = (float*)malloc(cond_size);
    float* cond_z = (float*)malloc(cond_size);


#if 0
    FILE* fp = 0;
    char debug_fn[1024];
    static int it = 0;
    if (parms->debug) {
        sprintf (debug_fn, "dump_mi_%02d.txt", it++);
        fp = fopen (debug_fn, "w");
    }
#endif

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));
    memset(cond_x, 0, cond_size);
    memset(cond_y, 0, cond_size);
    memset(cond_z, 0, cond_size);

    /* PASS 1 - Accumulate histogram */
    LOOP_THRU_ROI_Z (rijk, fijk, bxf) {
        p[2] = REGION_INDEX_Z (rijk, bxf);
        q[2] = REGION_OFFSET_Z (rijk, bxf);
        fxyz[2] = GET_WORLD_COORD_Z_NO_DCOS (fijk, bxf);

        LOOP_THRU_ROI_Y (rijk, fijk, bxf) {
            p[1] = REGION_INDEX_Y (rijk, bxf);
            q[1] = REGION_OFFSET_Y (rijk, bxf);
            fxyz[1] = GET_WORLD_COORD_Y_NO_DCOS (fijk, bxf);

            LOOP_THRU_ROI_X (rijk, fijk, bxf) {
                int rc;
                p[0] = REGION_INDEX_X (rijk, bxf);
                q[0] = REGION_OFFSET_X (rijk, bxf);
                fxyz[0] = GET_WORLD_COORD_X_NO_DCOS (fijk, bxf);

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                bspline_interp_pix_c (dxyz, bxf, pidx, q);

                /* Find correspondence in moving image */
                rc = bspline_find_correspondence_dcos (
                    mxyz, mijk,
                    fxyz, dxyz, moving
                );

                /* If voxel is not inside moving image */
                if (!rc) continue;

                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of fixed image voxel */
                fidx = volume_index (fixed->dim, fijk);

                /* Find linear index of "corner voxel" in moving image */
                midx_f = volume_index (moving->dim, mijk_f);

                /* Compute moving image intensity using linear interpolation */
                // NOTE: Not used by MI PVI8
                LI_VALUE (m_val, 
                    li_1[0], li_2[0],
                    li_1[1], li_2[1],
                    li_1[2], li_2[2],
                    midx_f, m_img, moving
                );

#if defined (commentout)
                /* LINEAR INTERPOLATION */
                bspline_mi_hist_add (mi_hist, f_img[fidx], m_val, 1.0);
#endif

                /* PARTIAL VALUE INTERPOLATION - 8 neighborhood */
                mi_hist->add_pvi_8 (
                    fixed, moving, fidx, midx_f, li_1, li_2);

#if defined (commentout)
                /* PARTIAL VALUE INTERPOLATION - 6 neighborhood */
                bspline_mi_hist_add_pvi_6 (
                    mi_hist, fixed, moving, 
                    fidx, midx_f, mijk
                );
#endif

                /* Compute intensity difference */
                diff = m_val - f_img[fidx];
                mse_score += diff * diff;
                ssd->curr_num_vox ++;

            } /* LOOP_THRU_ROI_X */
        } /* LOOP_THRU_ROI_Y */
    } /* LOOP_TRHU_ROI_Z */


    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Display histrogram stats in debug mode */
    if (parms->debug) {
        plm_long zz;
        double tmp;
        for (zz=0,tmp=0; zz < mi_hist->fixed.bins; zz++) {
            tmp += f_hist[zz];
        }
        printf ("f_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins; zz++) {
            tmp += m_hist[zz];
        }
        printf ("m_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins * mi_hist->fixed.bins; zz++) {
            tmp += j_hist[zz];
        }
        printf ("j_hist total: %f\n", tmp);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist_score_omp (mi_hist, ssd->curr_num_vox);
    num_vox_f = (float) ssd->curr_num_vox;

    /* PASS 2 - Compute Gradient (Parallel across tiles) */
#pragma omp parallel for
    LOOP_THRU_VOL_TILES (pidx, bxf) {
        int rc;
        plm_long fijk[3], fidx;
        float mijk[3];
        float fxyz[3];
        float mxyz[3];
        plm_long mijk_f[3], midx_f;      /* Floor */
        plm_long mijk_r[3];           /* Round */
        plm_long p[3];
        plm_long q[3];
        float dxyz[3];
        float li_1[3];           /* Fraction of interpolant in lower index */
        float li_2[3];           /* Fraction of interpolant in upper index */
        float dc_dv[3];
        float sets_x[64];
        float sets_y[64];
        float sets_z[64];

        memset(sets_x, 0, 64*sizeof(float));
        memset(sets_y, 0, 64*sizeof(float));
        memset(sets_z, 0, 64*sizeof(float));


        /* Get tile indices from linear index */
        COORDS_FROM_INDEX (p, pidx, bxf->rdims);

        /* Serial through the voxels in a tile */
        LOOP_THRU_TILE_Z (q, bxf) {
            LOOP_THRU_TILE_Y (q, bxf) {
                LOOP_THRU_TILE_X (q, bxf) {
                    
                    /* Construct coordinates into fixed image volume */
                    GET_VOL_COORDS (fijk, p, q, bxf);
                    
                    /* Check to make sure the indices are valid (inside volume) */
                    if (fijk[0] >= bxf->roi_offset[0] + bxf->roi_dim[0]) { continue; }
                    if (fijk[1] >= bxf->roi_offset[1] + bxf->roi_dim[1]) { continue; }
                    if (fijk[2] >= bxf->roi_offset[2] + bxf->roi_dim[2]) { continue; }

                    /* Compute space coordinates of fixed image voxel */
                    GET_REAL_SPACE_COORDS (fxyz, fijk, bxf);

                    /* Compute deformation vector (dxyz) for voxel */
                    bspline_interp_pix_c (dxyz, bxf, pidx, q);

                    /* Find correspondence in moving image */
                    rc = bspline_find_correspondence_dcos (mxyz, mijk, fxyz, dxyz, moving);

                    /* If voxel is not inside moving image */
                    if (!rc) continue;

                    /* Get tri-linear interpolation fractions */
                    li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);
                    
                    /* Constrcut the fixed image linear index within volume space */
                    fidx = volume_index (fixed->dim, fijk);

                    /* Find linear index the corner voxel used to identifiy the
                     * neighborhood of the moving image voxels corresponding
                     * to the current fixed image voxel */
                    midx_f = volume_index (moving->dim, mijk_f);

                    /* Compute dc_dv */
                    bspline_mi_pvi_8_dc_dv (
                        dc_dv, mi_hist, bst,
                        fixed, moving, 
                        fidx, midx_f, mijk,
                        num_vox_f, li_1, li_2
                    );

                    /* Update condensed tile sets */
                    bspline_update_sets_b (
                        sets_x, sets_y, sets_z,
                        q, dc_dv, bxf
                    );

                } /* LOOP_THRU_TILE_X */
            } /* LOOP_THRU_TILE_Y */
        } /* LOOP_THRU_TILE_Z */


        /* We now have a tile of condensed dc_dv values (64 values).
         * Let's put each one in the proper slot within the control
         * point bin its belogs to */
        bspline_sort_sets (cond_x, cond_y, cond_z,
            sets_x, sets_y, sets_z,
            pidx, bxf);

    } /* LOOP_THRU_VOL_TILES */


    /* Now we have a ton of bins and each bin's 64 slots are full.
     * Let's sum each bin's 64 slots.  The result will be dc_dp. */
    bspline_condense_smetric_grad (cond_x, cond_y, cond_z, bxf, ssd);

    free (cond_x);
    free (cond_y);
    free (cond_z);

#if 0
    if (parms->debug) {
        fclose (fp);
    }
#endif

    mse_score = mse_score / ssd->curr_num_vox;
    if (parms->debug) {
        printf ("<< MSE %3.3f >>\n", mse_score);
    }
}

/* Mutual information version of implementation "C" */
void
bspline_score_c_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_xform *bxf = bod->get_bspline_xform ();

    Volume *fixed = bst->fixed;
    Volume *moving = bst->moving;
    Volume *fixed_roi  = bst->fixed_roi;
    Volume *moving_roi = bst->moving_roi;

    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();
    //plm_long rijk[3];
    plm_long fijk[3], fidx;
    float mijk[3];
    float fxyz[3];
    float mxyz[3];
    plm_long mijk_f[3], midx_f;      /* Floor */
    plm_long mijk_r[3];           /* Round */
    plm_long p[3];
    plm_long q[3];
    float diff;
    float dc_dv[3];
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;
    float dxyz[3];
    float num_vox_f;
    plm_long pidx, qidx;
    float li_1[3];           /* Fraction of interpolant in lower index */
    float li_2[3];           /* Fraction of interpolant in upper index */
    float m_val;

    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

#if 0
    FILE* fp = 0;
    char debug_fn[1024];
    static int it = 0;
    if (parms->debug) {
        sprintf (debug_fn, "dump_mi_%02d.txt", it++);
        fp = fopen (debug_fn, "w");
    }
#endif

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));

    /* PASS 1 - Accumulate histogram */
    LOOP_Z (fijk, fxyz, fixed) {
        p[2] = REGION_INDEX_Z (fijk, bxf);
        q[2] = REGION_OFFSET_Z (fijk, bxf);
        LOOP_Y (fijk, fxyz, fixed) {
            p[1] = REGION_INDEX_Y (fijk, bxf);
            q[1] = REGION_OFFSET_Y (fijk, bxf);
            LOOP_X (fijk, fxyz, fixed) {
                p[0] = REGION_INDEX_X (fijk, bxf);
                q[0] = REGION_OFFSET_X (fijk, bxf);

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

                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of fixed image voxel */
                fidx = volume_index (fixed->dim, fijk);

                /* Find linear index of "corner voxel" in moving image */
                midx_f = volume_index (moving->dim, mijk_f);

                /* Compute moving image intensity using linear interpolation */
                /* Macro is slightly faster than function */
                // NOTE: Not used by MI PVI8
                LI_VALUE (m_val, 
                    li_1[0], li_2[0],
                    li_1[1], li_2[1],
                    li_1[2], li_2[2],
                    midx_f, m_img, moving
                );

#if defined (commentout)
                /* LINEAR INTERPOLATION */
                bspline_mi_hist_add (mi_hist, f_img[fidx], m_val, 1.0);
#endif

                /* PARTIAL VALUE INTERPOLATION - 8 neighborhood */
                mi_hist->add_pvi_8 (
                    fixed, moving, fidx, midx_f, li_1, li_2);

#if defined (commentout)
                /* PARTIAL VALUE INTERPOLATION - 6 neighborhood */
                bspline_mi_hist_add_pvi_6 (
                    mi_hist, fixed, moving, 
                    fidx, midx_f, mijk
                );
#endif

                /* Compute intensity difference */
                diff = m_val - f_img[fidx];
                mse_score += diff * diff;
                ssd->curr_num_vox++;

            } /* LOOP_THRU_ROI_X */
        } /* LOOP_THRU_ROI_Y */
    } /* LOOP_THRU_ROI_Z */

    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Display histrogram stats in debug mode */
    if (parms->debug) {
        plm_long zz;
        double tmp;
        for (zz=0,tmp=0; zz < mi_hist->fixed.bins; zz++) {
            tmp += f_hist[zz];
        }
        printf ("f_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins; zz++) {
            tmp += m_hist[zz];
        }
        printf ("m_hist total: %f\n", tmp);

        for (zz=0,tmp=0;
             zz < mi_hist->moving.bins * mi_hist->fixed.bins; zz++)
        {
            tmp += j_hist[zz];
        }
        printf ("j_hist total: %f\n", tmp);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist->compute_score (ssd->curr_num_vox);
    num_vox_f = (float) ssd->curr_num_vox;

    /* PASS 2 - Compute gradient */
    LOOP_Z (fijk, fxyz, fixed) {
        p[2] = REGION_INDEX_Z (fijk, bxf);
        q[2] = REGION_OFFSET_Z (fijk, bxf);
        LOOP_Y (fijk, fxyz, fixed) {
            p[1] = REGION_INDEX_Y (fijk, bxf);
            q[1] = REGION_OFFSET_Y (fijk, bxf);
            LOOP_X (fijk, fxyz, fixed) {
                p[0] = REGION_INDEX_X (fijk, bxf);
                q[0] = REGION_OFFSET_X (fijk, bxf);

                if (fixed_roi) {
                    if (!inside_roi (fxyz, fixed_roi)) continue;
                }

                /* Get B-spline deformation vector */
                pidx = volume_index (bxf->rdims, p);
                qidx = volume_index (bxf->vox_per_rgn, q);
                bspline_interp_pix_b (dxyz, bxf, pidx, qidx);

                /* Find linear index of fixed image voxel */
                fidx = volume_index (fixed->dim, fijk);

                /* Find correspondence in moving image */
                int rc;
                rc = bspline_find_correspondence_dcos_roi (
                    mxyz, mijk, fxyz, dxyz, moving, moving_roi);

                /* If voxel is not inside moving image */
                if (!rc) continue;

                /* LINEAR INTERPOLATION - (not implemented) */

                /* *** PARTIAL VALUE INTERPOLATION - 8 neighborhood *** */

                /* Get tri-linear interpolation fractions */
                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of fixed image voxel */
                fidx = volume_index (fixed->dim, fijk);

                /* Find linear index of "corner voxel" in moving image */
                midx_f = volume_index (moving->dim, mijk_f);

                /* Compute dc_dv */
                bspline_mi_pvi_8_dc_dv_dcos (
                    dc_dv, mi_hist, bst,
                    fixed, moving, 
                    fidx, midx_f, 
                    num_vox_f, li_1, li_2
                );

#if defined (commentout)
                /* PARTIAL VALUE INTERPOLATION - 6 neighborhood */
                bspline_mi_pvi_6_dc_dv (
                    dc_dv, mi_hist, bst,
                    fixed, moving, fidx, midx_f, mijk,
                    num_vox_f
                );
#endif

                bst->ssd.update_smetric_grad_b (bxf, pidx, qidx, dc_dv);

            } /* LOOP_THRU_ROI_X */
        } /* LOOP_THRU_ROI_Y */
    } /* LOOP_THRU_ROI_Z */

#if 0
    if (parms->debug) {
        fclose (fp);
    }
#endif

    mse_score = mse_score / ssd->curr_num_vox;
}

/* Mutual information version of implementation "k" */
void
bspline_score_k_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();

    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));

    /* Create/initialize bspline_loop_user (PASS 1) */
    Bspline_mi_k_pass_1 blu1 (bod);
    blu1.set_mi_hist (mi_hist);

    /* Run the loop */
    bspline_loop_voxel_serial (blu1, bod);

    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Display histrogram stats in debug mode */
    if (parms->debug) {
        plm_long zz;
        double tmp;
        for (zz=0,tmp=0; zz < mi_hist->fixed.bins; zz++) {
            tmp += f_hist[zz];
        }
        printf ("f_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins; zz++) {
            tmp += m_hist[zz];
        }
        printf ("m_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins * mi_hist->fixed.bins; zz++) {
            tmp += j_hist[zz];
        }
        printf ("j_hist total: %f\n", tmp);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist->compute_score (ssd->curr_num_vox);

    /* Create/initialize bspline_loop_user (PASS 2) */
    Bspline_mi_k_pass_2 blu2 (bod);
    blu2.set_mi_hist (mi_hist);

    /* Run the loop */
    bspline_loop_voxel_serial (blu2, bod);
}

/* Mutual information version of implementation "l" */
void
bspline_score_l_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();
    Bspline_score* ssd = &bst->ssd;
    Joint_histogram* mi_hist = bst->get_mi_hist();

    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;

    memset (f_hist, 0, mi_hist->fixed.bins * sizeof(double));
    memset (m_hist, 0, mi_hist->moving.bins * sizeof(double));
    memset (j_hist, 0, mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(double));

    /* Create/initialize bspline_loop_user (PASS 1) */
    Bspline_mi_k_pass_1 blu1 (bod);
    blu1.set_mi_hist (mi_hist);

    /* Run the loop */
    bspline_loop_voxel_serial (blu1, bod);

    /* Draw histogram images if user wants them */
    if (parms->xpm_hist_dump) {
        dump_xpm_hist (mi_hist, parms->xpm_hist_dump, bst->it);
    }

    /* Display histrogram stats in debug mode */
    if (parms->debug) {
        plm_long zz;
        double tmp;
        for (zz=0,tmp=0; zz < mi_hist->fixed.bins; zz++) {
            tmp += f_hist[zz];
        }
        printf ("f_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins; zz++) {
            tmp += m_hist[zz];
        }
        printf ("m_hist total: %f\n", tmp);

        for (zz=0,tmp=0; zz < mi_hist->moving.bins * mi_hist->fixed.bins; zz++) {
            tmp += j_hist[zz];
        }
        printf ("j_hist total: %f\n", tmp);
    }

    /* Compute score */
    ssd->curr_smetric = mi_hist->compute_score (ssd->curr_num_vox);

    /* Create/initialize bspline_loop_user (PASS 2) */
    Bspline_mi_k_pass_2 blu2 (bod);
    blu2.set_mi_hist (mi_hist);

    /* Run the loop */
    bspline_loop_voxel_serial (blu2, bod);
}

void
bspline_score_mi (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();
    Bspline_state *bst = bod->get_bspline_state ();

    Volume* fixed_roi  = bst->fixed_roi;
    Volume* moving_roi = bst->moving_roi;
    bool have_roi = fixed_roi || moving_roi;
    bool have_histogram_minmax_val = 
        (parms->mi_fixed_image_minVal!=0)
        || (parms->mi_fixed_image_maxVal!=0)
        || (parms->mi_moving_image_minVal!=0)
        || (parms->mi_moving_image_maxVal!=0);

    /* CPU Implementations */
    if (parms->threading == BTHR_CPU) {
            
        /* Metric: Mutual Information with roi or intensity min/max values*/
        if (have_roi || have_histogram_minmax_val) {
            switch (parms->implementation) {
            case 'c':
                bspline_score_c_mi (bod);
                break;
#if (OPENMP_FOUND)
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
                bspline_score_h_mi (bod);
                break;
#endif
            case 'k':
                bspline_score_k_mi (bod);
                break;
            case 'l':
                bspline_score_l_mi (bod);
                break;
            default:
#if (OPENMP_FOUND)
                bspline_score_h_mi (bod);
#else
                bspline_score_c_mi (bod);
#endif
                break;
            }
        }

        /* Metric: Mutual Information without roi */
        else {
            switch (parms->implementation) {
            case 'c':
                bspline_score_c_mi (bod);
                break;
#if (OPENMP_FOUND)
            case 'd':
                bspline_score_d_mi (bod);
                break;
            case 'e':
                bspline_score_e_mi (bod);
                break;
            case 'f':
                bspline_score_f_mi (bod);
                break;
            case 'g':
                bspline_score_g_mi (bod);
                break;
            case 'h':
                bspline_score_h_mi (bod);
                break;
            case 'i':
                bspline_score_i_mi (bod);
                break;
#endif
            case 'k':
                bspline_score_k_mi (bod);
                break;
            case 'l':
                bspline_score_l_mi (bod);
                break;
            default:
#if (OPENMP_FOUND)
                bspline_score_g_mi (bod);
#else
                bspline_score_c_mi (bod);
#endif
                break;
            }
        } /* end MI */

    } /* end CPU Implementations */


    /* CUDA Implementations */
#if (CUDA_FOUND)
    else if (parms->threading == BTHR_CUDA) {
            
        /* Be sure we loaded the CUDA plugin */
        LOAD_LIBRARY_SAFE (libplmregistercuda);
        LOAD_SYMBOL (CUDA_bspline_mi_a, libplmregistercuda);

        switch (parms->implementation) {
        case 'a':
            CUDA_bspline_mi_a (
                bod->get_bspline_parms(),
                bod->get_bspline_state(),
                bod->get_bspline_xform());
            break;
        default:
            CUDA_bspline_mi_a (
                bod->get_bspline_parms(),
                bod->get_bspline_state(),
                bod->get_bspline_xform());
            break;
        }

        UNLOAD_LIBRARY (libplmregistercuda);

    } /* CUDA Implementations */
#endif
}
