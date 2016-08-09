/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _stage_parms_h_
#define _stage_parms_h_

#include "plmregister_config.h"
#include <list>
#include <string>
#include <ctype.h>
#include <stdlib.h>

#include "bspline.h"            /* for enums */
#include "bspline_mi_hist.h"    /* for enums */
#include "plm_image_type.h"
#include "plm_return_code.h"
#include "process_parms.h"
#include "registration_metric_type.h"
#include "threading.h"

enum Stage_transform_type {
    STAGE_TRANSFORM_NONE,
    STAGE_TRANSFORM_ALIGN_CENTER,
    STAGE_TRANSFORM_ALIGN_CENTER_OF_GRAVITY,
    STAGE_TRANSFORM_TRANSLATION,
    STAGE_TRANSFORM_VERSOR,
    STAGE_TRANSFORM_QUATERNION,
    STAGE_TRANSFORM_AFFINE,
    STAGE_TRANSFORM_SIMILARITY,
    STAGE_TRANSFORM_BSPLINE,
    STAGE_TRANSFORM_VECTOR_FIELD
};

enum Optimization_type {
    OPTIMIZATION_NO_REGISTRATION,
    OPTIMIZATION_ALIGN_CENTER,
    OPTIMIZATION_ALIGN_ROI_CENTER,
    OPTIMIZATION_AMOEBA,
    OPTIMIZATION_RSG,
    OPTIMIZATION_VERSOR,
    OPTIMIZATION_LBFGS,
    OPTIMIZATION_LBFGSB,
    OPTIMIZATION_DEMONS,
    OPTIMIZATION_STEEPEST,
    OPTIMIZATION_QUAT,
    OPTIMIZATION_LIBLBFGS,
    OPTIMIZATION_ONEPLUSONE,
    OPTIMIZATION_FRPR,
    OPTIMIZATION_GRID_SEARCH
};

#define IMPLEMENTATION_NONE                 0
#define IMPLEMENTATION_ITK                  1
#define IMPLEMENTATION_PLASTIMATCH          2

#define OPTIMIZATION_SUB_FSF                0
#define OPTIMIZATION_SUB_DIFF_ITK           1
#define OPTIMIZATION_SUB_LOGDOM_ITK         2
#define OPTIMIZATION_SUB_SYM_LOGDOM_ITK     3

#define IMG_OUT_FMT_AUTO                    0
#define IMG_OUT_FMT_DICOM                   1

enum Stage_type {
    STAGE_TYPE_PROCESS,
    STAGE_TYPE_REGISTER
};

enum Resample_type {
    RESAMPLE_AUTO,
    RESAMPLE_VOXEL_RATE,           /* res, res_vox, ss */
    RESAMPLE_MM,                   /* res_mm */
    RESAMPLE_PCT,                  /* res_pct */
    RESAMPLE_DIM                   /* res_dim */
};

enum Regularization_type {
    REGULARIZATION_NONE, 
    REGULARIZATION_BSPLINE_ANALYTIC, 
    REGULARIZATION_BSPLINE_SEMI_ANALYTIC, 
    REGULARIZATION_BSPLINE_NUMERIC
};

enum Demons_gradient_type {
    SYMMETRIC,
    FIXED_IMAGE,
    WARPED_MOVING,
    MAPPED_MOVING
};

enum Gridsearch_strategy_type {
    GRIDSEARCH_STRATEGY_AUTO,
    GRIDSEARCH_STRATEGY_GLOBAL,
    GRIDSEARCH_STRATEGY_LOCAL
};

enum Gridsearch_step_size_type {
    GRIDSEARCH_STEP_SIZE_AUTO,
    GRIDSEARCH_STEP_SIZE_MANUAL
};

class Plm_image;
class Process_parms;
class Shared_parms;
class Stage_parms_private;

