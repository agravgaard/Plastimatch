/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bspline.h"
#include "bspline_interpolate.h"
#include "bspline_landmarks.h"
#include "bspline_optimize.h"
#include "bspline_regularize.h"
#include "bspline_parms.h"
#include "bspline_stage.h"
#include "logfile.h"
#include "mha_io.h"
#include "plm_image_header.h"
#include "plm_image.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "registration_data.h"
#include "registration_parms.h"
#include "registration_resample.h"
#include "shared_parms.h"
#include "stage_parms.h"
#include "string_util.h"
#include "volume.h"
#include "volume_grad.h"
#include "volume_header.h"
#include "volume_resample.h"
#include "xform.h"

class Bspline_stage_private {
public:
    Registration_parms *regp;
    Registration_data *regd;
    const Stage_parms *stage;
    Xform *xf_in;
    Xform::Pointer xf_out;
    Bspline_parms bsp_parms;

    Volume::Pointer fixed_ss;
    Volume::Pointer moving_ss;
    Volume::Pointer moving_grad;
    Volume::Pointer f_roi_ss;
    Volume::Pointer m_roi_ss;
    Volume::Pointer f_stiffness_ss;
public:
    Bspline_stage_private () {
        xf_out = Xform::New ();
    }
};

Bspline_stage::Bspline_stage (
    Registration_parms *regp,
    Registration_data *regd,
    const Stage_parms *stage,
    Xform *xf_in)
{
    d_ptr = new Bspline_stage_private;
    d_ptr->regp = regp;
    d_ptr->regd = regd;
    d_ptr->stage = stage;
    d_ptr->xf_in = xf_in;

    initialize ();
}

Bspline_stage::~Bspline_stage ()
{
    this->cleanup ();
    delete d_ptr;
}

static void
update_roi (Volume::Pointer& roi, Volume::Pointer& image, float min_val, 
    float max_val, bool fill_empty_roi)
{
    plm_long p=0;
    float* image_temp = (float*)image->img;
    unsigned char* roi_temp = roi->get_raw<unsigned char> ();
    for (unsigned int i=0; i < roi->dim[2]; i++) {
        for (unsigned int j=0; j < roi->dim[1]; j++) {
            for (unsigned int k=0; k < roi->dim[0]; k++) {
                if (fill_empty_roi 
                    && (image_temp[p]>=min_val && image_temp[p]<=max_val))
                {
                    roi_temp[p]=(unsigned char)1;
                }
                else if ((image_temp[p]<min_val || image_temp[p]>max_val) 
                    && roi_temp[p]>0)
                {
                    roi_temp[p]=(unsigned char)0;
                }
                p++;
            }
        }
    }
}

void
Bspline_stage::run_stage ()
{
    Xform *xf_out = d_ptr->xf_out.get();
    Bspline_parms *bsp_parms = &d_ptr->bsp_parms;

    /* Run bspline optimization */
    bspline_optimize (xf_out->get_gpuit_bsp(), bsp_parms);
}

