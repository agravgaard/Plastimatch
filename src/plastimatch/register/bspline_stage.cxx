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
#include "bspline_xform.h"
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
    Registration_data *regd;
    const Stage_parms *stage;
    Xform *xf_in;
    Xform::Pointer xf_out;

    Bspline_parms parms;
    Bspline_optimize bod;

    Volume::Pointer f_stiffness_ss;
public:
    Bspline_stage_private () {
        xf_out = Xform::New ();
    }
};

Bspline_stage::Bspline_stage (
    Registration_data *regd,
    const Stage_parms *stage,
    Xform *xf_in)
{
    d_ptr = new Bspline_stage_private;
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
    /* Run bspline optimization */
    d_ptr->bod.optimize ();
}

void
Bspline_stage::initialize ()
{
    Registration_data *regd = d_ptr->regd;
    const Stage_parms *stage = d_ptr->stage;
    const Shared_parms *shared = d_ptr->stage->get_shared_parms();
    Xform *xf_in = d_ptr->xf_in;
    Xform *xf_out = d_ptr->xf_out.get();
    Bspline_optimize *bod = &d_ptr->bod;
    Bspline_parms *parms = &d_ptr->parms;
    Bspline_state *bst = bod->get_bspline_state ();

    /* Tell bod what parameters to use */
    d_ptr->bod.set_bspline_parms (parms);

    /* Set up metric state */
    populate_similarity_list (bst->similarity_data, regd, stage);

    /* Transform input xform to bspline and give to bod */
    Plm_image_header pih;
    pih.set (bst->similarity_data.front()->fixed_ss);
    xform_to_gpuit_bsp (xf_out, xf_in, &pih, stage->grid_spac);
    Bspline_xform *bxf = xf_out->get_gpuit_bsp();
    d_ptr->bod.set_bspline_xform (bxf);

    /* Set roi's */
    Volume::Pointer m_roi;
    Volume::Pointer f_roi;
    if (shared->fixed_roi_enable && regd->get_fixed_roi()) {
        f_roi = regd->get_fixed_roi()->get_volume_uchar();
    }
    if (shared->moving_roi_enable && regd->get_moving_roi()) {
        m_roi = regd->get_moving_roi()->get_volume_uchar();
    }

    /* Copy parameters from stage_parms to bspline_parms */
    parms->mi_fixed_image_minVal = stage->mi_fixed_image_minVal;
    parms->mi_fixed_image_maxVal = stage->mi_fixed_image_maxVal;
    parms->mi_moving_image_minVal = stage->mi_moving_image_minVal;
    parms->mi_moving_image_maxVal = stage->mi_moving_image_maxVal;

    /* GCS FIX BEGIN */
    /* BSpline code needs work to support multi-planar imaging 
       until that is done, this should maintain the old behavior */
    Volume::Pointer fixed = regd->get_fixed_image()->get_volume_float();
    Volume::Pointer moving = regd->get_moving_image()->get_volume_float();
    
    // Check if min/max values for moving image are set (correctly)
    if ((parms->mi_moving_image_minVal!=0 
            || parms->mi_moving_image_maxVal!=0) 
        && (parms->mi_moving_image_minVal < 
            parms->mi_moving_image_maxVal))
    {
        bool fill=!m_roi;

        // create new moving roi if not available
        if (!m_roi)
        {
            m_roi = Volume::New ();
            m_roi->create (moving->dim, moving->origin, moving->spacing,
                moving->direction_cosines, PT_UCHAR);
        }

        // Modify fixed roi according to min and max values for moving image
        update_roi (m_roi, moving, parms->mi_moving_image_minVal,
            parms->mi_moving_image_maxVal,fill);
    }

    // Check if min/max values for fixed image are set (correctly)
    if ((parms->mi_fixed_image_minVal!=0 
            || parms->mi_fixed_image_maxVal!=0) 
        && (parms->mi_fixed_image_minVal < 
            parms->mi_fixed_image_maxVal))
    {
        bool fill=!f_roi;

        // create new fixed roi if not available
        if (!f_roi)
        {
            f_roi = Volume::New ();
            f_roi->create (fixed->dim, fixed->origin, fixed->spacing,
                fixed->direction_cosines, PT_UCHAR);
        }

        // Modify fixed roi according to min and max values for fixed image
        update_roi (f_roi, fixed, parms->mi_fixed_image_minVal,
            parms->mi_fixed_image_maxVal, fill);
    }

    /* Subsample rois (if we are using them) */
    if (m_roi) {
        Volume::Pointer m_roi_ss = 
            volume_subsample_vox_legacy_nn (
                m_roi, stage->resample_rate_moving);
        bst->similarity_data.front()->moving_roi = m_roi_ss;
    }
    if (f_roi) {
        Volume::Pointer f_roi_ss = 
            volume_subsample_vox_legacy_nn (
                f_roi, stage->resample_rate_fixed);
        bst->similarity_data.front()->fixed_roi = f_roi_ss;
    }

    /* GCS FIX END */

    /* Subsample stiffness */
    if (shared->fixed_stiffness_enable && regd->fixed_stiffness) {
        Volume::Pointer& stiffness 
            = regd->fixed_stiffness->get_volume_float ();
        d_ptr->f_stiffness_ss = registration_resample_volume (
            stiffness, stage, stage->resample_rate_fixed);
    }

    /* Stiffness image */
    if (d_ptr->f_stiffness_ss) {
        parms->fixed_stiffness = d_ptr->f_stiffness_ss.get();
    }

    /* Optimization */
    if (stage->optim_type == OPTIMIZATION_STEEPEST) {
        parms->optimization = BOPT_STEEPEST;
    } else if (stage->optim_type == OPTIMIZATION_LIBLBFGS) {
        parms->optimization = BOPT_LIBLBFGS;
    } else {
        parms->optimization = BOPT_LBFGSB;
    }
    parms->lbfgsb_pgtol = stage->pgtol;
    parms->lbfgsb_mmax = stage->lbfgsb_mmax;

    /* Threading */
    switch (stage->threading_type) {
    case THREADING_CPU_SINGLE:
        if (stage->alg_flavor == 0) {
            parms->implementation = 'h';
        } else {
            parms->implementation = stage->alg_flavor;
        }
        parms->threading = BTHR_CPU;
        break;
    case THREADING_CPU_OPENMP:
        if (stage->alg_flavor == 0) {
            parms->implementation = 'g';
        } else {
            parms->implementation = stage->alg_flavor;
        }
        parms->threading = BTHR_CPU;
        break;
    case THREADING_CUDA:
        if (stage->alg_flavor == 0) {
            parms->implementation = 'j';
        } else {
            parms->implementation = stage->alg_flavor;
        }
        parms->threading = BTHR_CUDA;
        break;
    default:
        print_and_exit ("Undefined impl type in gpuit_bspline\n");
    }
    logfile_printf ("Algorithm flavor = %c\n", parms->implementation);
    logfile_printf ("Threading = %d\n", parms->threading);

    if (stage->threading_type == THREADING_CUDA) {
        parms->gpuid = stage->gpuid;
        logfile_printf ("GPU ID = %d\n", parms->gpuid);
    }
    
    /* Regularization */
    parms->reg_parms->lambda = stage->regularization_lambda;
    switch (stage->regularization_type) {
    case REGULARIZATION_NONE:
        parms->reg_parms->lambda = 0.0f;
        break;
    case REGULARIZATION_BSPLINE_ANALYTIC:
        if (stage->threading_type == THREADING_CPU_OPENMP) {
            parms->reg_parms->implementation = 'c';
        } else {
            parms->reg_parms->implementation = 'b';
        }
        break;
    case REGULARIZATION_BSPLINE_SEMI_ANALYTIC:
        parms->reg_parms->implementation = 'd';
        break;
    case REGULARIZATION_BSPLINE_NUMERIC:
        parms->reg_parms->implementation = 'a';
        break;
    default:
        print_and_exit ("Undefined regularization type in gpuit_bspline\n");
    }
    if (stage->regularization_lambda != 0) {
        parms->reg_parms->lambda = stage->regularization_lambda;
    }
    logfile_printf ("Regularization: flavor = %c lambda = %f\n", 
        parms->reg_parms->implementation,
        parms->reg_parms->lambda);

    /* Mutual information histograms */
    parms->mi_hist_type = stage->mi_hist_type;
    parms->mi_hist_fixed_bins = stage->mi_hist_fixed_bins;
    parms->mi_hist_moving_bins = stage->mi_hist_moving_bins;

    /* Other stuff */
    parms->min_its = stage->min_its;
    parms->max_its = stage->max_its;
    parms->max_feval = stage->max_its;
    parms->convergence_tol = stage->convergence_tol;

    /* Landmarks */
    if (regd->fixed_landmarks && regd->moving_landmarks) {
        logfile_printf ("Landmarks: %d fixed, %d moving, lambda = %f\n",
            regd->fixed_landmarks->get_count(),
            regd->moving_landmarks->get_count(),
            stage->landmark_stiffness);
        parms->blm->set_landmarks (regd->fixed_landmarks, 
            regd->moving_landmarks);
        parms->blm->landmark_implementation = stage->landmark_flavor;
        parms->blm->landmark_stiffness = stage->landmark_stiffness;
    }

    /* Set debugging directory */
    if (stage->debug_dir != "") {
        parms->debug = 1;
        parms->debug_dir = stage->debug_dir;
        parms->debug_stage = stage->stage_no;
        logfile_printf ("Set debug directory to %s (%d)\n", 
            parms->debug_dir.c_str(), parms->debug_stage);

        /* Write fixed, moving, moving_grad */
        std::string fn;
        fn = string_format ("%s/%02d/fixed.mha",
            parms->debug_dir.c_str(), parms->debug_stage);
        write_mha (fn.c_str(), 
            bst->similarity_data.front()->fixed_ss.get());
        fn = string_format ("%s/%02d/moving.mha",
            parms->debug_dir.c_str(), parms->debug_stage);
        write_mha (fn.c_str(),
            bst->similarity_data.front()->moving_ss.get());
        fn = string_format ("%s/%02d/moving_grad.mha", 
            parms->debug_dir.c_str(), parms->debug_stage);
        write_mha (fn.c_str(), 
            bst->similarity_data.front()->moving_grad.get());
    }
}

void
Bspline_stage::cleanup ()
{
}

Xform::Pointer
do_gpuit_bspline_stage (
    Registration_data* regd, 
    const Xform::Pointer& xf_in,
    const Stage_parms* stage)
{
    Xform::Pointer xf_out = Xform::New ();
    Bspline_stage pb (regd, stage, xf_in.get());
    pb.run_stage ();
    xf_out = pb.d_ptr->xf_out;
    return xf_out;
}
