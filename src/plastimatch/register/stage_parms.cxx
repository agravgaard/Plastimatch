/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shared_parms.h"
#include "stage_parms.h"
#include "string_util.h"

class Stage_parms_private
{
public:
    Stage_type stage_type;
    Process_parms::Pointer process_parms;
    Shared_parms *shared;
public:
    Stage_parms_private () {
        this->stage_type = STAGE_TYPE_REGISTER;
        this->shared = new Shared_parms;
    }
    Stage_parms_private (const Stage_parms_private& s) {
        this->stage_type = STAGE_TYPE_REGISTER;
        this->shared = new Shared_parms (*s.shared);
    }
    ~Stage_parms_private () {
        delete shared;
    }
};

Stage_parms::Stage_parms ()
{
    d_ptr = new Stage_parms_private;

    /* Stage # */
    stage_no = -1;
    /* Stage resume? */
    resume_stage = false;
    finalize_stage = false;
    /* Number of substages */
    num_substages = 1;
    /* Generic optimization parms */
    xform_type = STAGE_TRANSFORM_VERSOR;
    optim_type = OPTIMIZATION_VERSOR;
    impl_type = IMPLEMENTATION_NONE;
    optim_subtype = OPTIMIZATION_SUB_FSF;
    alg_flavor = 0;
    threading_type = THREADING_CPU_OPENMP;
    metric_type.push_back (REGISTRATION_METRIC_MSE);
    metric_lambda.push_back (1.0);
    regularization_type = REGULARIZATION_BSPLINE_ANALYTIC;
    demons_gradient_type = SYMMETRIC;
    regularization_lambda = 0.0f;
    /* Image resample */
    resample_type = RESAMPLE_AUTO;
    resample_rate_fixed[0] = 4;
    resample_rate_fixed[1] = 4;
    resample_rate_fixed[2] = 1;
    resample_rate_moving[0] = 4;
    resample_rate_moving[1] = 4;
    resample_rate_moving[2] = 1;
    /* Intensity values for air */
    background_max = -999.0;
    default_value = 0.0;
    /* Generic optimization parms */
    min_its = 2;
    max_its = 25;
    convergence_tol = 1e-6;
    /* LBGFG optimizer */
    grad_tol = 1.5;
    /* LBGFGB optimizer */
    pgtol = 1.0e-5;
    /* Versor & RSG optimizer */
    max_step = 1.00;
    min_step = 0.001;
    rsg_grad_tol = 0.0001;
    translation_scale_factor = 1000.0;
    rotation_scale_factor = 1;
    scaling_scale_factor = 10;
    /*OnePlusOne evolutionary optimizer*/
    opo_initial_search_rad=1.01;
    opo_epsilon=1e-7;
    /*frpr optimizer*/
    frpr_step_tol=0.000001;
    frpr_step_length=5.0;
    frpr_max_line_its=100;
    /* Quaternion optimizer */
    learn_rate = 0.01 ;
    /* Mattes mutual information */
    mi_hist_fixed_bins = 20;
    mi_hist_moving_bins = 20;
    mi_num_spatial_samples = -1;
    mi_num_spatial_samples_pct = 0.3;
    mi_hist_type = HIST_EQSP;
    /* MI threshold values */
    /*Setting values to zero by default. In this case minVal and 
      maxVal will be calculated from image*/
    mi_fixed_image_minVal=0;
    mi_fixed_image_maxVal=0;
    mi_moving_image_minVal=0;
    mi_moving_image_maxVal=0;
    /* ITK & GPUIT demons */
    demons_std = 1.0;
    demons_std_update_field = 1.0;
    demons_smooth_deformation_field =true;
    demons_smooth_update_field=false;
    demons_step_length = 2.0;
    num_approx_terms_log_demons=2;
    /* GPUIT demons */
    demons_acceleration = 1.0;
    demons_homogenization = 1.0;
    demons_filter_width[0] = 3;
    demons_filter_width[1] = 3;
    demons_filter_width[2] = 3;
    /* ITK amoeba */
    amoeba_parameter_tol = 1.0;
    /* Bspline parms */
    grid_spac[0] = 20.;
    grid_spac[1] = 20.;
    grid_spac[2] = 20.; 
    histoeq = false;         // by default, don't do it
    thresh_mean_intensity=false;
    num_matching_points=500;
    num_hist_levels=1000;
    /* Native grid search */
    gridsearch_strategy = GRIDSEARCH_STRATEGY_AUTO;
    gridsearch_min_overlap[0] = 0.5;
    gridsearch_min_overlap[1] = 0.5;
    gridsearch_min_overlap[2] = 0.5;
    gridsearch_step_size_type = GRIDSEARCH_STEP_SIZE_AUTO;
    gridsearch_step_size[0] = 10;
    gridsearch_step_size[1] = 10;
    gridsearch_step_size[2] = 10;
    /* Landmarks */
    landmark_stiffness = 1.0;
    landmark_flavor = 'a';
    /* Overlap penalty */
    overlap_penalty_lambda = 1.0;
    overlap_penalty_fraction = 0.01;
    /* Output files */
    img_out_fmt = IMG_OUT_FMT_AUTO;
    img_out_type = PLM_IMG_TYPE_UNDEFINED;
    xf_out_itk = false;
}

