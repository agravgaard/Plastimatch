/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_parms_h_
#define _bspline_parms_h_

#include "plmregister_config.h"
#include <list>
#include <string>
#include <vector>
#include "double_align8.h"
#include "joint_histogram.h"
#include "metric_state.h"
#include "similarity_metric_type.h"
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
    /* General optimizer parms */
    enum BsplineOptimization optimization;
    int min_its;                 /* Miniumum iterations (line searches) */
    int max_its;                 /* Max iterations (line searches) */
    int max_feval;               /* Max function evaluations */
    double_align8 convergence_tol; /* When to stop iterations based on score */

    /* LBFGSB optimizer parms */
    double_align8 lbfgsb_factr;  /* Function value tolerance for L-BFGS-B */
    double_align8 lbfgsb_pgtol;  /* Projected grad tolerance for L-BFGS-B */
    int lbfgsb_mmax;             /* Number of rows in M matrix */

    /* Debugging */
    int debug;                   /* Create grad & histogram files */
    std::string debug_dir;       /* Directory where to create debug files */
    int debug_stage;             /* Used to tag debug files by stage */
    char* xpm_hist_dump;         /* Pointer to base string of hist dumps */

    /* Threading */
    enum BsplineThreading threading;
    int gpuid;                   /* Sets GPU to use for multi-gpu machines */

    /*! \brief Implementation ('a', 'b', etc.) -- to be moved into 
      Stage_similarity_data */
    char implementation;

    /* MI similarity metric */
    enum Mi_hist_type mi_hist_type;
    plm_long mi_hist_fixed_bins;
    plm_long mi_hist_moving_bins;

    /* Image ROI selection */
    float mi_fixed_image_minVal;
    float mi_fixed_image_maxVal;
    float mi_moving_image_minVal;
    float mi_moving_image_maxVal;

    /* Regularization */
    Regularization_parms* reg_parms;
    Volume* fixed_stiffness;

    /* Landmarks */
    Bspline_landmarks* blm;      /* Landmarks parameters */
    /*! \brief Radius of RBF; if rbf_radius>0, RBF are used */
    float rbf_radius;
    /*! \brief Penalty for the large 2nd derivative of RBF vector field */
    float rbf_young_modulus;

public:
    void log ();
};

#endif
