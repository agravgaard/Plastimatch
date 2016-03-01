/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <fstream>
#include <list>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined (_WIN32)
// win32 directory stuff
#else
#include <sys/types.h>
#include <dirent.h>
#endif

#include "logfile.h"
#include "parameter_parser.h"
#include "plm_return_code.h"
#include "registration_parms.h"
#include "shared_parms.h"
#include "stage_parms.h"
#include "string_util.h"

class Registration_parms_private
{
public:
    std::string moving_fn;
    std::string fixed_fn;
    std::list<Stage_parms*> stages;
    Shared_parms *shared;

    /* GCS FIX: These were disabled, and are needing some re-engineering */
    std::list< std::string > moving_jobs;
    std::list< std::string > fixed_jobs;

public:
    Registration_parms_private () {
        shared = new Shared_parms;
    }
    ~Registration_parms_private () {
        delete_all_stages ();
        delete shared;
    }
    void delete_all_stages () {
        std::list<Stage_parms*>::iterator it;
        for (it = stages.begin(); it != stages.end(); it++) {
            delete *it;
        }
        stages.clear ();
    }
};

class Registration_parms_parser : public Parameter_parser
{
public:
    Registration_parms *rp;
public:
    Registration_parms_parser (Registration_parms *rp)
    {
        this->rp = rp;
    }
public:
    virtual Plm_return_code begin_section (
        const std::string& section)
    {
        if (section == "GLOBAL") {
            return PLM_SUCCESS;
        }
        if (section == "STAGE") {
            rp->append_stage ();
            return PLM_SUCCESS;
        }
        if (section == "COMMENT") {
            return PLM_SUCCESS;
        }
        if (section == "PROCESS") {
            rp->append_process_stage ();
            return PLM_SUCCESS;
        }

        /* else, unknown section */
        return PLM_ERROR;
    }
    virtual Plm_return_code end_section (
        const std::string& section)
    {
        /* Do nothing */
        return PLM_SUCCESS;
    }
    virtual Plm_return_code set_key_value (
        const std::string& section,
        const std::string& key, 
        const std::string& val)
    {
        return this->rp->set_key_value (section, key, val);
    }
};

Registration_parms::Registration_parms()
{
    d_ptr = new Registration_parms_private;

    img_out_fmt = IMG_OUT_FMT_AUTO;
    img_out_type = PLM_IMG_TYPE_UNDEFINED;
    xf_out_itk = false;
    init_type = STAGE_TRANSFORM_NONE;
    default_value = 0.0;
    num_stages = 0;
    job_idx = 0;
    num_jobs = 1;
}

Registration_parms::~Registration_parms()
{
    delete d_ptr;
}

#if defined (GCS_FIXME)  // Delete this, replace with compose_filename
// JAS 2012.02.13 -- TODO: Move somewhere more appropriate
static void
check_trailing_slash (char* s)
{
    int i=0;
    while (s[i++] != '\0');

    if (s[i-2] != '/') {
        strcat (s, "/");
    }
}
#endif


#if defined (GCS_FIXME)  // Delete this, replace with Dir_list
int
populate_jobs ()
{
    /* This function should read through the directory, 
       and identify the input files for each registration role 
       (fixed, moving, etc.) */
}
#endif