Stage_parms::Stage_parms (const Stage_parms& s) 
{
    d_ptr = new Stage_parms_private (*s.d_ptr);

    /* Copy most of the parameters ... */

    /* Stage # */
    stage_no = s.stage_no;
    /* Stage resume? */
    /* ** Do not copy */
    /* Number of substages */
    num_substages = s.num_substages;
    /* Generic optimization parms */
    xform_type = s.xform_type;
    optim_type = s.optim_type;
    impl_type = s.impl_type;
    optim_subtype = s.optim_subtype;
    alg_flavor = s.alg_flavor;
    threading_type = s.threading_type;
    metric_type = s.metric_type;
    metric_lambda = s.metric_lambda;
    regularization_type = s.regularization_type;
    regularization_lambda = s.regularization_lambda;
    /* Image resample */
    resample_type = s.resample_type;
    resample_rate_fixed[0] = s.resample_rate_fixed[0];
    resample_rate_fixed[1] = s.resample_rate_fixed[1];
    resample_rate_fixed[2] = s.resample_rate_fixed[2];
    resample_rate_moving[0] = s.resample_rate_moving[0];
    resample_rate_moving[1] = s.resample_rate_moving[1];
    resample_rate_moving[2] = s.resample_rate_moving[2];
    /* Intensity values for air */
    background_max = s.background_max;
    default_value = s.default_value;
    /* Generic optimization parms */
    min_its = s.min_its;
    max_its = s.max_its;
    convergence_tol = s.convergence_tol;
    /* LBGFG optimizer */
    grad_tol = s.grad_tol;
    /* LBGFGB optimizer */
    pgtol = s.pgtol;
    /* Versor & RSG optimizer */
    max_step = s.max_step;
    min_step = s.min_step;
    rsg_grad_tol = s.rsg_grad_tol;
    translation_scale_factor = s.translation_scale_factor;
    rotation_scale_factor = s.rotation_scale_factor;
    scaling_scale_factor = s.scaling_scale_factor;

    /*OnePlusOne optmizer*/
    opo_epsilon=s.opo_epsilon;
    opo_initial_search_rad=s.opo_initial_search_rad;

   /*FRPR optmizer*/
    frpr_step_length=s.frpr_step_length;
    frpr_step_tol=s.frpr_step_tol;
    frpr_max_line_its=s.frpr_max_line_its;

    /* Quaternion optimizer */
    learn_rate = s.learn_rate;
    /* Mattes mutual information */
    mi_hist_fixed_bins = s.mi_hist_fixed_bins;
    mi_hist_moving_bins = s.mi_hist_moving_bins;
    mi_num_spatial_samples = s.mi_num_spatial_samples;
    mi_num_spatial_samples_pct = s.mi_num_spatial_samples_pct;
    mi_hist_type = s.mi_hist_type;
    /* MI threshold values */
    mi_fixed_image_minVal = s.mi_fixed_image_minVal;
    mi_fixed_image_maxVal = s.mi_fixed_image_maxVal;
    mi_moving_image_minVal = s.mi_moving_image_minVal;
    mi_moving_image_maxVal = s.mi_moving_image_maxVal;
    /* ITK & GPUIT demons */
    demons_std = s.demons_std;
    demons_std_update_field=s.demons_std_update_field;
    demons_smooth_deformation_field=s.demons_smooth_deformation_field;
    demons_smooth_update_field=s.demons_smooth_update_field;
    demons_step_length=s.demons_step_length;
    demons_gradient_type=s.demons_gradient_type;
    num_approx_terms_log_demons=s.num_approx_terms_log_demons;
    /* GPUIT demons */
    demons_acceleration = s.demons_acceleration;
    demons_homogenization = s.demons_homogenization;
    demons_filter_width[0] = s.demons_filter_width[0];
    demons_filter_width[1] = s.demons_filter_width[1];
    demons_filter_width[2] = s.demons_filter_width[2];
    /* ITK amoeba */
    amoeba_parameter_tol = s.amoeba_parameter_tol;
    /* Bspline parms */
    grid_spac[0] = s.grid_spac[0];
    grid_spac[1] = s.grid_spac[1];
    grid_spac[2] = s.grid_spac[2];
    histoeq = s.histoeq;
    thresh_mean_intensity= s.thresh_mean_intensity;
    num_matching_points= s.num_matching_points;
    num_hist_levels= s.num_hist_levels;
    /* Native grid search */
    gridsearch_strategy = s.gridsearch_strategy;
    gridsearch_min_overlap[0] = s.gridsearch_min_overlap[0];
    gridsearch_min_overlap[1] = s.gridsearch_min_overlap[1];
    gridsearch_min_overlap[2] = s.gridsearch_min_overlap[2];
    gridsearch_step_size_type = s.gridsearch_step_size_type;
    gridsearch_step_size[0] = s.gridsearch_step_size[0];
    gridsearch_step_size[1] = s.gridsearch_step_size[1];
    gridsearch_step_size[2] = s.gridsearch_step_size[2];
    /* Landmarks */
    landmark_stiffness = s.landmark_stiffness;
    landmark_flavor = s.landmark_flavor;
    /* Overlap penalty */
    overlap_penalty_lambda = s.overlap_penalty_lambda;
    overlap_penalty_fraction = s.overlap_penalty_fraction;
    /* Output files */
    img_out_type = s.img_out_type;
    xf_out_itk = s.xf_out_itk;

    /* ...but not the output filenames */
    img_out_fmt = IMG_OUT_FMT_AUTO;
    xf_out_fn.clear ();

    /* ...and don't to resume/finalize unless specifically requested */
    resume_stage = false;
    finalize_stage = false;
}

