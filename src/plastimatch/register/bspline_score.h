/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_score_h_
#define _bspline_score_h_

#include "plmregister_config.h"
#include <vector>
#include "plm_int.h"

class Bspline_xform;

class PLMREGISTER_API Metric_score
{
public:
    Metric_score () {
        score = 0.f;
        time = 0.f;
        num_vox = 0;
    }
    Metric_score (float score, float time, plm_long num_vox) 
        : score(score), time(time), num_vox(num_vox) {
        score = 0.f;
        time = 0.f;
        num_vox = 0;
    }
public:
    float score;
    double time;
    plm_long num_vox;
};

class PLMREGISTER_API Bspline_score
{
public:
    Bspline_score ();
    ~Bspline_score ();
public:
    float total_score;     /* Total Score (sent to optimizer) */
    float* total_grad;     /* Total cost function gradient */

    float lmetric;         /* Landmark metric */
    float rmetric;         /* Regularization metric */

    /*! \brief The metric_record keeps track of score statistics 
      for reporting purposes */
    std::vector<Metric_score> metric_record;

    plm_long num_coeff;    /* Size of gradient vector = num coefficents */

    float curr_smetric;         /* Current smetric value */
    float* curr_smetric_grad;   /* Gradient of score for current smetric */
    plm_long curr_num_vox;      /* Number of voxel with correspondence */

    /* Time to compute regularization metric */
    double time_rmetric;
public:
    void set_num_coeff (plm_long num_coeff);
    void reset_smetric_grad ();
    void reset_score ();
    void accumulate (float lambda);
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