Plm_return_code
Registration_parms::set_key_value (
    const std::string& section,
    const std::string& key, 
    const std::string& val)
{
    int rc;
    Stage_parms *stage = 0;
    Shared_parms *shared = 0;
    Process_parms::Pointer process;
    bool section_global = false;
    bool section_stage = false;
    bool section_process = false;

    if (section == "COMMENT") {
        return PLM_SUCCESS;
    }

    if (section == "GLOBAL") {
        shared = d_ptr->shared;
        section_global = true;
    }
    else if (section == "STAGE") {
        stage = d_ptr->stages.back();
        shared = stage->get_shared_parms();
        section_stage = true;
    }
    else if (section == "PROCESS") {
        stage = d_ptr->stages.back();
        process = stage->get_process_parms();
        section_process = true;
    }

    /* The following keywords are only allowed globally */
    if (key == "fixed") {
        if (!section_global) goto key_only_allowed_in_section_global;
        d_ptr->fixed_fn = val;
    }
    else if (key == "moving") {
        if (!section_global) goto key_only_allowed_in_section_global;
        d_ptr->moving_fn = val;
    }
#if defined (GCS_FIXME) // stubbed out until above is fixed
    else if (key == "fixed_dir") {
        if (!section_global) goto key_only_allowed_in_section_global;
        this->fixed_dir = val;
        check_trailing_slash (this->fixed_dir);
        this->num_jobs = populate_jobs (this->fixed_jobs, this->fixed_dir);
    }
    else if (key == "moving_dir") {
        if (!section_global) goto key_only_allowed_in_section_global;
        this->moving_dir = val;
        check_trailing_slash (this->moving_dir);
        this->num_jobs = populate_jobs (this->moving_jobs, this->moving_dir);
    }
    else if (key == "img_out_dir") {
        if (!section_global) goto key_only_allowed_in_section_global;
        this->img_out_dir = val;
        check_trailing_slash (this->img_out_dir);
    }
    else if (key == "vf_out_dir") {
        if (!section_global) goto key_only_allowed_in_section_global;
        this->vf_out_dir = val;
        check_trailing_slash (this->vf_out_dir);
    }
#endif
    else if (key == "xf_in"
        || key == "xform_in"
        || key == "vf_in")
    {
        if (!section_global) goto key_only_allowed_in_section_global;
        this->xf_in_fn = val;
    }
    else if (key == "log" || key == "logfile") {
        if (!section_global) goto key_only_allowed_in_section_global;
        this->log_fn = val;
    }

    /* The following keywords are allowed either globally or in stages */
    else if (key == "background_val"
        || key == "default_value")
    {
        float f;
        if (sscanf (val.c_str(), "%g", &f) != 1) {
            goto error_exit;
        }
        if (section_global) {
            this->default_value = f;
        } else if (section_stage) {
            stage->default_value = f;
        } else {
            goto key_not_allowed_in_section_process;
        }
    }
    else if (key == "fixed_mask" || key == "fixed_roi") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->fixed_roi_fn = val;
    }
    else if (key == "moving_mask" || key == "moving_roi") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->moving_roi_fn = val;
    }
    else if (key == "fixed_roi_enable") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->fixed_roi_enable = string_value_true (val);
    }
    else if (key == "moving_roi_enable")
    {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->moving_roi_enable = string_value_true (val);
    }
    else if (key == "fixed_stiffness") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->fixed_stiffness_fn = val;
    }
    else if (key == "fixed_stiffness_enable") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->fixed_stiffness_enable = string_value_true (val);
    }
    else if (key == "legacy_subsampling") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->legacy_subsampling = string_value_true (val);
    }
    else if (key == "img_out" || key == "image_out") {
        if (section_global) {
            this->img_out_fn = val;
        } else if (section_stage) {
            stage->img_out_fn = val;
        } else {
            goto key_not_allowed_in_section_process;
        }
    }
    else if (key == "img_out_fmt") {
        int fmt = IMG_OUT_FMT_AUTO;
        if (val == "dicom") {
            fmt = IMG_OUT_FMT_DICOM;
        } else {
            goto error_exit;
        }
        if (section_global) {
            this->img_out_fmt = fmt;
        } else if (section_stage) {
            stage->img_out_fmt = fmt;
        } else {
            goto key_not_allowed_in_section_process;
        }
    }
    else if (key == "img_out_type") {
        Plm_image_type type = plm_image_type_parse (val.c_str());
        if (type == PLM_IMG_TYPE_UNDEFINED) {
            goto error_exit;
        }
        if (section_global) {
            this->img_out_type = type;
        } else if (section_stage) {
            stage->img_out_type = type;
        } else {
            goto key_not_allowed_in_section_process;
        }
    }
    else if (key == "vf_out") {
        if (section_global) {
            this->vf_out_fn = val;
        } else if (section_stage) {
            stage->vf_out_fn = val;
        } else {
            goto key_not_allowed_in_section_process;
        }
    }
    else if (key == "xf_out_itk") {
        bool value;
        if (string_value_true (val)) {
            value = true;
        } else {
            value = false;
        }
        if (section_global) {
            this->xf_out_itk = value;
        } else if (section_stage) {
            stage->xf_out_itk = value;
        } else {
            goto key_not_allowed_in_section_process;
        }
    }
    else if (key == "xf_out" || key == "xform_out") {
        /* xf_out is special.  You can have more than one of these.  
           This capability is used by the slicer plugin. */
        if (section_global) {
            this->xf_out_fn.push_back (val.c_str());
        } else if (section_stage) {
            stage->xf_out_fn.push_back (val.c_str());
        } else {
            goto key_not_allowed_in_section_process;
        }
    }
    else if (key == "fixed_landmarks") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->fixed_landmarks_fn = val;
    }
    else if (key == "moving_landmarks") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->moving_landmarks_fn = val;
    }
    else if (key == "fixed_landmark_list") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->fixed_landmarks_list = val;
    }
    else if (key == "moving_landmark_list") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->moving_landmarks_list = val;
    }
    else if (key == "warped_landmarks") {
        if (section_process) goto key_not_allowed_in_section_process;
        shared->warped_landmarks_fn = val;
    }

    /* The following keywords are only allowed in stages */
    else if (key == "num_substages") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->num_substages) != 1) {
            goto error_exit;
        }
    }
    else if (key == "flavor" || key == "alg_flavor") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val.length() >= 1) {
            stage->alg_flavor = val[0];
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "resume") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (string_value_true (val)) {
            stage->resume_stage = true;
        }
    }
    else if (key == "xform") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "translation") {
            stage->xform_type = STAGE_TRANSFORM_TRANSLATION;
        }
        else if (val == "rigid" || val == "versor") {
            stage->xform_type = STAGE_TRANSFORM_VERSOR;
        }
        else if (val == "quaternion") {
            stage->xform_type = STAGE_TRANSFORM_QUATERNION;
        }
        else if (val == "affine") {
            stage->xform_type = STAGE_TRANSFORM_AFFINE;
        }
        else if (val == "similarity") {
            stage->xform_type = STAGE_TRANSFORM_SIMILARITY;
        }
        else if (val == "bspline") {
            stage->xform_type = STAGE_TRANSFORM_BSPLINE;
        }
        else if (val == "vf") {
            stage->xform_type = STAGE_TRANSFORM_VECTOR_FIELD;
        }
        else if (val == "align_center") {
            stage->xform_type = STAGE_TRANSFORM_ALIGN_CENTER;
        }
        else if (val == "align_center_of_gravity") {
            stage->xform_type = STAGE_TRANSFORM_ALIGN_CENTER_OF_GRAVITY;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "optim") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "none") {
            stage->optim_type = OPTIMIZATION_NO_REGISTRATION;
        }
        else if (val == "amoeba") {
            stage->optim_type = OPTIMIZATION_AMOEBA;
        }
        else if (val == "demons") {
            stage->optim_type = OPTIMIZATION_DEMONS;
        }
        else if (val == "frpr") {
            stage->optim_type = OPTIMIZATION_FRPR;
        }
        else if (val == "grid" || val == "grid_search"
            || val == "gridsearch") {
            stage->optim_type = OPTIMIZATION_GRID_SEARCH;
        }
        else if (val == "lbfgs") {
            stage->optim_type = OPTIMIZATION_LBFGS;
        }
        else if (val == "lbfgsb") {
            stage->optim_type = OPTIMIZATION_LBFGSB;
        }
        else if (val == "liblbfgs") {
            stage->optim_type = OPTIMIZATION_LIBLBFGS;
        }
        else if (val == "nocedal") {
            stage->optim_type = OPTIMIZATION_LBFGSB;
        }
        else if (val == "oneplusone") {
            stage->optim_type = OPTIMIZATION_ONEPLUSONE;
        }
        else if (val == "rsg") {
            stage->optim_type = OPTIMIZATION_RSG;
        }
        else if (val == "steepest") {
            stage->optim_type = OPTIMIZATION_STEEPEST;
        }
        else if (val == "versor") {
            stage->optim_type = OPTIMIZATION_VERSOR;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "impl") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "none") {
            stage->impl_type = IMPLEMENTATION_NONE;
        }
        else if (val == "itk") {
            stage->impl_type = IMPLEMENTATION_ITK;
        }
        else if (val == "plastimatch") {
            stage->impl_type = IMPLEMENTATION_PLASTIMATCH;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "optim_subtype") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "fsf") {
            stage->optim_subtype = OPTIMIZATION_SUB_FSF;
        }
        else if (val == "diffeomorphic") {
            stage->optim_subtype = OPTIMIZATION_SUB_DIFF_ITK;
        }
        else if (val == "log_domain") {
            stage->optim_subtype = OPTIMIZATION_SUB_LOGDOM_ITK;
        }
        else if (val == "sym_log_domain") {
            stage->optim_subtype = OPTIMIZATION_SUB_SYM_LOGDOM_ITK;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "threading") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "single") {
            stage->threading_type = THREADING_CPU_SINGLE;
        }
        else if (val == "openmp") {
#if (OPENMP_FOUND)
            stage->threading_type = THREADING_CPU_OPENMP;
#else
            stage->threading_type = THREADING_CPU_SINGLE;
#endif
        }
        else if (val == "cuda") {
#if (CUDA_FOUND)
            stage->threading_type = THREADING_CUDA;
#elif (OPENMP_FOUND)
            stage->threading_type = THREADING_CPU_OPENMP;
#else
            stage->threading_type = THREADING_CPU_SINGLE;
#endif
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "metric" || key == "smetric") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        std::vector<std::string> metric_vec = string_split (val, ',');
        if (metric_vec.size() == 0) {
            goto error_exit;
        }
        stage->metric_type.clear();
        for (int i = 0; i < metric_vec.size(); i++) {
            if (metric_vec[i] == "gm") {
                stage->metric_type.push_back (REGISTRATION_METRIC_GM);
            }
            else if (metric_vec[i] == "mattes") {
                stage->metric_type.push_back (REGISTRATION_METRIC_MI_MATTES);
            }
            else if (metric_vec[i] == "mse" || metric_vec[i] == "MSE") {
                stage->metric_type.push_back (REGISTRATION_METRIC_MSE);
            }
            else if (metric_vec[i] == "mi" || metric_vec[i] == "MI") {
#if PLM_CONFIG_LEGACY_MI_METRIC
                stage->metric_type.push_back (REGISTRATION_METRIC_MI_VW);
#else
                stage->metric_type.push_back (REGISTRATION_METRIC_MI_MATTES);
#endif
            }
            else if (metric_vec[i] == "mi_vw"
                    || metric_vec[i] == "viola-wells")
            {
                stage->metric_type.push_back (REGISTRATION_METRIC_MI_VW);
            }
            else if (metric_vec[i] == "nmi" || metric_vec[i] == "NMI") {
                stage->metric_type.push_back (REGISTRATION_METRIC_NMI);
            }
            else {
                goto error_exit;
            }
        }
    }
    else if (key == "metric_lambda" || key == "smetric_lambda") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        stage->metric_lambda = parse_float_string (val);
        if (stage->metric_lambda.size() == 0) {
            goto error_exit;
        }
    }
    else if (key == "histogram_type") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "eqsp" || val == "EQSP") {
            stage->mi_hist_type = HIST_EQSP;
        }
        else if (val == "vopt" || val == "VOPT") {
            stage->mi_hist_type = HIST_VOPT;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "regularization")
    {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "none") {
            stage->regularization_type = REGULARIZATION_NONE;
        }
        else if (val == "analytic") {
            stage->regularization_type = REGULARIZATION_BSPLINE_ANALYTIC;
        }
        else if (val == "semi_analytic") {
            stage->regularization_type = REGULARIZATION_BSPLINE_SEMI_ANALYTIC;
        }
        else if (val == "numeric") {
            stage->regularization_type = REGULARIZATION_BSPLINE_NUMERIC;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "regularization_lambda"
        || key == "young_modulus") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%f", &stage->regularization_lambda) != 1) {
            goto error_exit;
        }
    }
    else if (key == "background_max") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->background_max) != 1) {
            goto error_exit;
        }
    }
    else if (key == "min_its") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->min_its) != 1) {
            goto error_exit;
        }
    }
    else if (key == "iterations" 
        || key == "max_iterations"
        || key == "max_its"
        || key == "its")
    {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->max_its) != 1) {
            goto error_exit;
        }
    }
    else if (key == "learn_rate") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->learn_rate) != 1) {
            goto error_exit;
        }
    }
    else if (key == "grad_tol") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->grad_tol) != 1) {
            goto error_exit;
        }
    }
    else if (key == "pgtol") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%f", &stage->pgtol) != 1) {
            goto error_exit;
        }
    }
    else if (key == "max_step") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->max_step) != 1) {
            goto error_exit;
        }
    }
    else if (key == "min_step") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->min_step) != 1) {
            goto error_exit;
        }
    }
    else if (key == "rsg_grad_tol") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->rsg_grad_tol) != 1) {
            goto error_exit;
        }
    }
    else if (key == "translation_scale_factor") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->translation_scale_factor) != 1) {
            goto error_exit;
        }
    }
    else if (key == "rotation_scale_factor") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->rotation_scale_factor) != 1) {
            goto error_exit;
        }
    }
    else if (key == "scaling_scale_factor") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%f", &stage->scaling_scale_factor) != 1) {
            goto error_exit;
        }
    }
    else if (key == "convergence_tol") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->convergence_tol) != 1) {
            goto error_exit;
        }
    }
    else if (key == "opo_epsilon") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->opo_epsilon) != 1) {
            goto error_exit;
        }
    }
    else if (key == "opo_initial_search_rad") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->opo_initial_search_rad) != 1) {
            goto error_exit;
        }
    }
    else if (key == "frpr_step_tol") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->frpr_step_tol) != 1) {
            goto error_exit;
        }
    }
    else if (key == "frpr_step_length") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->frpr_step_length) != 1) {
            goto error_exit;
        }
    }
    else if (key == "frpr_max_line_its") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->frpr_max_line_its) != 1) {
            goto error_exit;
        }
    }
    else if (key == "mattes_histogram_bins" 
        || key == "mi_histogram_bins") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        rc = sscanf (val.c_str(), "%d %d", &stage->mi_hist_fixed_bins,
            &stage->mi_hist_moving_bins);
        if (rc == 1) {
            stage->mi_hist_moving_bins = stage->mi_hist_fixed_bins;
        } else if (rc != 2) {
            goto error_exit;
        }
    }
    else if (key == "mattes_fixed_minVal"
        ||key == "mi_fixed_minVal") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->mi_fixed_image_minVal) != 1) {
            goto error_exit;
        }
    }
    else if (key == "mattes_fixed_maxVal"
        ||key == "mi_fixed_maxVal") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->mi_fixed_image_maxVal) != 1) {
            goto error_exit;
        }
    }
    else if (key == "mattes_moving_minVal"
        ||key == "mi_moving_minVal") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->mi_moving_image_minVal) != 1) {
            goto error_exit;
        }
    }
    else if (key == "mattes_moving_maxVal"
        ||key == "mi_moving_maxVal") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->mi_moving_image_maxVal) != 1) {
            goto error_exit;
        }
    }
    else if (key == "num_samples"
        || key == "mattes_num_spatial_samples"
        || key == "mi_num_spatial_samples") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->mi_num_spatial_samples) != 1) {
            goto error_exit;
        }
    }
    else if (key == "num_samples_pct") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%f", &stage->mi_num_spatial_samples_pct) != 1) {
            goto error_exit;
        }
    }
    else if ((key == "demons_std_deformation_field") || (key == "demons_std")) {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->demons_std) != 1) {
            goto error_exit;
        }
    }
    else if (key == "demons_std_update_field") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->demons_std_update_field) != 1) {
            goto error_exit;
        }
    }
    else if (key == "demons_step_length") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->demons_step_length) != 1) {
            goto error_exit;
        }
    }
    else if (key == "demons_smooth_deformation_field") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (string_value_true (val)) {
            stage->demons_smooth_deformation_field = true;
        }
        else
            stage->demons_smooth_deformation_field = false;
    }
    else if (key == "demons_smooth_update_field") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (string_value_true (val)) {
            stage->demons_smooth_update_field = true;
        }
        else
            stage->demons_smooth_update_field = false;
    }
    else if (key == "demons_gradient_type")
    {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "symmetric") {
            stage->demons_gradient_type = SYMMETRIC;
        }
        else if (val == "fixed") {
            stage->demons_gradient_type = FIXED_IMAGE;
        }
        else if (val == "warped_moving") {
            stage->demons_gradient_type = WARPED_MOVING;
        }
        else if (val == "mapped_moving") {
            stage->demons_gradient_type = MAPPED_MOVING;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "num_approx_terms_log_demons") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->num_approx_terms_log_demons) != 1) {
            goto error_exit;
        }
    }
    else if (key == "demons_acceleration") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->demons_acceleration) != 1) {
            goto error_exit;
        }
    }
    else if (key == "demons_homogenization") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->demons_homogenization) != 1) {
            goto error_exit;
        }
    }
    else if (key == "demons_filter_width") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d %d %d", 
                &(stage->demons_filter_width[0]), 
                &(stage->demons_filter_width[1]), 
                &(stage->demons_filter_width[2])) != 3) {
            goto error_exit;
        }
    }
    else if (key == "amoeba_parameter_tol") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &(stage->amoeba_parameter_tol)) != 1) {
            goto error_exit;
        }
    }
    else if (key == "gridsearch_min_overlap") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g %g %g", 
                &(stage->gridsearch_min_overlap[0]), 
                &(stage->gridsearch_min_overlap[1]), 
                &(stage->gridsearch_min_overlap[2])) != 3) {
            goto error_exit;
        }
    }
    else if (key == "gridsearch_strategy") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "global") {
            stage->gridsearch_strategy = GRIDSEARCH_STRATEGY_GLOBAL;
        }
        else if (val == "local") {
            stage->gridsearch_strategy = GRIDSEARCH_STRATEGY_LOCAL;
        }
        else {
            goto error_exit;
        }
    }
    else if (key == "stages") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (val == "global") {
        }
    }
    else if (key == "landmark_stiffness") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->landmark_stiffness) != 1) {
            goto error_exit;
        }
    }   
    else if (key == "landmark_flavor") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%c", &stage->landmark_flavor) != 1) {
            goto error_exit;
        }
    }   
    else if (key == "overlap_penalty_lambda") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->overlap_penalty_lambda) != 1) {
            goto error_exit;
        }
    }   
    else if (key == "overlap_penalty_fraction") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g", &stage->overlap_penalty_fraction) != 1) {
            goto error_exit;
        }
    }   
    else if (key == "res_vox" || key == "res" || key == "ss") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_VOXEL_RATE;
    }
    else if (key == "res_vox_fixed" 
        || key == "ss_fixed" || key == "fixed_ss")
    {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample_fixed (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_VOXEL_RATE;
    }
    else if (key == "res_vox_moving" 
        || key == "ss_moving" || key == "moving_ss")
    {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample_moving (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_VOXEL_RATE;
    }
    else if (key == "res_mm") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_MM;
    }
    else if (key == "res_mm_fixed") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample_fixed (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_MM;
    }
    else if (key == "res_mm_moving") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample_moving (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_MM;
    }
    else if (key == "res_pct") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_PCT;
    }
    else if (key == "res_pct_fixed") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample_fixed (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_PCT;
    }
    else if (key == "res_pct_moving") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        Plm_return_code rc = stage->set_resample_moving (val);
        if (rc != PLM_SUCCESS) {
            goto error_exit;
        }
        stage->resample_type = RESAMPLE_PCT;
    }
    else if (key == "grid_spac"
        || key == "grid_spacing")
    {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%g %g %g", 
                &(stage->grid_spac[0]), 
                &(stage->grid_spac[1]), 
                &(stage->grid_spac[2])) != 3) {
            goto error_exit;
        }
    }
    else if (key == "histo_equ") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (string_value_true (val)) {
            stage->histoeq = true;
        }
        else
            stage->histoeq= false;
    }
    else if (key == "thresh_mean_intensity") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (string_value_true (val)) {
            stage->thresh_mean_intensity = true;
        }
        else
            stage->thresh_mean_intensity= false;
    }
    else if (key == "num_hist_levels") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->num_hist_levels) != 1) {
            goto error_exit;
        }
    }
    else if (key == "num_matching_points") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        if (sscanf (val.c_str(), "%d", &stage->num_matching_points) != 1) {
            goto error_exit;
        }
    }
    else if (key == "debug_dir") {
        if (!section_stage) goto key_only_allowed_in_section_stage;
        stage->debug_dir = val;
    }

    /* The following keywords are only allowed in process section */
    else if (section_process) {
        Process_parms::Pointer pp = stage->get_process_parms ();
        if (key == "action") {
            pp->set_action (val);
        } else {
            pp->set_key_value (key, val);
        }
    }

    else {
        goto error_exit;
    }
    return PLM_SUCCESS;

