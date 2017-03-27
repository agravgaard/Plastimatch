/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "distance_map.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_type.h"
#include "print_and_exit.h"
#include "registration_data.h"
#include "registration_parms.h"
#include "registration_resample.h"
#include "shared_parms.h"
#include "stage_parms.h"
#include "volume_grad.h"

class Registration_data_private
{
public:
    Stage_parms auto_parms;
    std::map <std::string, Registration_similarity_data::Pointer>
        similarity_images;
    std::list<std::string> similarity_indices;
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
    std::map<std::string,Metric_parms>::const_iterator metric_it;
    for (metric_it = shared->metric.begin();
         metric_it != shared->metric.end(); ++metric_it)
    {
        const std::string& index = metric_it->first;
        const Metric_parms& mp = metric_it->second;

        /* Sanity check -- there should be at least a fixed and moving */
        if (mp.fixed_fn == "") {
            continue;
        }
        if (mp.moving_fn == "") {
            continue;
        }
        
        /* Load images */
        logfile_printf ("Loading fixed image [%s]: %s\n", 
            index.c_str(), mp.fixed_fn.c_str());
        this->set_fixed_image (index,
            Plm_image::New (mp.fixed_fn, PLM_IMG_TYPE_ITK_FLOAT));
        logfile_printf ("Loading moving image [%s]: %s\n", 
            index.c_str(), mp.moving_fn.c_str());
        this->set_moving_image (index,
            Plm_image::New (mp.moving_fn, PLM_IMG_TYPE_ITK_FLOAT));
        
        /* Load rois */
        if (mp.fixed_roi_fn != "") {
            logfile_printf ("Loading fixed roi [%s]: %s\n", 
                index.c_str(), mp.fixed_roi_fn.c_str());
            this->set_fixed_roi (index,
                Plm_image::New (mp.fixed_roi_fn, PLM_IMG_TYPE_ITK_UCHAR));
        }
        if (mp.moving_roi_fn != "") {
            logfile_printf ("Loading moving roi [%s]: %s\n", 
                index.c_str(), mp.moving_roi_fn.c_str());
            this->set_moving_roi (index,
                Plm_image::New (mp.moving_roi_fn, PLM_IMG_TYPE_ITK_UCHAR));
        }
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

Registration_similarity_data::Pointer&
Registration_data::get_similarity_images (
    std::string index)
{
    if (index == "") {
        index = DEFAULT_IMAGE_KEY;
    }
    if (!d_ptr->similarity_images[index]) {
        d_ptr->similarity_images[index] = Registration_similarity_data::New();
    }
    return d_ptr->similarity_images[index];
}

void
Registration_data::set_fixed_image (const Plm_image::Pointer& image)
{
    this->set_fixed_image (DEFAULT_IMAGE_KEY, image);
}

void
Registration_data::set_fixed_image (
    const std::string& index,
    const Plm_image::Pointer& image)
{
    this->get_similarity_images(index)->fixed = image;
}

void
Registration_data::set_moving_image (const Plm_image::Pointer& image)
{
    this->set_moving_image (DEFAULT_IMAGE_KEY, image);
}

void
Registration_data::set_moving_image (
    const std::string& index,
    const Plm_image::Pointer& image)
{
    this->get_similarity_images(index)->moving = image;
}

void
Registration_data::set_fixed_roi (const Plm_image::Pointer& image)
{
    this->set_fixed_roi (DEFAULT_IMAGE_KEY, image);
}

void
Registration_data::set_fixed_roi (
    const std::string& index,
    const Plm_image::Pointer& image)
{
    this->get_similarity_images(index)->fixed_roi = image;
}

void
Registration_data::set_moving_roi (const Plm_image::Pointer& image)
{
    this->set_moving_roi (DEFAULT_IMAGE_KEY, image);
}

void
Registration_data::set_moving_roi (
    const std::string& index,
    const Plm_image::Pointer& image)
{
    this->get_similarity_images(index)->moving_roi = image;
}

Plm_image::Pointer&
Registration_data::get_fixed_image ()
{
    return this->get_fixed_image(DEFAULT_IMAGE_KEY);
}

Plm_image::Pointer&
Registration_data::get_fixed_image (
    const std::string& index)
{
    return this->get_similarity_images(index)->fixed;
}

Plm_image::Pointer&
Registration_data::get_moving_image ()
{
    return this->get_moving_image(DEFAULT_IMAGE_KEY);
}

Plm_image::Pointer&
Registration_data::get_moving_image (
    const std::string& index)
{
    return this->get_similarity_images(index)->moving;
}

Plm_image::Pointer&
Registration_data::get_fixed_roi ()
{
    return this->get_fixed_roi(DEFAULT_IMAGE_KEY);
}

Plm_image::Pointer&
Registration_data::get_fixed_roi (
    const std::string& index)
{
    return this->get_similarity_images(index)->fixed_roi;
}

Plm_image::Pointer&
Registration_data::get_moving_roi ()
{
    return this->get_moving_roi(DEFAULT_IMAGE_KEY);
}

Plm_image::Pointer&
Registration_data::get_moving_roi (
    const std::string& index)
{
    return this->get_similarity_images(index)->moving_roi;
}

const std::list<std::string>&
Registration_data::get_similarity_indices ()
{
    d_ptr->similarity_indices.clear ();
    
    std::map<std::string,
        Registration_similarity_data::Pointer>::const_iterator it;
    for (it = d_ptr->similarity_images.begin();
         it != d_ptr->similarity_images.end(); ++it)
    {
        if (it->second->fixed && it->second->moving) {
            if (it->first == DEFAULT_IMAGE_KEY) {
                d_ptr->similarity_indices.push_front (it->first);
            } else {
                d_ptr->similarity_indices.push_back (it->first);
            }
        }
    }
    return d_ptr->similarity_indices;
}

Stage_parms*
Registration_data::get_auto_parms ()
{
    return &d_ptr->auto_parms;
}

static Volume::Pointer
make_dmap (const Volume::Pointer& image)
{
    Plm_image::Pointer pi = Plm_image::New (image);
    Distance_map dm;

    dm.set_input_image (pi);
    dm.run ();

    Plm_image im_out (dm.get_output_image());
    return im_out.get_volume_float ();
}

void populate_similarity_list (
    std::list<Metric_state::Pointer>& similarity_data,
    Registration_data *regd,
    const Stage_parms *stage
)
{
    const Shared_parms *shared = stage->get_shared_parms();

    /* Clear out the list */
    similarity_data.clear ();

    const std::list<std::string>& similarity_indices
        = regd->get_similarity_indices ();
    std::list<std::string>::const_iterator ind_it;
    for (ind_it = similarity_indices.begin();
         ind_it != similarity_indices.end(); ++ind_it)
    {
        Plm_image::Pointer fixed_image = regd->get_fixed_image (*ind_it);
        Plm_image::Pointer moving_image = regd->get_moving_image (*ind_it);
        Volume::Pointer& fixed = fixed_image->get_volume_float ();
        Volume::Pointer& moving = moving_image->get_volume_float ();

        Metric_state::Pointer ssi = Metric_state::New();

        /* Subsample images */
        ssi->fixed_ss = registration_resample_volume (
            fixed, stage, stage->resample_rate_fixed);
        ssi->moving_ss = registration_resample_volume (
            moving, stage, stage->resample_rate_moving);

        /* Metric */
        const Metric_parms& metric_parms = shared->metric.find(*ind_it)->second;
        ssi->metric_type = metric_parms.metric_type;
        if (ssi->metric_type == SIMILARITY_METRIC_MI_VW) {
            ssi->metric_type = SIMILARITY_METRIC_MI_MATTES;
        }
        ssi->metric_lambda = metric_parms.metric_lambda;

        /* Gradient magnitude is MSE on gradient image */
        if (ssi->metric_type == SIMILARITY_METRIC_GM) {
            ssi->fixed_ss = volume_gradient_magnitude (ssi->fixed_ss);
            ssi->moving_ss = volume_gradient_magnitude (ssi->moving_ss);
        }

        /* Distance map is MSE on distance map images */
        if (ssi->metric_type == SIMILARITY_METRIC_DMAP) {
            ssi->fixed_ss = make_dmap (ssi->fixed_ss);
            ssi->moving_ss = make_dmap (ssi->moving_ss);
        }

        /* Make spatial gradient image */
        ssi->moving_grad = volume_gradient (ssi->moving_ss);

        /* Append to list */
        similarity_data.push_back (ssi);
    }
}