Stage_parms::~Stage_parms ()
{
    delete d_ptr;
}

Stage_type 
Stage_parms::get_stage_type () const
{
    return d_ptr->stage_type;
}

Shared_parms*
Stage_parms::get_shared_parms () 
{
    return d_ptr->shared;
}

const Shared_parms*
Stage_parms::get_shared_parms () const
{
    return d_ptr->shared;
}

Process_parms::Pointer 
Stage_parms::get_process_parms ()
{
    return d_ptr->process_parms;
}

const Process_parms::Pointer 
Stage_parms::get_process_parms () const
{
    return d_ptr->process_parms;
}

void 
Stage_parms::set_process_parms (const Process_parms::Pointer& pp)
{
    d_ptr->stage_type = STAGE_TYPE_PROCESS;
    d_ptr->process_parms = pp;
}

Plm_return_code
Stage_parms::set_resample (const std::string& s)
{
    Plm_return_code rc = parse_float13 (this->resample_rate_fixed, s.c_str());
    if (rc != PLM_SUCCESS) {
        return rc;
    }
    this->resample_rate_moving[0] = this->resample_rate_fixed[0];
    this->resample_rate_moving[1] = this->resample_rate_fixed[1];
    this->resample_rate_moving[2] = this->resample_rate_fixed[2];
    return PLM_SUCCESS;
}

Plm_return_code
Stage_parms::set_resample_fixed (const std::string& s)
{
    Plm_return_code rc = parse_float13 (this->resample_rate_fixed, s.c_str());
    if (rc != PLM_SUCCESS) {
        return rc;
    }
    if (this->resample_type == RESAMPLE_AUTO) {
        this->resample_rate_moving[0] = this->resample_rate_fixed[0];
        this->resample_rate_moving[1] = this->resample_rate_fixed[1];
        this->resample_rate_moving[2] = this->resample_rate_fixed[2];
    }
    return PLM_SUCCESS;
}

Plm_return_code
Stage_parms::set_resample_moving (const std::string& s)
{
    Plm_return_code rc = parse_float13 (this->resample_rate_moving, s.c_str());
    if (rc != PLM_SUCCESS) {
        return rc;
    }
    if (this->resample_type == RESAMPLE_AUTO) {
        this->resample_rate_fixed[0] = this->resample_rate_moving[0];
        this->resample_rate_fixed[1] = this->resample_rate_moving[1];
        this->resample_rate_fixed[2] = this->resample_rate_moving[2];
    }
    return PLM_SUCCESS;
}
