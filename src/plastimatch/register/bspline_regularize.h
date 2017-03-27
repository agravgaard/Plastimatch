/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_regularize_h_
#define _bspline_regularize_h_

#include "plmregister_config.h"
#include "volume.h"

class Bspline_score;
class Bspline_xform;

class Regularization_parms
{
public:
    char implementation;    /* Implementation: a, b, c, etc */
    float lambda;           /* Smoothness weighting factor  */
public:
    Regularization_parms () {
        this->implementation = '\0';
        this->lambda = 0.0f;
    }
};

class PLMREGISTER_API Bspline_regularize {
public:
    SMART_POINTER_SUPPORT (Bspline_regularize);
public:
    Bspline_regularize ();
    ~Bspline_regularize ();
public:
    /* all methods */
    Regularization_parms *reg_parms;
    Bspline_xform *bxf;

    Volume* fixed_stiffness;

    /* numeric methods */
    float* q_dxdyz_lut;          /* LUT for influence of dN1/dx*dN2/dy*N3 */
    float* q_xdydz_lut;          /* LUT for influence of N1*dN2/dy*dN3/dz */
    float* q_dxydz_lut;          /* LUT for influence of dN1/dx*N2*dN3/dz */
    float* q_d2xyz_lut;          /* LUT for influence of (d2N1/dx2)*N2*N3 */
    float* q_xd2yz_lut;          /* LUT for influence of N1*(d2N2/dy2)*N3 */
    float* q_xyd2z_lut;          /* LUT for influence of N1*N2*(d2N3/dz2) */

    /* analytic methods */
    double* QX_mats;    /* Three 4x4 matrices */
    double* QY_mats;    /* Three 4x4 matrices */
    double* QZ_mats;    /* Three 4x4 matrices */
    double** QX;
    double** QY;
    double** QZ;
    double* V_mats;     /* The 6 64x64 V matricies */
    double** V;
    double* cond;
public:
    void initialize (
        Regularization_parms* reg_parms,
        Bspline_xform* bxf
    );
    void compute_score (
        Bspline_score* bsp_score,    /* Gets updated */
        const Regularization_parms* reg_parms,
        const Bspline_xform* bxf
    );

protected:
    void numeric_init (
        const Bspline_xform* bxf);
    void compute_score_numeric (
        Bspline_score *bscore, 
        const Regularization_parms *parms, 
        const Bspline_regularize *rst,
        const Bspline_xform* bxf);

    void analytic_init (
        const Bspline_xform* bxf);
    void compute_score_analytic (
        Bspline_score *bspline_score, 
        const Regularization_parms* reg_parms,
        const Bspline_regularize* rst,
        const Bspline_xform* bxf);
    void compute_score_analytic_omp (
        Bspline_score *bspline_score, 
        const Regularization_parms* reg_parms,
        const Bspline_regularize* rst,
        const Bspline_xform* bxf);

    void semi_analytic_init (
        const Bspline_xform* bxf);
    void create_qlut_grad (
        const Bspline_xform* bxf,
        const float img_spacing[3],
        const plm_long vox_per_rgn[3]);
    void hessian_component (
        float out[3], 
        const Bspline_xform* bxf, 
        plm_long p[3], 
        plm_long qidx, 
        int derive1, 
        int derive2);
    void hessian_update_grad (
        Bspline_score *bscore, 
        const Bspline_xform* bxf, 
        plm_long p[3], 
        plm_long qidx, 
        float dc_dv[3], 
        int derive1, 
        int derive2);
    void compute_score_semi_analytic (
        Bspline_score *bscore, 
        const Regularization_parms *parms, 
        const Bspline_regularize *rst,
        const Bspline_xform* bxf);
};

#endif