key_only_allowed_in_section_global:
    lprintf (
        "This key (%s) is only allowed in a global section\n", key.c_str());
    return PLM_ERROR;

key_only_allowed_in_section_stage:
    lprintf (
        "This key (%s) is only allowed in a stage section\n", key.c_str());
    return PLM_ERROR;

key_not_allowed_in_section_process:
    lprintf (
        "This key (%s) not is allowed in a process section\n", key.c_str());
    return PLM_ERROR;

error_exit:
    lprintf (
        "Unknown (key,val) combination: (%s,%s)\n", key.c_str(), val.c_str());
    return PLM_ERROR;
}

int
Registration_parms::set_command_string (
    const std::string& command_string
)
{
    this->delete_all_stages ();
    Registration_parms_parser rpp (this);
    return rpp.parse_config_string (command_string);
}

int
Registration_parms::parse_command_file (const char* options_fn)
{
    /* Read file into string */
    std::ifstream t (options_fn);
    std::stringstream buffer;
    buffer << t.rdbuf();

    /* Parse the string */
    return this->set_command_string (buffer.str());
}

#if defined (GCS_FIX)  // Needs re-engineering
/* JAS 2012.03.13
 *  This is a temp solution */
/* GCS 2012-12-28: Nb. regp->job_idx must be set prior to calling 
   this function */
