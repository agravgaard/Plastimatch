/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "dicom_util.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "rt_study_metadata.h"
#include "slice_list.h"
#include "volume.h"

class Rt_study_metadata_private {
public:
    std::string date_string;
    std::string time_string;
    std::string ct_series_uid;
    std::string dose_instance_uid;
    std::string dose_series_uid;
    std::string for_uid;
    std::string plan_instance_uid;
    std::string rtss_instance_uid;
    std::string rtss_series_uid;
    std::string study_uid;
    Slice_list slice_list;

    Metadata::Pointer study_metadata;
    Metadata::Pointer image_metadata;
    Metadata::Pointer rtss_metadata;
    Metadata::Pointer dose_metadata;

public:
    Rt_study_metadata_private () {
        dicom_get_date_time (&date_string, &time_string);
        study_uid = dicom_uid (PLM_UID_PREFIX);
        for_uid = dicom_uid (PLM_UID_PREFIX);
        ct_series_uid = dicom_uid (PLM_UID_PREFIX);
        plan_instance_uid = dicom_uid (PLM_UID_PREFIX);
        rtss_instance_uid = dicom_uid (PLM_UID_PREFIX);
        rtss_series_uid = dicom_uid (PLM_UID_PREFIX);
        dose_instance_uid = dicom_uid (PLM_UID_PREFIX);
        dose_series_uid = dicom_uid (PLM_UID_PREFIX);

        study_metadata = Metadata::New ();
        image_metadata = Metadata::New ();
        rtss_metadata = Metadata::New ();
        dose_metadata = Metadata::New ();

        study_metadata->create_anonymous ();
        image_metadata->set_parent (study_metadata);
        rtss_metadata->set_parent (study_metadata);
        dose_metadata->set_parent (study_metadata);
    }
public:
    void
    generate_new_uids () {
        study_uid = dicom_uid (PLM_UID_PREFIX);
        for_uid = dicom_uid (PLM_UID_PREFIX);
        ct_series_uid = dicom_uid (PLM_UID_PREFIX);
        plan_instance_uid = dicom_uid (PLM_UID_PREFIX);
        rtss_instance_uid = dicom_uid (PLM_UID_PREFIX);
        rtss_series_uid = dicom_uid (PLM_UID_PREFIX);
        dose_instance_uid = dicom_uid (PLM_UID_PREFIX);
        dose_series_uid = dicom_uid (PLM_UID_PREFIX);
    }
};

Rt_study_metadata::Rt_study_metadata ()
{
    this->d_ptr = new Rt_study_metadata_private;
}

Rt_study_metadata::~Rt_study_metadata ()
{
    delete this->d_ptr;
}

Rt_study_metadata::Pointer
Rt_study_metadata::load (const char* dicom_path)
{
    Rt_study_metadata::Pointer rsm = Rt_study_metadata::New ();
    dicom_load_rdd (rsm, dicom_path);
    return rsm;
}

Rt_study_metadata::Pointer
Rt_study_metadata::load (const std::string& dicom_path)
{
    return Rt_study_metadata::load (dicom_path.c_str());
}

const char*
Rt_study_metadata::get_ct_series_uid () const
{
    return d_ptr->ct_series_uid.c_str();
}

void
Rt_study_metadata::set_ct_series_uid (const char* uid)
{
    if (!uid) return;
    d_ptr->ct_series_uid = uid;
}

const char*
Rt_study_metadata::get_dose_instance_uid () const
{
    return d_ptr->dose_instance_uid.c_str();
}

const char*
Rt_study_metadata::get_dose_series_uid () const
{
    return d_ptr->dose_series_uid.c_str();
}

const char*
Rt_study_metadata::get_frame_of_reference_uid () const
{
    return d_ptr->for_uid.c_str();
}

void
Rt_study_metadata::set_frame_of_reference_uid (const char* uid)
{
    if (!uid) return;
    d_ptr->for_uid = uid;
}

const char*
Rt_study_metadata::get_plan_instance_uid () const
{
    return d_ptr->plan_instance_uid.c_str();
}

const char*
Rt_study_metadata::get_rtss_instance_uid () const
{
    return d_ptr->rtss_instance_uid.c_str();
}

const char*
Rt_study_metadata::get_rtss_series_uid () const
{
    return d_ptr->rtss_series_uid.c_str();
}

const char*
Rt_study_metadata::get_study_date () const
{
    return d_ptr->date_string.c_str();
}

