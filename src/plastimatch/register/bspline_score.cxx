/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "bspline_score.h"
#include "bspline_xform.h"

Bspline_score::Bspline_score ()
{
    this->score = 0;
    this->lmetric = 0;
    this->rmetric = 0;

    this->num_vox = 0;
    this->num_coeff = 0;
    this->smetric_grad = 0;
    this->total_grad = 0;

    this->time_rmetric = 0;
}

Bspline_score::~Bspline_score ()
{
    delete[] smetric_grad;
    delete[] total_grad;
}

void
Bspline_score::set_num_coeff (plm_long num_coeff)
{
    this->num_coeff = num_coeff;
    delete[] this->smetric_grad;
    delete[] this->total_grad;
    this->smetric_grad = new float[num_coeff];
    this->total_grad = new float[num_coeff];
}

void
Bspline_score::reset_smetric_grad ()
{
    memset (this->smetric_grad, 0, this->num_coeff * sizeof(float));
}

void
Bspline_score::reset_score ()
{
    this->score = 0;
    this->lmetric = 0;
    this->rmetric = 0;
    this->smetric.clear();
    this->num_vox = 0;
    memset (this->smetric_grad, 0, this->num_coeff * sizeof(float));
    memset (this->total_grad, 0, this->num_coeff * sizeof(float));
    this->time_smetric.clear();
    this->time_rmetric = 0;
}

void
Bspline_score::accumulate_grad (float lambda)
{
    for (plm_long i = 0; i < this->num_coeff; i++) {
        this->total_grad[i] += lambda * this->smetric_grad[i];
    }
    this->reset_smetric_grad ();
}

void
Bspline_score::update_smetric_grad (
    const Bspline_xform* bxf, 
    const plm_long p[3],
    plm_long qidx,
    const float dc_dv[3])
{
    this->update_grad (this->smetric_grad, bxf, p, qidx, dc_dv);
}

void
Bspline_score::update_total_grad (
    const Bspline_xform* bxf, 
    const plm_long p[3],
    plm_long qidx,
    const float dc_dv[3])
{
    this->update_grad (this->total_grad, bxf, p, qidx, dc_dv);
}

void
Bspline_score::update_smetric_grad_b (
    const Bspline_xform* bxf, 
    plm_long pidx, 
    plm_long qidx, 
    const float dc_dv[3])
{
    this->update_grad_b (this->smetric_grad, bxf, pidx, qidx, dc_dv);
}

void
Bspline_score::update_total_grad_b (
    const Bspline_xform* bxf, 
    plm_long pidx, 
    plm_long qidx, 
    const float dc_dv[3])
{
    this->update_grad_b (this->total_grad, bxf, pidx, qidx, dc_dv);
}

void
Bspline_score::update_grad (
    float *grad,
    const Bspline_xform* bxf, 
    const plm_long p[3],
    plm_long qidx,
    const float dc_dv[3])
{
    plm_long i, j, k, m;
    plm_long cidx;
    float* q_lut = &bxf->q_lut[qidx*64];

    m = 0;
    for (k = 0; k < 4; k++) {
        for (j = 0; j < 4; j++) {
            for (i = 0; i < 4; i++) {
                cidx = (p[2] + k) * bxf->cdims[1] * bxf->cdims[0]
                    + (p[1] + j) * bxf->cdims[0]
                    + (p[0] + i);
                cidx = cidx * 3;
                grad[cidx+0] += dc_dv[0] * q_lut[m];
                grad[cidx+1] += dc_dv[1] * q_lut[m];
                grad[cidx+2] += dc_dv[2] * q_lut[m];
                m ++;
            }
        }
    }
}

void
Bspline_score::update_grad_b (
    float *grad,
    const Bspline_xform* bxf, 
    plm_long pidx, 
    plm_long qidx, 
    const float dc_dv[3])
{
    plm_long i, j, k, m;
    plm_long cidx;
    float* q_lut = &bxf->q_lut[qidx*64];
    plm_long* c_lut = &bxf->c_lut[pidx*64];

    m = 0;
    for (k = 0; k < 4; k++) {
        for (j = 0; j < 4; j++) {
            for (i = 0; i < 4; i++) {
                cidx = 3 * c_lut[m];
                grad[cidx+0] += dc_dv[0] * q_lut[m];
                grad[cidx+1] += dc_dv[1] * q_lut[m];
                grad[cidx+2] += dc_dv[2] * q_lut[m];
                m ++;
            }
        }
    }
}
