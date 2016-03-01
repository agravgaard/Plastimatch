/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_score_h_
#define _bspline_score_h_

#include "plmregister_config.h"
#include <vector>
#include "plm_int.h"

class Bspline_xform;

class PLMREGISTER_API Bspline_score
{
public:
    Bspline_score ();
    ~Bspline_score ();
public:
    float score;           /* Total Score (sent to optimizer) */
    float lmetric;         /* Landmark metric */
    float rmetric;         /* Regularization metric */
    std::vector<float> smetric;  /* Similarity metric */

    plm_long num_vox;      /* Number of voxel with correspondence */
    plm_long num_coeff;    /* Size of gradient vector = num coefficents */
    float* smetric_grad;   /* Gradient of score for current smetric */
    float* total_grad;     /* Total cost function gradient wrt coefficients */

    /* Time to compute similarity metric */
    std::vector<double> time_smetric;
    /* Time to compute regularization metric */
    double time_rmetric;
public:
    void set_num_coeff (plm_long num_coeff);
    void reset_smetric_grad ();
    void reset_score ();
    void accumulate_grad (float lambda);
    void update_smetric_grad (
        const Bspline_xform* bxf, 
        const plm_long p[3],
        plm_long qidx,
        const float dc_dv[3]);
    void update_total_grad (
        const Bspline_xform* bxf, 
        const plm_long p[3],
        plm_long qidx,
        const float dc_dv[3]);
    void update_smetric_grad_b (
        const Bspline_xform* bxf, 
        plm_long pidx, 
        plm_long qidx, 
        const float dc_dv[3]);
    void update_total_grad_b (
        const Bspline_xform* bxf, 
        plm_long pidx, 
        plm_long qidx, 
        const float dc_dv[3]);
protected:
    void update_grad (
        float *grad,
        const Bspline_xform* bxf, 
        const plm_long p[3],
        plm_long qidx,
        const float dc_dv[3]);
    void update_grad_b (
        float *grad,
        const Bspline_xform *bxf, 
        plm_long pidx, 
        plm_long qidx, 
        const float dc_dv[3]);
};

#endif
