/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdlib.h>
#include <string.h>
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkMultiThreader.h"

#include "bspline_stage.h"
#include "bspline_xform.h"
#include "dlib_threads.h"
#include "gpuit_demons.h"
#include "itk_align_center.h"
#include "itk_demons.h"
#include "itk_image_save.h"
#include "itk_image_stats.h"
#include "itk_registration.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_timer.h"
#include "plm_warp.h"
#include "pointset_warp.h"
#include "registration.h"
#include "registration_data.h"
#include "registration_parms.h"
#include "shared_parms.h"
#include "stage_parms.h"
#include "translation_grid_search.h"
#include "volume.h"
#include "xform.h"

class Registration_private
{
public:
    Registration_data::Pointer rdata;
    Registration_parms::Pointer rparms;

    Xform::Pointer xf_in;
    Xform::Pointer xf_out;

    itk::MultiThreader::Pointer threader;
    Dlib_master_slave master_slave;
    Dlib_semaphore worker_running;
    int thread_no;
    bool time_to_quit;

public:
    Registration_private () {
        rdata = Registration_data::New ();
        rparms = Registration_parms::New ();
        xf_in = Xform::New ();
        xf_out = Xform::New ();

        threader = itk::MultiThreader::New ();
        thread_no = -1;
        time_to_quit = false;
    }
    ~Registration_private () {
        this->time_to_quit = true;
        // do something more here ... wait for running threads to exit
    }
};

Registration::Registration ()
{
    d_ptr = new Registration_private;
}

Registration::~Registration ()
{
    delete d_ptr;
}

int
Registration::set_command_file (const std::string& command_file)
{
    return d_ptr->rparms->parse_command_file (command_file.c_str());
}

int
Registration::set_command_string (const std::string& command_string)
{
    return d_ptr->rparms->set_command_string (command_string);
}

void
Registration::set_fixed_image (Plm_image::Pointer& fixed)
{
    d_ptr->rdata->fixed_image = fixed;
}

void
Registration::set_moving_image (Plm_image::Pointer& moving)
{
    d_ptr->rdata->moving_image = moving;
}

void
Registration::set_fixed_roi (Plm_image::Pointer& fixed_roi)
{
    d_ptr->rdata->fixed_roi = fixed_roi;
}

void
Registration::set_moving_roi (Plm_image::Pointer& moving_roi)
{
    d_ptr->rdata->moving_roi = moving_roi;
}

Registration_data::Pointer
Registration::get_registration_data ()
{
    return d_ptr->rdata;
}

Registration_parms::Pointer
Registration::get_registration_parms ()
{
    return d_ptr->rparms;
}


/* This helps speed up the registration, by setting the bounding box to the 
   smallest size needed.  To find the bounding box, either use the extent 
   of the fixed_roi (if one is used), or by eliminating excess air 
   by thresholding */