class PLMREGISTER_API Stage_parms {
public:
    Stage_parms_private *d_ptr;
public:
    Stage_parms ();
    Stage_parms (const Stage_parms& s);
    ~Stage_parms ();
public:
    /* Stage # */
    int stage_no;
    /* Stage resume? */
    bool resume_stage;
    bool finalize_stage;
    /* Number of substages */
    int num_substages;
    /* Generic optimization parms */
    Stage_transform_type xform_type;
    Optimization_type optim_type;
    int impl_type;
    int optim_subtype;       /* used for demons types (diffeomorphic, etc.) */
    char alg_flavor;
    Threading threading_type;
    int gpuid;               /* Sets GPU to use for multi-gpu machines */
    /* Similarity metric */
    std::vector<Registration_metric_type> metric_type;
    std::vector<float> metric_lambda;
    /* Regularization */
    Regularization_type regularization_type;
    float regularization_lambda;
    /* Image resampling */
    /* The units of fixed_resampling_rate are: voxels for res_vox, 
       mm for res_mm, pct for res_pct, voxels for res_dim */
    Resample_type resample_type;
    float resample_rate_fixed[3];
    float resample_rate_moving[3];
    /* Intensity values for air */
    float background_max;              /* Threshold to find the valid region */
    float default_value;               /* Replacement when out-of-view */
    /* Generic optimization parms */
    int min_its;
    int max_its;
    float convergence_tol;
    /* LBGFG optimizer */
    float grad_tol;
    /* LBGFGB optimizer */
    float pgtol;
    int lbfgsb_mmax;
    /* Versor & RSG optimizer */
    float max_step;
    float min_step;
    float rsg_grad_tol;
    float translation_scale_factor;
    int rotation_scale_factor;
    float scaling_scale_factor;
    /*OnePlusOne evvolutionary optimizer*/
    float opo_epsilon;
    float opo_initial_search_rad;
    /*FRPR optimizer*/
    float frpr_step_tol;
    float frpr_step_length;
    int frpr_max_line_its;
    /* Quaternion optimizer */
    float learn_rate;
    /* Mutual information */
    int mi_hist_fixed_bins;
    int mi_hist_moving_bins;
    int mi_num_spatial_samples;
    float mi_num_spatial_samples_pct;
    enum Mi_hist_type mi_hist_type;
    float mi_fixed_image_minVal;
    float mi_fixed_image_maxVal;
    float mi_moving_image_minVal;
    float mi_moving_image_maxVal;
    /* ITK (& GPUIT) demons */
    float demons_std;
    float demons_std_update_field;
    float demons_step_length;
    bool demons_smooth_update_field, demons_smooth_deformation_field;
    unsigned int num_approx_terms_log_demons;
    bool histoeq;         // histogram matching flag
    bool thresh_mean_intensity;
    unsigned int num_matching_points;
    unsigned int num_hist_levels;
    Demons_gradient_type demons_gradient_type;
    /* GPUIT demons */
    float demons_acceleration;
    float demons_homogenization;
    int demons_filter_width[3];
    /* ITK amoeba */
    float amoeba_parameter_tol;
    /* Bspline parms */
    float grid_spac[3];  // absolute grid spacing in mm in x,y,z directions
    /* Native grid search */
    Gridsearch_strategy_type gridsearch_strategy;
    float gridsearch_min_overlap[3];
    Gridsearch_step_size_type gridsearch_step_size_type;
    float gridsearch_step_size[3];
    int gridsearch_min_steps[3];
    /* Landmarks */
    float landmark_stiffness; //strength of attraction between landmarks
    char landmark_flavor;
    /* Overlap penalty */
    float overlap_penalty_lambda;
    float overlap_penalty_fraction;
    /* Output files */
    int img_out_fmt;
    Plm_image_type img_out_type;
    std::string img_out_fn;
    bool xf_out_itk;
    std::list<std::string> xf_out_fn;
    std::string vf_out_fn;
    std::string debug_dir;

public:
    Stage_type get_stage_type () const;
    Shared_parms *get_shared_parms ();
    const Shared_parms *get_shared_parms () const;
    Process_parms::Pointer get_process_parms ();
    const Process_parms::Pointer get_process_parms () const;
    void set_process_parms (const Process_parms::Pointer&);

    Plm_return_code set_resample (const std::string& s);
    Plm_return_code set_resample_fixed (const std::string& s);
    Plm_return_code set_resample_moving (const std::string& s);
};

#endif
