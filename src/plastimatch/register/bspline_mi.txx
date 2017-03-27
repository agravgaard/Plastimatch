/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_mi_txx_
#define _bspline_mi_txx_

#include "compiler_warnings.h"

static inline void
bspline_mi_pvi_8_dc_dv_dcos (
    float dc_dv[3],                /* Output */
    Joint_histogram* mi_hist,  /* Input */
    Bspline_state *bst,            /* Input */
    const Volume *fixed,           /* Input */
    const Volume *moving,          /* Input */
    int fidx,                        /* Input */
    int mvf,                       /* Input */
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
    n[0] = mvf;
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
    idx_fbin = floor ((f_img[fidx] - mi_hist->fixed.offset) / mi_hist->fixed.delta);
    if (mi_hist->fixed.type == HIST_VOPT) {
        idx_fbin = mi_hist->fixed.key_lut[idx_fbin];
    }
    offset_fbin = idx_fbin * mi_hist->moving.bins;

    /* Partial Volume Contributions */
    for (idx_pv=0; idx_pv<8; idx_pv++) {
        idx_mbin = floor ((m_img[n[idx_pv]] - mi_hist->moving.offset) / mi_hist->moving.delta);
        if (mi_hist->moving.type == HIST_VOPT) {
            idx_mbin = mi_hist->moving.key_lut[idx_mbin];
        }
        idx_jbin = offset_fbin + idx_mbin;
        if (j_hist[idx_jbin] > 0.0001) {
            dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - ssd->curr_smetric;
            dc_dv[0] -= dw[3*idx_pv+0] * dS_dP;
            dc_dv[1] -= dw[3*idx_pv+1] * dS_dP;
            dc_dv[2] -= dw[3*idx_pv+2] * dS_dP;
        }
    }

    dc_dv[0] = dc_dv[0] / num_vox_f;
    dc_dv[1] = dc_dv[1] / num_vox_f;
    dc_dv[2] = dc_dv[2] / num_vox_f;
    dc_dv[0] = PROJECT_X (dc_dv, moving->proj);
    dc_dv[1] = PROJECT_Y (dc_dv, moving->proj);
    dc_dv[2] = PROJECT_Z (dc_dv, moving->proj);


#if defined (commentout)
    for (idx_pv=0; idx_pv<8; idx_pv++) {
        printf ("dw%i [ %2.5f %2.5f %2.5f ]\n", idx_pv, dw[3*idx_pv+0], dw[3*idx_pv+1], dw[3*idx_pv+2]);
    }

    printf ("S [ %2.5f %2.5f %2.5f ]\n\n\n", dc_dv[0], dc_dv[1], dc_dv[2]);
    exit(0);
#endif
}

class Bspline_mi_k_pass_1
{
public:
    double score_acc;
    Joint_histogram *mi_hist;
public:
    Bspline_mi_k_pass_1 (Bspline_optimize *bod) {
        score_acc = 0.f;
    }
    void set_mi_hist (Joint_histogram *mi_hist) {
        this->mi_hist = mi_hist;
    }
public:
    void
    loop_function (
        Bspline_optimize *bod,    /* In/out: generic optimization data */
        Bspline_xform *bxf,       /* Input:  coefficient values */
        Bspline_state *bst,       /* Input:  state of bspline */
        Bspline_score *ssd,       /* In/out: score and gradient */
        const Volume *fixed,      /* Input:  fixed image */
        const Volume *moving,     /* Input:  moving image */
        const float *f_img,       /* Input:  raw intensity array for fixed */
        const float *m_img,       /* Input:  raw intensity array for moving */
        plm_long fidx,            /* Input:  index of voxel in fixed image */
        plm_long midx_f,          /* Input:  index (floor) in moving image*/
        plm_long mijk_r[3],       /* Input:  coords (rounded) in moving image*/
        plm_long pidx,            /* Input:  region index of fixed voxel */
        plm_long qidx,            /* Input:  offset index of fixed voxel */
        float li_1[3],            /* Input:  linear interpolation fraction */
        float li_2[3]             /* Input:  linear interpolation fraction */
    )
    {
        /* Compute moving image intensity using linear interpolation */
        /* Macro is slightly faster than function */
        float m_val;
        LI_VALUE (m_val, 
            li_1[0], li_2[0],
            li_1[1], li_2[1],
            li_1[2], li_2[2],
            midx_f, m_img, moving);
        UNUSED_VARIABLE (m_val);

        /* PARTIAL VALUE INTERPOLATION - 8 neighborhood */
        mi_hist->add_pvi_8 (
            fixed, moving, 
            fidx, midx_f, li_1, li_2
        );

        /* Keep track of voxels used */
        ssd->curr_num_vox++;
    }
};

class Bspline_mi_k_pass_2
{
public:
    float num_vox_f;
    Joint_histogram *mi_hist;
public:
    Bspline_mi_k_pass_2 (Bspline_optimize *bod) {
        Bspline_score* ssd = bod->get_bspline_state()->get_bspline_score();
        num_vox_f = (float) ssd->curr_num_vox;
    }
    void set_mi_hist (Joint_histogram *mi_hist) {
        this->mi_hist = mi_hist;
    }
public:
    void
    loop_function (
        Bspline_optimize *bod,    /* In/out: generic optimization data */
        Bspline_xform *bxf,       /* Input:  coefficient values */
        Bspline_state *bst,       /* Input:  state of bspline */
        Bspline_score *ssd,       /* In/out: score and gradient */
        const Volume *fixed,      /* Input:  fixed image */
        const Volume *moving,     /* Input:  moving image */
        const float *f_img,       /* Input:  raw intensity array for fixed */
        const float *m_img,       /* Input:  raw intensity array for moving */
        plm_long fidx,            /* Input:  index of voxel in fixed image */
        plm_long midx_f,          /* Input:  index (floor) in moving image*/
        plm_long mijk_r[3],       /* Input:  coords (rounded) in moving image*/
        plm_long pidx,            /* Input:  region index of fixed voxel */
        plm_long qidx,            /* Input:  offset index of fixed voxel */
        float li_1[3],            /* Input:  linear interpolation fraction */
        float li_2[3]             /* Input:  linear interpolation fraction */
    )
    {
        /* Compute dc_dv */
        float dc_dv[3];
        bspline_mi_pvi_8_dc_dv_dcos (
            dc_dv, mi_hist, bst,
            fixed, moving, 
            fidx, midx_f, 
            num_vox_f, li_1, li_2
        );

        /* Update cost function gradient */
        ssd->update_smetric_grad_b (bxf, pidx, qidx, dc_dv);
    }
};

#endif