static void
set_fixed_image_region_global (Registration_data::Pointer& regd)
{
    int use_magic_value = 1;

    regd->fixed_region_origin = regd->fixed_image->itk_float()->GetOrigin();
    regd->fixed_region_spacing = regd->fixed_image->itk_float()->GetSpacing();

    if (regd->fixed_roi) {
        FloatImageType::RegionType::IndexType valid_index;
        FloatImageType::RegionType::SizeType valid_size;

        /* Search for bounding box of fixed roi */
        typedef itk::ImageRegionConstIteratorWithIndex< 
            UCharImageType > IteratorType;
        UCharImageType::RegionType region 
            = regd->fixed_roi->itk_uchar()->GetLargestPossibleRegion();
        IteratorType it (regd->fixed_roi->itk_uchar(), region);

        int first = 1;
        valid_index[0] = 0;
        valid_index[1] = 0;
        valid_index[2] = 0;
        valid_size[0] = 0;
        valid_size[1] = 0;
        valid_size[2] = 0;

        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            unsigned char c = it.Get();
            if (c) {
                UCharImageType::RegionType::IndexType idx = it.GetIndex();
                if (first) {
                    first = 0;
                    valid_index = idx;
                    valid_size[0] = 1;
                    valid_size[1] = 1;
                    valid_size[2] = 1;
                } else {
                    for (int i = 0; i < 3; i++) {
                        if (valid_index[i] > idx[i]) {
                            valid_size[i] += valid_index[i] - idx[i];
                            valid_index[i] = idx[i];
                        }
                        if (idx[i] - valid_index[i] >= (long) valid_size[i]) {
                            valid_size[i] = idx[i] - valid_index[i] + 1;
                        }
                    }
                }
            }
        }
        regd->fixed_region.SetIndex(valid_index);
        regd->fixed_region.SetSize(valid_size);
    } else if (use_magic_value) {
        FloatImageType::RegionType::IndexType valid_index;
        FloatImageType::RegionType::SizeType valid_size;
        valid_index[0] = 0;
        valid_index[1] = 0;
        valid_index[2] = 0;
        valid_size[0] = 1;
        valid_size[1] = 1;
        valid_size[2] = 1;

        /* Make sure the image is ITK float */
        FloatImageType::Pointer fixed_image = regd->fixed_image->itk_float();

        /* Search for bounding box of patient */
        typedef itk::ImageRegionConstIteratorWithIndex <
            FloatImageType > IteratorType;
        FloatImageType::RegionType region 
            = fixed_image->GetLargestPossibleRegion();
        IteratorType it (fixed_image, region);

        int first = 1;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            float c = it.Get();
#define FIXME_BACKGROUND_MAX (-1200)
            if (c > FIXME_BACKGROUND_MAX) {
                FloatImageType::RegionType::IndexType idx = it.GetIndex();
                if (first) {
                    first = 0;
                    valid_index = idx;
                    valid_size[0] = 1;
                    valid_size[1] = 1;
                    valid_size[2] = 1;
                } else {
                    for (int i = 0; i < 3; i++) {
                        if (valid_index[i] > idx[i]) {
                            valid_size[i] += valid_index[i] - idx[i];
                            valid_index[i] = idx[i];
                        }
                        if (idx[i] - valid_index[i] >= (long) valid_size[i]) {
                            valid_size[i] = idx[i] - valid_index[i] + 1;
                        }
                    }
                }
            }
        }
        /* Try to include a margin of at least one air pixel everywhere */
        for (int i = 0; i < 3; i++) {
            if (valid_index[i] > 0) {
                valid_index[i]--;
                valid_size[i]++;
            }
            if (valid_size[i] + valid_index[i] 
                < fixed_image->GetLargestPossibleRegion().GetSize()[i])
            {
                valid_size[i]++;
            }
        }
        regd->fixed_region.SetIndex(valid_index);
        regd->fixed_region.SetSize(valid_size);
    } else {
        regd->fixed_region 
            = regd->fixed_image->itk_float()->GetLargestPossibleRegion();
    }
}

static void
save_output (
    Registration_data* regd, 
    Xform::Pointer& xf_out, 
    const std::list<std::string>& xf_out_fn, 
    bool xf_out_itk,
    int img_out_fmt,
    Plm_image_type img_out_type,
    float default_value, 
    const std::string& img_out_fn,
    const std::string& vf_out_fn,
    const std::string& warped_landmarks_fn
)
{
    /* Handle null xf, make it zero translation */
    if (xf_out->m_type == XFORM_NONE) {
        xf_out->init_trn ();
    }

    /* Save xf to all filenames in list */
    std::list<std::string>::const_iterator it;
    for (it = xf_out_fn.begin(); it != xf_out_fn.end(); ++it) {
        logfile_printf ("Writing transformation ...\n");
        if (xf_out_itk && xf_out->m_type == XFORM_GPUIT_BSPLINE) {
            Plm_image_header pih;
            pih.set_from_plm_image (regd->fixed_image);
            Xform::Pointer xf_tmp = xform_to_itk_bsp (xf_out, &pih, 0);
            xf_tmp->save (*it);
        } else {
            xf_out->save (*it);
        }
    }

    if (img_out_fn[0] || vf_out_fn[0] || warped_landmarks_fn[0]) {
        DeformationFieldType::Pointer vf;
        DeformationFieldType::Pointer *vfp;
        Plm_image::Pointer im_warped;
        Plm_image_header pih;

        if (vf_out_fn[0] || warped_landmarks_fn[0]) {
            vfp = &vf;
        } else {
            vfp = 0;
        }
        if (img_out_fn[0]) {
            im_warped = Plm_image::New();
        }
        
        pih.set_from_plm_image (regd->fixed_image);

        logfile_printf ("Warping...\n");
        plm_warp (im_warped, vfp, xf_out, &pih, regd->moving_image, 
            default_value, 0, 1);

        if (img_out_fn[0]) {
            logfile_printf ("Saving image...\n");
            if (img_out_fmt == IMG_OUT_FMT_AUTO) {
                if (img_out_type == PLM_IMG_TYPE_UNDEFINED) {
                    im_warped->save_image (img_out_fn);
                } else {
                    im_warped->convert_and_save (img_out_fn, img_out_type);
                }
            } else {
                im_warped->save_short_dicom (img_out_fn, 0);
            }
        }
        if (warped_landmarks_fn[0]) {
            Labeled_pointset warped_pointset;
            logfile_printf ("Saving warped landmarks...\n");
            pointset_warp (&warped_pointset, regd->moving_landmarks, vf);
            warped_pointset.save (warped_landmarks_fn.c_str());
        }
        if (vf_out_fn[0]) {
            logfile_printf ("Saving vf...\n");
            itk_image_save (vf, vf_out_fn);
        }
    }
}