void
Registration_parms::set_job_paths (void)
{
    /* Setup input paths */
    if (*(this->fixed_dir)) {
        d_ptr->fixed_fn = string_format (
            "%s%s", this->fixed_dir, this->fixed_jobs[this->job_idx]);
    }
    if (*(this->moving_dir)) {
        d_ptr->moving_fn = string_format (
            "%s%s", this->moving_dir, this->moving_jobs[this->job_idx]);
    }

    /* Setup output paths */
    /*   NOTE: For now, output files inherit moving image names */
    if (*(this->img_out_dir)) {
        if (!strcmp (this->img_out_dir, this->moving_dir)) {
            strcpy (this->img_out_fn, this->img_out_dir);
            strcat (this->img_out_fn, "warp/");
            strcat (this->img_out_fn, this->moving_jobs[this->job_idx]);
        } else {
            strcpy (this->img_out_fn, this->img_out_dir);
            strcat (this->img_out_fn, this->moving_jobs[this->job_idx]);
        }
        /* If not dicom, we give a default name */
        if (this->img_out_fmt != IMG_OUT_FMT_DICOM) {
            std::string fn = string_format ("%s.mha", this->img_out_fn);
            strcpy (this->img_out_fn, fn.c_str());
        }
    } else {
        /* Output directory not specifed but img_out was... smart fallback*/
        if (*(this->img_out_fn)) {
            strcpy (this->img_out_fn, this->moving_dir);
            strcat (this->img_out_fn, "warp/");
            strcat (this->img_out_fn, this->moving_jobs[this->job_idx]);
        }
    }
    if (*(this->vf_out_dir)) {
        if (!strcmp (this->vf_out_dir, this->moving_dir)) {
            strcpy (this->vf_out_fn, this->img_out_dir);
            strcat (this->vf_out_fn, "vf/");
            strcat (this->vf_out_fn, this->moving_jobs[this->job_idx]);
        } else {
            strcpy (this->vf_out_fn, this->vf_out_dir);
            strcat (this->vf_out_fn, this->moving_jobs[this->job_idx]);
        }
        /* Give a default name */
        std::string fn = string_format ("%s_vf.mha", this->vf_out_fn);
        strcpy (this->vf_out_fn, fn.c_str());
    } else {
        /* Output directory not specifed but vf_out was... smart fallback*/
        if (*(this->vf_out_fn)) {
            strcpy (this->vf_out_fn, this->moving_dir);
            strcat (this->vf_out_fn, "vf/");
            strcat (this->vf_out_fn, this->moving_jobs[this->job_idx]);
        }
    }
}
#endif