void
Bspline_stage::initialize ()
{
    Registration_data *regd = d_ptr->regd;
    const Stage_parms *stage = d_ptr->stage;
    const Shared_parms *shared = d_ptr->stage->get_shared_parms();
    Xform *xf_in = d_ptr->xf_in;
    Xform *xf_out = d_ptr->xf_out.get();
    Bspline_parms *bsp_parms = &d_ptr->bsp_parms;

    Plm_image_header pih;

    logfile_printf ("Converting fixed\n");
    Volume::Pointer& fixed = regd->fixed_image->get_volume_float ();
    logfile_printf ("Converting moving\n");
    Volume::Pointer& moving = regd->moving_image->get_volume_float ();
    logfile_printf ("Done.\n");

    Volume::Pointer m_roi;
    Volume::Pointer f_roi;

    /* Set roi's */
    if (shared->fixed_roi_enable && regd->fixed_roi) {
        f_roi = regd->fixed_roi->get_volume_uchar();
    }
    if (shared->moving_roi_enable && regd->moving_roi) {
        m_roi = regd->moving_roi->get_volume_uchar();
    }

    /* Convert images to gpuit format */
    fixed->convert (PT_FLOAT);              /* Maybe not necessary? */
    moving->convert (PT_FLOAT);             /* Maybe not necessary? */

    /* Subsample images */
    d_ptr->fixed_ss = registration_resample_volume (
        fixed, stage, stage->resample_rate_fixed);
    d_ptr->moving_ss = registration_resample_volume (
        moving, stage, stage->resample_rate_moving);

    /* Gradient magnitude uses different fixed and moving images */
    if (stage->metric_type[0] == REGISTRATION_METRIC_GM) {
        d_ptr->fixed_ss = volume_gradient_magnitude (d_ptr->fixed_ss);
        d_ptr->moving_ss = volume_gradient_magnitude (d_ptr->moving_ss);
    }

    /* Copy parameters from stage_parms to bspline_parms */
    bsp_parms->mi_fixed_image_minVal = stage->mi_fixed_image_minVal;
    bsp_parms->mi_fixed_image_maxVal = stage->mi_fixed_image_maxVal;
    bsp_parms->mi_moving_image_minVal = stage->mi_moving_image_minVal;
    bsp_parms->mi_moving_image_maxVal = stage->mi_moving_image_maxVal;

    //Check if min/max values for moving image are set (correctly)
    if ((bsp_parms->mi_moving_image_minVal!=0 
            || bsp_parms->mi_moving_image_maxVal!=0) 
        && (bsp_parms->mi_moving_image_minVal < 
            bsp_parms->mi_moving_image_maxVal))
    {
        bool fill=!m_roi;

        //create new moving roi if not available
        if (!m_roi)
        {
            m_roi = Volume::New ();
            m_roi->create (moving->dim, moving->origin, moving->spacing,
                moving->direction_cosines, PT_UCHAR);
        }

        //Modify fixed roi according to min and max values for moving image
        update_roi (m_roi, moving, bsp_parms->mi_moving_image_minVal,
            bsp_parms->mi_moving_image_maxVal,fill);
    }

    //Check if min/max values for fixed image are set (correctly)
    if ((bsp_parms->mi_fixed_image_minVal!=0 
            || bsp_parms->mi_fixed_image_maxVal!=0) 
        && (bsp_parms->mi_fixed_image_minVal < 
            bsp_parms->mi_fixed_image_maxVal))
    {
        bool fill=!f_roi;

        //create new fixed roi if not available
        if(!f_roi)
        {
            f_roi = Volume::New ();
            f_roi->create (fixed->dim, fixed->origin, fixed->spacing,
                fixed->direction_cosines, PT_UCHAR);
        }

        //Modify fixed roi according to min and max values for fixed image
        update_roi (f_roi, fixed, bsp_parms->mi_fixed_image_minVal,
            bsp_parms->mi_fixed_image_maxVal, fill);
    }

    /* Subsample rois (if we are using them) */
    if (m_roi) {
        d_ptr->m_roi_ss = volume_subsample_vox_legacy_nn (
            m_roi, stage->resample_rate_moving);
    }
    if (f_roi) {
        d_ptr->f_roi_ss = volume_subsample_vox_legacy_nn (
            f_roi, stage->resample_rate_fixed);
    }

    logfile_printf ("moving_ss size = %d %d %d\n", 
        d_ptr->moving_ss->dim[0], 
        d_ptr->moving_ss->dim[1], 
        d_ptr->moving_ss->dim[2]);
    logfile_printf ("fixed_ss size = %d %d %d\n", 
        d_ptr->fixed_ss->dim[0], 
        d_ptr->fixed_ss->dim[1], 
        d_ptr->fixed_ss->dim[2]);

    /* Subsample stiffness */
    if (shared->fixed_stiffness_enable && regd->fixed_stiffness) {
        Volume::Pointer& stiffness 
            = regd->fixed_stiffness->get_volume_float ();
        d_ptr->f_stiffness_ss = registration_resample_volume (
            stiffness, stage, stage->resample_rate_fixed);
    }

    /* Make spatial gradient image */
    Volume *moving_grad = volume_make_gradient (d_ptr->moving_ss.get());
    d_ptr->moving_grad = Volume::New (moving_grad);

    /* --- Initialize parms --- */

    /* Images */
    bsp_parms->fixed = d_ptr->fixed_ss.get();
    bsp_parms->moving = d_ptr->moving_ss.get();
    bsp_parms->moving_grad = d_ptr->moving_grad.get();
    if (f_roi) {
        bsp_parms->fixed_roi = d_ptr->f_roi_ss.get();
    }
    if (m_roi) {
        bsp_parms->moving_roi = d_ptr->m_roi_ss.get();
    }
    if (d_ptr->f_stiffness_ss) {
        bsp_parms->fixed_stiffness = d_ptr->f_stiffness_ss.get();
    }

    /* Optimization */
    if (stage->optim_type == OPTIMIZATION_STEEPEST) {
        bsp_parms->optimization = BOPT_STEEPEST;
    } else if (stage->optim_type == OPTIMIZATION_LIBLBFGS) {
        bsp_parms->optimization = BOPT_LIBLBFGS;
    } else {
        bsp_parms->optimization = BOPT_LBFGSB;
    }
    bsp_parms->lbfgsb_pgtol = stage->pgtol;
    bsp_parms->lbfgsb_mmax = stage->lbfgsb_mmax;

    /* Metric */
    bsp_parms->metric_type = stage->metric_type;
    bsp_parms->metric_lambda = stage->metric_lambda;
    for (size_t i = 0; i < stage->metric_type.size(); i++) {
        if (bsp_parms->metric_type[i] == REGISTRATION_METRIC_MI_VW) {
            bsp_parms->metric_type[i] = REGISTRATION_METRIC_MI_MATTES;
        }
    }

    /* Threading */
    switch (stage->threading_type) {
    case THREADING_CPU_SINGLE:
        if (stage->alg_flavor == 0) {
            bsp_parms->implementation = 'h';
        } else {
            bsp_parms->implementation = stage->alg_flavor;
        }
        bsp_parms->threading = BTHR_CPU;
        break;
    case THREADING_CPU_OPENMP:
        if (stage->alg_flavor == 0) {
            bsp_parms->implementation = 'g';
        } else {
            bsp_parms->implementation = stage->alg_flavor;
        }
        bsp_parms->threading = BTHR_CPU;
        break;
    case THREADING_CUDA:
        if (stage->alg_flavor == 0) {
            bsp_parms->implementation = 'j';
        } else {
            bsp_parms->implementation = stage->alg_flavor;
        }
        bsp_parms->threading = BTHR_CUDA;
        break;
    default:
        print_and_exit ("Undefined impl type in gpuit_bspline\n");
    }
    logfile_printf ("Algorithm flavor = %c\n", bsp_parms->implementation);
    logfile_printf ("Threading = %d\n", bsp_parms->threading);

    if (stage->threading_type == THREADING_CUDA) {
        bsp_parms->gpuid = stage->gpuid;
        logfile_printf ("GPU ID = %d\n", bsp_parms->gpuid);
    }
    
    /* Regularization */
    bsp_parms->reg_parms->lambda = stage->regularization_lambda;
    switch (stage->regularization_type) {
    case REGULARIZATION_NONE:
        bsp_parms->reg_parms->lambda = 0.0f;
        break;
    case REGULARIZATION_BSPLINE_ANALYTIC:
        if (stage->threading_type == THREADING_CPU_OPENMP) {
            bsp_parms->reg_parms->implementation = 'c';
        } else {
            bsp_parms->reg_parms->implementation = 'b';
        }
        break;
    case REGULARIZATION_BSPLINE_SEMI_ANALYTIC:
        bsp_parms->reg_parms->implementation = 'd';
        break;
    case REGULARIZATION_BSPLINE_NUMERIC:
        bsp_parms->reg_parms->implementation = 'a';
        break;
    default:
        print_and_exit ("Undefined regularization type in gpuit_bspline\n");
    }
    if (stage->regularization_lambda != 0) {
        bsp_parms->reg_parms->lambda = stage->regularization_lambda;
    }
    logfile_printf ("Regularization: flavor = %c lambda = %f\n", 
        bsp_parms->reg_parms->implementation,
        bsp_parms->reg_parms->lambda);

    /* Mutual information histograms */
    bsp_parms->mi_hist_type = stage->mi_hist_type;
    bsp_parms->mi_hist_fixed_bins = stage->mi_hist_fixed_bins;
    bsp_parms->mi_hist_moving_bins = stage->mi_hist_moving_bins;

    /* Other stuff */
    bsp_parms->min_its = stage->min_its;
    bsp_parms->max_its = stage->max_its;
    bsp_parms->max_feval = stage->max_its;
    bsp_parms->convergence_tol = stage->convergence_tol;

    /* Landmarks */
    if (regd->fixed_landmarks && regd->moving_landmarks) {
        logfile_printf ("Landmarks: %d fixed, %d moving, lambda = %f\n",
            regd->fixed_landmarks->get_count(),
            regd->moving_landmarks->get_count(),
            stage->landmark_stiffness);
        bsp_parms->blm->set_landmarks (regd->fixed_landmarks, 
            regd->moving_landmarks);
        bsp_parms->blm->landmark_implementation = stage->landmark_flavor;
        bsp_parms->blm->landmark_stiffness = stage->landmark_stiffness;
    }

    /* Transform input xform to gpuit vector field */
    pih.set_from_gpuit (d_ptr->fixed_ss->dim, 
        d_ptr->fixed_ss->origin, d_ptr->fixed_ss->spacing, 
        d_ptr->fixed_ss->direction_cosines);
    xform_to_gpuit_bsp (xf_out, xf_in, &pih, stage->grid_spac);

    /* Set debugging directory */
    if (stage->debug_dir != "") {
        bsp_parms->debug = 1;
        bsp_parms->debug_dir = stage->debug_dir;
        bsp_parms->debug_stage = stage->stage_no;
        logfile_printf ("Set debug directory to %s (%d)\n", 
            bsp_parms->debug_dir.c_str(), bsp_parms->debug_stage);

        /* Write fixed, moving, moving_grad */
        std::string fn;
        fn = string_format ("%s/%02d/moving.mha",
            bsp_parms->debug_dir.c_str(), bsp_parms->debug_stage);
        write_mha (fn.c_str(), d_ptr->moving_ss.get());
        fn = string_format ("%s/%02d/fixed.mha",
            bsp_parms->debug_dir.c_str(), bsp_parms->debug_stage);
        write_mha (fn.c_str(), d_ptr->fixed_ss.get());
        fn = string_format ("%s/%02d/moving_grad.mha", 
            bsp_parms->debug_dir.c_str(), bsp_parms->debug_stage);
        write_mha (fn.c_str(), d_ptr->moving_grad.get());
    }
}

void
Bspline_stage::cleanup ()
{
}

Xform::Pointer
do_gpuit_bspline_stage (
    Registration_parms* regp, 
    Registration_data* regd, 
    const Xform::Pointer& xf_in,
    const Stage_parms* stage)
{
    Xform::Pointer xf_out = Xform::New ();
    Bspline_stage pb (regp, regd, stage, xf_in.get());
    pb.run_stage ();
    xf_out = pb.d_ptr->xf_out;
    return xf_out;
}