Xform::Pointer
Registration::do_registration_stage (
    Stage_parms* stage            /* Input */
)
{
    Registration_data::Pointer regd = d_ptr->rdata;
    Registration_parms::Pointer regp = d_ptr->rparms;
    const Xform::Pointer& xf_in = d_ptr->xf_in;

    Xform::Pointer xf_out = Xform::New ();
    lprintf ("[1] xf_in->m_type = %d, xf_out->m_type = %d\n", 
        xf_in->m_type, xf_out->m_type);

    /* Run registration */
    switch (stage->xform_type) {
    case STAGE_TRANSFORM_ALIGN_CENTER:
        xf_out = do_itk_align_center (regd.get(), xf_in, stage);
        break;
    case STAGE_TRANSFORM_ALIGN_CENTER_OF_GRAVITY:
        xf_out = do_itk_align_center_of_gravity (regd.get(), xf_in, stage);
        break;
    case STAGE_TRANSFORM_TRANSLATION:
        if (stage->optim_type == OPTIMIZATION_ALIGN_CENTER) {
            xf_out = do_itk_align_center (regd.get(), xf_in, stage);
        } else if (stage->optim_type == OPTIMIZATION_ALIGN_ROI_CENTER) {
            xf_out = do_itk_align_center_of_gravity (regd.get(), xf_in, stage);
        } else if (stage->impl_type == IMPLEMENTATION_ITK) {
            xf_out = do_itk_registration_stage (regd.get(), xf_in, stage);
        } else if (stage->impl_type == IMPLEMENTATION_PLASTIMATCH) {
            xf_out = translation_grid_search_stage (regd.get(), xf_in, stage);
        } else {
            if (stage->optim_type == OPTIMIZATION_GRID_SEARCH) {
                xf_out = translation_grid_search_stage (
                    regd.get(), xf_in, stage);
            } else {
                xf_out = do_itk_registration_stage (regd.get(), xf_in, stage);
            }
        }
        break;
    case STAGE_TRANSFORM_NONE:
    case STAGE_TRANSFORM_VERSOR:
    case STAGE_TRANSFORM_QUATERNION:
    case STAGE_TRANSFORM_SIMILARITY:
    case STAGE_TRANSFORM_AFFINE:
        xf_out = do_itk_registration_stage (regd.get(), xf_in, stage);
        break;
    case STAGE_TRANSFORM_BSPLINE:
        if (stage->impl_type == IMPLEMENTATION_ITK) {
            xf_out = do_itk_registration_stage (regd.get(), xf_in, stage);
        } else {
            xf_out = do_gpuit_bspline_stage (regp.get(), regd.get(), 
                xf_in, stage);
        }
        break;
    case STAGE_TRANSFORM_VECTOR_FIELD:
        if (stage->impl_type == IMPLEMENTATION_ITK) {
            xf_out = do_itk_demons_stage (regd.get(), xf_in, stage);
        } else {
            xf_out = do_gpuit_demons_stage (regd.get(), xf_in, stage);
        }
        break;
    }

    lprintf ("[2] xf_out->m_type = %d, xf_in->m_type = %d\n", 
        xf_out->m_type, xf_in->m_type);

    return xf_out;
}

