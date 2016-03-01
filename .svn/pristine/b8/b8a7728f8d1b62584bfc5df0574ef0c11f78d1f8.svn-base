/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <string>

#if PLM_DCM_USE_DCMTK
#include "dcmtk_sro.h"
#endif
#include "dicom_sro_save.h"
#include "plm_image.h"
#include "rt_study.h"
#include "rt_study_metadata.h"
#include "xform.h"

class Dicom_sro_save_private {
public:
    Dicom_sro_save_private () {
        output_dir = "sro_export";
    }
public:
    std::string fixed_image_path;
    std::string moving_image_path;
    Plm_image::Pointer fixed_image;
    Plm_image::Pointer moving_image;
    Xform::Pointer xform;
    std::string output_dir;

public:
Rt_study_metadata::Pointer load_rt_study (
    Plm_image::Pointer& image, const std::string& path, 
    const std::string& output_suffix);
};

/* Utility function */
Rt_study_metadata::Pointer 
Dicom_sro_save_private::load_rt_study (
    Plm_image::Pointer& image, const std::string& path,
    const std::string& output_suffix)
{
    if (image) {
        Rt_study::Pointer rtds = Rt_study::New ();
        rtds->set_image (image);
        std::string fixed_path = this->output_dir + "/" + output_suffix;
        rtds->save_dicom (fixed_path);
        return rtds->get_rt_study_metadata();
    }
    if (path != "") {
        Plm_file_format format = plm_file_format_deduce (path);
        if (format == PLM_FILE_FMT_DICOM_DIR) {
            return Rt_study_metadata::load (path);
        }
        Plm_image::Pointer new_image = Plm_image::New ();
        new_image->load_native (path);
        return this->load_rt_study (new_image, path, output_suffix);
    }
    /* Return null pointer */
    return Rt_study_metadata::Pointer();
}

Dicom_sro_save::Dicom_sro_save ()
{
    d_ptr = new Dicom_sro_save_private;
}

Dicom_sro_save::~Dicom_sro_save ()
{
    delete d_ptr;
}

void
Dicom_sro_save::set_fixed_image (const char* path)
{
    d_ptr->fixed_image_path = path;
}

void
Dicom_sro_save::set_fixed_image (const Plm_image::Pointer& fixed_image)
{
    d_ptr->fixed_image = fixed_image;
}

void
Dicom_sro_save::set_moving_image (const char* path)
{
    d_ptr->moving_image_path = path;
}

void
Dicom_sro_save::set_moving_image (const Plm_image::Pointer& moving_image)
{
    d_ptr->moving_image = moving_image;
}

void
Dicom_sro_save::set_xform (const Xform::Pointer& xform)
{
    d_ptr->xform = xform;
}

void
Dicom_sro_save::set_output_dir (const std::string& output_dir)
{
    d_ptr->output_dir = output_dir;
}

void
Dicom_sro_save::run ()
{
    /* Load referenced image sets */
#if PLM_DCM_USE_DCMTK
    Rt_study_metadata::Pointer rtm_reg;
    Rt_study_metadata::Pointer rtm_src;

    /* Fixed image */
    rtm_reg = d_ptr->load_rt_study (
        d_ptr->fixed_image, d_ptr->fixed_image_path, "fixed");

    /* Moving image */
    rtm_src = d_ptr->load_rt_study (
        d_ptr->moving_image, d_ptr->moving_image_path, "moving");

    Dcmtk_sro::save (
        d_ptr->xform, rtm_src, rtm_reg, d_ptr->output_dir, true);

#if defined (commentout)
    /* Fixed image */
    if (!parms->fixed_image.empty()) {
        lprintf ("Loading fixed...\n");
        Rt_study::Pointer rtds = Rt_study::New ();
        rtds->load_image (parms->fixed_image);
        std::string fixed_path = parms->output_dicom_dir + "/fixed";
        rtds->save_dicom (fixed_path);
        rtm_reg = rtds->get_rt_study_metadata();
    }
    else if (!parms->registered_rcs.empty()) {
        lprintf ("Loading registered...\n");
        rtm_reg = Rt_study_metadata::load (parms->registered_rcs);
    }

    /* Moving image */
    if (!parms->moving_image.empty()) {
        lprintf ("Loading moving...\n");
        Rt_study::Pointer rtds = Rt_study::New ();
        rtds->load_image (parms->fixed_image);
        std::string moving_path = parms->output_dicom_dir + "/moving";
        rtds->save_dicom (moving_path);
        rtm_src = rtds->get_rt_study_metadata();
    }
    else if (!parms->source_rcs.empty()) {
        lprintf ("Loading source...\n");
        rtm_src = Rt_study_metadata::load (parms->source_rcs);
    }

    Dcmtk_sro::save (
        xfc->m_xf_out, rtm_src, rtm_reg, parms->output_dicom_dir);
#endif
#endif
}