const std::string& 
Registration_parms::get_fixed_fn ()
{
    return d_ptr->fixed_fn;
}

const std::string& 
Registration_parms::get_moving_fn ()
{
    return d_ptr->moving_fn;
}

Shared_parms*
Registration_parms::get_shared_parms ()
{
    return d_ptr->shared;
}

void
Registration_parms::delete_all_stages ()
{
    d_ptr->delete_all_stages ();
    this->num_stages = 0;
}

std::list<Stage_parms*>& 
Registration_parms::get_stages ()
{
    return d_ptr->stages;
}

Stage_parms* 
Registration_parms::append_stage ()
{
    Stage_parms *sp;

    this->num_stages ++;
    if (this->num_stages == 1) {
        sp = new Stage_parms();
    } else {
        sp = new Stage_parms(*d_ptr->stages.back());
    }
    d_ptr->stages.push_back (sp);

    /* Some parameters that should be copied from global 
       to the first stage. */
    if (this->num_stages == 1) {
        sp->default_value = this->default_value;
    }

    sp->stage_no = this->num_stages;

    return sp;
}

Stage_parms* 
Registration_parms::append_process_stage ()
{
    Stage_parms *sp = this->append_stage ();

    Process_parms::Pointer pp = Process_parms::New ();
    sp->set_process_parms (pp);
    return sp;
}