static void
set_auto_resample (float subsample_rate[], Plm_image *pli)
{
    Plm_image_header pih (pli);

    for (int d = 0; d < 3; d++) {
        subsample_rate[d] = (float) ((pih.dim(d)+99) / 100);
    }
}

static void
set_automatic_parameters (
    Registration_data::Pointer& regd, 
    Registration_parms::Pointer& regp)
{
    std::list<Stage_parms*>& stages = regp->get_stages();
    std::list<Stage_parms*>::iterator it;
    for (it = stages.begin(); it != stages.end(); it++) {
        Stage_parms* sp = *it;
        if (sp->resample_type == RESAMPLE_AUTO) {
            set_auto_resample (
                sp->resample_rate_fixed, regd->fixed_image.get());
            set_auto_resample (
                sp->resample_rate_moving, regd->moving_image.get());
        }
    }
}

static void
check_output_resolution (Xform::Pointer& xf_out, Registration_data::Pointer& regd)
{
    Volume *fixed = regd->fixed_image->get_vol ();
    int ss[3];
    Plm_image_header pih;
    float grid_spacing[3];

    if (xf_out->get_type() != XFORM_GPUIT_BSPLINE) {
        return;
    }

    Bspline_xform *bxf_out = xf_out->get_gpuit_bsp();
    if ( (bxf_out->img_dim[0] != fixed->dim[0]) ||
         (bxf_out->img_dim[1] != fixed->dim[1]) ||
         (bxf_out->img_dim[2] != fixed->dim[2]) ) {

        ss[0] = fixed->dim[0] / bxf_out->img_dim[0];
        ss[1] = fixed->dim[1] / bxf_out->img_dim[1];
        ss[2] = fixed->dim[2] / bxf_out->img_dim[2];

        /* last stage was not [1 1 1], un-subsample the final xform */
        logfile_printf ("RESTORE NATIVE RESOLUTION: (%d %d %d), (1 1 1)\n",
                ss[0], ss[1], ss[2]);

        /* Transform input xform to gpuit vector field */
        pih.set_from_gpuit (
            fixed->dim, 
            fixed->origin,
            fixed->spacing, 
            fixed->direction_cosines
        );
        xf_out->get_grid_spacing (grid_spacing);
        xform_to_gpuit_bsp (xf_out.get(), xf_out.get(), &pih, grid_spacing);
    }
}

void
Registration::load_global_inputs ()
{
    d_ptr->rdata->load_global_input_files (d_ptr->rparms);
}

void
Registration::run_main_thread ()
{
    Registration_data::Pointer regd = d_ptr->rdata;
    Registration_parms::Pointer regp = d_ptr->rparms;
    
    /* Load initial guess of xform */
    if (regp->xf_in_fn[0]) {
        d_ptr->xf_out = xform_load (regp->xf_in_fn);
    }

    /* Set fixed image region */
    set_fixed_image_region_global (regd);

    /* Set automatic parameters based on image size */
    set_automatic_parameters (regd, regp);

    std::list<Stage_parms*>& stages = regp->get_stages();
    std::list<Stage_parms*>::iterator it;
    for (it = stages.begin(); it != stages.end(); it++) {
        Stage_parms* stage = *it;
        const Shared_parms *shared = stage->get_shared_parms();
        if (stage->get_stage_type() == STAGE_TYPE_PROCESS) {

#if defined (commentout)
            int non_zero, num_vox;
            double min_val, max_val, avg;
            itk_image_stats (regd->moving_image->itk_float (),
                &min_val, &max_val, &avg, &non_zero, &num_vox);
            printf ("min = %g, max = %g\n", min_val, max_val);
#endif

            const Process_parms::Pointer& pp = stage->get_process_parms ();
            pp->execute_process (regd);

#if defined (commentout)
            itk_image_stats (regd->moving_image->itk_float (),
                &min_val, &max_val, &avg, &non_zero, &num_vox);
            printf ("min = %g, max = %g\n", min_val, max_val);
#endif

        } else if (stage->get_stage_type() == STAGE_TYPE_REGISTER) {

            /* Check if parent wants us to pause */
            d_ptr->master_slave.slave_grab_resource ();

            /* Load stage images */
            regd->load_stage_input_files (stage);

            /* Run registation, results are stored in xf_out */
            printf ("Doing registration stage\n");

            for (int substage = 0; substage < stage->num_substages; 
                 substage++)
            {
                /* Swap xf_in and xf_out.  Memory for previous xf_in 
                   gets released at this time. */
                d_ptr->xf_in = d_ptr->xf_out;

                /* Do the registration */
                d_ptr->xf_out = this->do_registration_stage (stage);
            }

            /* Save intermediate output */
            save_output (regd.get(), d_ptr->xf_out, 
                stage->xf_out_fn, stage->xf_out_itk, 
                stage->img_out_fmt, stage->img_out_type, 
                stage->default_value, stage->img_out_fn, 
                stage->vf_out_fn, shared->warped_landmarks_fn);

            /* Tell the parent thread that we finished a stage, 
               so it can wake up if needed. */
            d_ptr->master_slave.slave_release_resource ();

            if (d_ptr->time_to_quit) {
                break;
            }
        }
    }

    /* JAS 2012.03.29 - for GPUIT Bspline
     * make output match input resolution - not final stage resolution */
    check_output_resolution (d_ptr->xf_out, regd);

    /* Done. */
    d_ptr->worker_running.release ();
}