void
Rt_study_metadata::set_study_date (const char* date)
{
    if (!date) return;
    d_ptr->date_string = date;
}

void
Rt_study_metadata::set_study_date (const std::string& date)
{
    d_ptr->date_string = date;
}

const char*
Rt_study_metadata::get_study_time () const
{
    return d_ptr->time_string.c_str();
}

void
Rt_study_metadata::set_study_time (const char* time)
{
    if (!time) return;
    d_ptr->time_string = time;
}

void
Rt_study_metadata::set_study_time (const std::string& time)
{
    d_ptr->time_string = time;
}

const char*
Rt_study_metadata::get_study_uid () const
{
    return d_ptr->study_uid.c_str();
}

void
Rt_study_metadata::set_study_uid (const char* uid)
{
    if (!uid) return;
    d_ptr->study_uid = uid;
}

const std::string& 
Rt_study_metadata::get_patient_name ()
{
    return d_ptr->image_metadata->get_metadata (0x0010, 0x0010);
}

void
Rt_study_metadata::set_patient_name (const std::string& name)
{
    d_ptr->image_metadata->set_metadata (0x0010, 0x0010, name.c_str());
}

const std::string& 
Rt_study_metadata::get_patient_id ()
{
    return d_ptr->image_metadata->get_metadata (0x0010, 0x0020);
}

void
Rt_study_metadata::set_patient_id (const std::string& id)
{
    d_ptr->image_metadata->set_metadata (0x0010, 0x0020, id.c_str());
}

const Plm_image_header*
Rt_study_metadata::get_image_header () const
{
    return d_ptr->slice_list.get_image_header ();
}

void
Rt_study_metadata::set_image_header (const Plm_image::Pointer& pli)
{
    d_ptr->slice_list.set_image_header (Plm_image_header (pli.get()));
}

void
Rt_study_metadata::set_image_header (const Plm_image_header& pih)
{
    d_ptr->slice_list.set_image_header (pih);
}

void
Rt_study_metadata::set_image_header (ShortImageType::Pointer img)
{
    d_ptr->slice_list.set_image_header (img);
}

const Slice_list* 
Rt_study_metadata::get_slice_list () const
{
    return &d_ptr->slice_list;
}

void
Rt_study_metadata::reset_slice_uids ()
{
    return d_ptr->slice_list.reset_slice_uids ();
}

const char*
Rt_study_metadata::get_slice_uid (int index) const
{
    return d_ptr->slice_list.get_slice_uid (index);
}

void 
Rt_study_metadata::set_slice_uid (int index, const char* slice_uid)
{
    if (!slice_uid) return;
    d_ptr->slice_list.set_slice_uid (index, slice_uid);
}

void 
Rt_study_metadata::set_slice_list_complete ()
{
    d_ptr->slice_list.set_slice_list_complete ();
}

bool
Rt_study_metadata::slice_list_complete () const
{
    return d_ptr->slice_list.slice_list_complete ();
}

int 
Rt_study_metadata::num_slices () const
{
    return d_ptr->slice_list.num_slices ();
}

Metadata::Pointer&
Rt_study_metadata::get_study_metadata ()
{
    return d_ptr->study_metadata;
}

const Metadata::Pointer&
Rt_study_metadata::get_study_metadata () const
{
    return d_ptr->study_metadata;
}

void
Rt_study_metadata::set_study_metadata (
    unsigned short key1, 
    unsigned short key2,
    const std::string& val
) {
    d_ptr->study_metadata->set_metadata (key1, key2, val);
}

Metadata::Pointer&
Rt_study_metadata::get_image_metadata ()
{
    return d_ptr->image_metadata;
}

const Metadata::Pointer&
Rt_study_metadata::get_image_metadata () const
{
    return d_ptr->image_metadata;
}

Metadata::Pointer&
Rt_study_metadata::get_rtss_metadata ()
{
    return d_ptr->rtss_metadata;
}

const Metadata::Pointer&
Rt_study_metadata::get_rtss_metadata () const
{
    return d_ptr->rtss_metadata;
}

Metadata::Pointer&
Rt_study_metadata::get_dose_metadata ()
{
    return d_ptr->dose_metadata;
}

const Metadata::Pointer&
Rt_study_metadata::get_dose_metadata () const
{
    return d_ptr->dose_metadata;
}

void
Rt_study_metadata::generate_new_uids () 
{
    d_ptr->generate_new_uids ();
}
