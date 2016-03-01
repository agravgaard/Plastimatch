/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "logfile.h"
#include "plm_image.h"
#include "plm_image_type.h"
#include "print_and_exit.h"
#include "registration_data.h"
#include "registration_parms.h"
#include "shared_parms.h"
#include "stage_parms.h"

class Registration_data_private
{
public:
    Stage_parms auto_parms;
public:
    Registration_data_private () {
    }
    ~Registration_data_private () {
    }
};

Registration_data::Registration_data ()
{
    fixed_landmarks = 0;
    moving_landmarks = 0;
    d_ptr = new Registration_data_private;
}

Registration_data::~Registration_data ()
{
    if (fixed_landmarks) delete fixed_landmarks;
    if (moving_landmarks) delete moving_landmarks;
    delete d_ptr;
}

void
Registration_data::load_global_input_files (Registration_parms::Pointer& regp)
{
    Plm_image_type image_type = PLM_IMG_TYPE_ITK_FLOAT;

    /* Load images */
    logfile_printf ("Loading fixed image: %s\n", 
        regp->get_fixed_fn().c_str());
    this->fixed_image = Plm_image::New (new Plm_image (
            regp->get_fixed_fn(), image_type));

    logfile_printf ("Loading moving image: %s\n", 
        regp->get_moving_fn().c_str());
    this->moving_image = Plm_image::New (new Plm_image (
            regp->get_moving_fn(), image_type));

    this->load_shared_input_files (regp->get_shared_parms());
}

void
Registration_data::load_stage_input_files (const Stage_parms* stage)
{
    this->load_shared_input_files (stage->get_shared_parms());
}

void
Registration_data::load_shared_input_files (const Shared_parms* shared)
{
    /* load "global" rois */
    if (shared->fixed_roi_fn != "") {
        logfile_printf ("Loading fixed roi: %s\n", 
            shared->fixed_roi_fn.c_str());
        this->fixed_roi = Plm_image::New (
            shared->fixed_roi_fn, PLM_IMG_TYPE_ITK_UCHAR);
    }
    if (shared->moving_roi_fn != "") {
        logfile_printf ("Loading moving roi: %s\n", 
            shared->moving_roi_fn.c_str());
        this->moving_roi = Plm_image::New (
            shared->moving_roi_fn, PLM_IMG_TYPE_ITK_UCHAR);
    }

    /* load stiffness */
    if (shared->fixed_stiffness_fn != "") {
        logfile_printf ("Loading fixed stiffness: %s\n", 
            shared->fixed_stiffness_fn.c_str());
        this->fixed_stiffness = Plm_image::New (
            shared->fixed_stiffness_fn, PLM_IMG_TYPE_ITK_FLOAT);
    }

    /* load landmarks */
    if (shared->fixed_landmarks_fn != "") {
        if (shared->moving_landmarks_fn != "") {
            logfile_printf ("Loading fixed landmarks: %s\n", 
                shared->fixed_landmarks_fn.c_str());
            fixed_landmarks = new Labeled_pointset;
            fixed_landmarks->load_fcsv (
                shared->fixed_landmarks_fn.c_str());
            logfile_printf ("Loading moving landmarks: %s\n", 
                shared->moving_landmarks_fn.c_str());
            moving_landmarks = new Labeled_pointset;
            moving_landmarks->load_fcsv (
                shared->moving_landmarks_fn.c_str());
        } else {
            print_and_exit (
                "Sorry, you need to specify both fixed and moving landmarks");
        }
    }
    else if (shared->moving_landmarks_fn != "") {
        print_and_exit (
            "Sorry, you need to specify both fixed and moving landmarks");
    }
    else if (shared->fixed_landmarks_list != ""
        && shared->moving_landmarks_list != "")
    {
        fixed_landmarks = new Labeled_pointset;
        moving_landmarks = new Labeled_pointset;
        fixed_landmarks->insert_ras (shared->fixed_landmarks_list.c_str());
        moving_landmarks->insert_ras (shared->moving_landmarks_list.c_str());
    }
}

Stage_parms*
Registration_data::get_auto_parms ()
{
    return &d_ptr->auto_parms;
}