static 
ITK_THREAD_RETURN_TYPE
registration_main_thread (void* param)
{
    itk::MultiThreader::ThreadInfoStruct *info 
        = (itk::MultiThreader::ThreadInfoStruct*) param;
    Registration* reg = (Registration*) info->UserData;

    printf ("Inside registration worker thread\n");
    reg->run_main_thread ();
    printf ("** Registration worker thread finished.\n");

    return ITK_THREAD_RETURN_VALUE;
}

void 
Registration::start_registration ()
{
    /* Otherwise, start a new job */
    d_ptr->time_to_quit = false;
    printf ("Launching registration worker thread\n");
    d_ptr->worker_running.grab ();
    d_ptr->thread_no = d_ptr->threader->SpawnThread (
        registration_main_thread, (void*) this);
}

void 
Registration::pause_registration ()
{
    d_ptr->master_slave.master_grab_resource ();
}

void 
Registration::resume_registration ()
{
    d_ptr->master_slave.master_release_resource ();
}

void 
Registration::wait_for_complete ()
{
    d_ptr->worker_running.grab ();
    d_ptr->worker_running.release ();
}

Xform::Pointer 
Registration::get_current_xform ()
{
    return d_ptr->xf_out;
}

void 
Registration::save_global_outputs ()
{
    Registration_data::Pointer regd = d_ptr->rdata;
    Registration_parms::Pointer regp = d_ptr->rparms;
    const Shared_parms* shared = regp->get_shared_parms ();

    save_output (regd.get(), d_ptr->xf_out, regp->xf_out_fn, regp->xf_out_itk, 
        regp->img_out_fmt, regp->img_out_type, 
        regp->default_value, regp->img_out_fn, 
        regp->vf_out_fn, shared->warped_landmarks_fn.c_str());
}

Xform::Pointer
Registration::do_registration_pure ()
{
    this->start_registration ();
    this->wait_for_complete ();
    return this->get_current_xform ();
}

void
Registration::do_registration ()
{
    Registration_data::Pointer regd = d_ptr->rdata;
    Registration_parms::Pointer regp = d_ptr->rparms;
    Xform::Pointer xf_out = Xform::New ();
    Plm_timer timer1, timer2, timer3;

    /* Start logging */
    logfile_open (regp->log_fn.c_str());

    timer1.start();
    this->load_global_inputs ();
    timer1.stop();
    
    timer2.start();
    this->start_registration ();
    this->wait_for_complete ();
    xf_out = this->get_current_xform ();
    timer2.stop();

    /* RMK: If no stages, we still generate output (same as input) */
    
    timer3.start();
    this->save_global_outputs ();
    timer3.stop();
    
    logfile_open (regp->log_fn.c_str());
    logfile_printf (
        "Load:   %g\n"
        "Run:    %g\n"
        "Save:   %g\n"
        "Total:  %g\n",
        (double) timer1.report(),
        (double) timer2.report(),
        (double) timer3.report(),
        (double) timer1.report() + 
        (double) timer2.report() + 
        (double) timer3.report());
    
    /* Done logging */
    logfile_printf ("Finished!\n");
    logfile_close ();
}
