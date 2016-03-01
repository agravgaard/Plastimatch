/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_parms_h_
#define _bspline_parms_h_

#include "plmregister_config.h"
#include <string>
#include <vector>
#include "bspline_mi_hist.h"
#include "double_align8.h"
#include "registration_metric_type.h"
#include "smart_pointer.h"

enum BsplineOptimization {
    BOPT_LBFGSB,
    BOPT_STEEPEST,
    BOPT_LIBLBFGS,
    BOPT_NLOPT_LBFGS,
    BOPT_NLOPT_LD_MMA,
    BOPT_NLOPT_PTN_1,
};

enum BsplineThreading {
    BTHR_CPU,
    BTHR_CUDA
};

class Bspline_landmarks;
class Regularization_parms;

class PLMREGISTER_API Bspline_parms
{
public:
    SMART_POINTER_SUPPORT (Bspline_parms);
public:
    Bspline_parms ();
    ~Bspline_parms ();
public:
    enum BsplineThreading threading;
    enum BsplineOptimization optimization;
    std::vector<Registration_metric_type> metric_type;
    std::vector<float> metric_lambda;
    char implementation;         /* Implementation ('a', 'b', etc.) */
    int min_its;                 /* Miniumum iterations (line searches) */
    int max_its;                 /* Max iterations (line searches) */
    int max_feval;               /* Max function evaluations */
    int debug;                   /* Create grad & histogram files */
    std::string debug_dir;       /* Directory where to create debug files */
    int debug_stage;             /* Used to tag debug files by stage */
    int gpuid;                   /* Sets GPU to use for multi-gpu machines */
    double_align8 convergence_tol; /* When to stop iterations based on score */

    /* MI parms */
    enum Mi_hist_type mi_hist_type;
    plm_long mi_hist_fixed_bins;
    plm_long mi_hist_moving_bins;

    float mi_fixed_image_minVal;
    float mi_fixed_image_maxVal;
    float mi_moving_image_minVal;
    float mi_moving_image_maxVal;

    /* LBFGSB optimizer parms */
    double_align8 lbfgsb_factr;  /* Function value tolerance for L-BFGS-B */
    double_align8 lbfgsb_pgtol;  /* Projected grad tolerance for L-BFGS-B */

    /* Image Volumes - these are not owned by Bspline_parms */
    Volume* fixed;
    Volume* moving;
    Volume* fixed_grad;
    Volume* moving_grad;
    Volume* fixed_roi;
    Volume* moving_roi;
    Volume* fixed_stiffness;

    /* Regularization */
    Regularization_parms* reg_parms;        /* Regularization Parameters */

    /* Landmarks */
    Bspline_landmarks* blm;      /* Landmarks parameters */
    float rbf_radius;            /* Radius of RBF; if rbf_radius>0, RBF are used */
    float rbf_young_modulus;     /* Penalty for the large 2nd derivative of RBF vector field */
    char* xpm_hist_dump;         /* Pointer to base string of hist dumps */
};

#endif
