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
    std::string description_string;
    std::string referring_physician_name_string;
    std::string accession_number_string;
    std::string time_string;
    std::string study_id_string;

    std::string study_uid;
    std::string for_uid;

    std::string position_reference_indicator_string;

    std::string ct_series_uid;
    std::string dose_instance_uid;
    std::string dose_series_uid;
    std::string plan_instance_uid;
    std::string rtstruct_instance_uid;
    std::string rtstruct_series_uid;
    Slice_list slice_list;

    Metadata::Pointer study_metadata;
    Metadata::Pointer image_metadata;
    Metadata::Pointer rtstruct_metadata;
    Metadata::Pointer dose_metadata;
    Metadata::Pointer rtplan_metadata;
    Metadata::Pointer sro_metadata;

public:
    Rt_study_metadata_private () {
        dicom_get_date_time (&date_string, &time_string);

        study_metadata = Metadata::New ();
        image_metadata = Metadata::New ();
        rtstruct_metadata = Metadata::New ();
        dose_metadata = Metadata::New ();
        rtplan_metadata = Metadata::New ();
        sro_metadata = Metadata::New ();

        study_metadata->create_anonymous ();
        image_metadata->set_parent (study_metadata);
        rtstruct_metadata->set_parent (study_metadata);
        dose_metadata->set_parent (study_metadata);
        rtplan_metadata->set_parent (study_metadata);
        sro_metadata->set_parent (study_metadata);

        this->generate_new_study_uids ();
        this->generate_new_series_uids ();
    }
public:
    void
    generate_new_study_uids () {
        study_uid = dicom_uid (PLM_UID_PREFIX);
        for_uid = dicom_uid (PLM_UID_PREFIX);
    }
    void
    generate_new_series_uids () {
        ct_series_uid = dicom_uid (PLM_UID_PREFIX);
        dose_instance_uid = dicom_uid (PLM_UID_PREFIX);
        dose_series_uid = dicom_uid (PLM_UID_PREFIX);
        plan_instance_uid = dicom_uid (PLM_UID_PREFIX);
        rtstruct_instance_uid = dicom_uid (PLM_UID_PREFIX);
        rtstruct_series_uid = dicom_uid (PLM_UID_PREFIX);
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

const std::string&
Rt_study_metadata::get_dose_instance_uid () const
{
    return d_ptr->dose_instance_uid;
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

const std::string&
Rt_study_metadata::get_rtstruct_instance_uid () const
{
    return d_ptr->rtstruct_instance_uid;
}

const char*
Rt_study_metadata::get_rtstruct_series_uid () const
{
    return d_ptr->rtstruct_series_uid.c_str();
}

const char*
Rt_study_metadata::get_referring_physician_name () const
{
    return d_ptr->referring_physician_name_string.c_str();
}

void
Rt_study_metadata::set_referring_physician_name (const char* referring_physician_name)
{
    if (!referring_physician_name) return;
    d_ptr->referring_physician_name_string = referring_physician_name;
}

void
Rt_study_metadata::set_referring_physician_name (const std::string& referring_physician_name)
{
    d_ptr->referring_physician_name_string = referring_physician_name;
}

const char*
Rt_study_metadata::get_position_reference_indicator () const
{
    return d_ptr->position_reference_indicator_string.c_str();
}

void
Rt_study_metadata::set_position_reference_indicator (const char* position_reference_indicator)
{
    if (!position_reference_indicator) return;
    d_ptr->position_reference_indicator_string = position_reference_indicator;
}

void
Rt_study_metadata::set_position_reference_indicator (const std::string& position_reference_indicator)
{
    d_ptr->position_reference_indicator_string = position_reference_indicator;
}

const char*
Rt_study_metadata::get_accession_number () const
{
    return d_ptr->accession_number_string.c_str();
}

void
Rt_study_metadata::set_accession_number (const char* accession_number)
{
    if (!accession_number) return;
    d_ptr->accession_number_string = accession_number;
}

void
Rt_study_metadata::set_accession_number (const std::string& accession_number)
{
    d_ptr->accession_number_string = accession_number;
}

const char*
Rt_study_metadata::get_study_description () const
{
    return d_ptr->description_string.c_str();
}

void
Rt_study_metadata::set_study_description (const char* description)
{
    if (!description) return;
    d_ptr->description_string = description;
}

void
Rt_study_metadata::set_study_description (const std::string& description)
{
    d_ptr->description_string = description;
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

const char*
Rt_study_metadata::get_study_id () const
{
    return d_ptr->study_id_string.c_str();
}

void
Rt_study_metadata::set_study_id (const char* study_id)
{
    if (!study_id) return;
    d_ptr->study_id_string = study_id;
}

void
Rt_study_metadata::set_study_id (const std::string& study_id)
{
    d_ptr->study_id_string = study_id;
}

const std::string& 
Rt_study_metadata::get_patient_name ()
{
    return d_ptr->study_metadata->get_metadata (0x0010, 0x0010);
}

void
Rt_study_metadata::set_patient_name (const char* name)
{
    d_ptr->study_metadata->set_metadata (0x0010, 0x0010, name);

    /* GCS FIX: Should I remove from child metadata?
       Logically it seems necessary, but it is an ugly design 
       that it is needed.  Existing code does not seem to rely 
       on this, as patient name is not stored within child metadata. */
    // d_ptr->image_metadata->remove_metadata (0x0010, 0x0010);
}

void
Rt_study_metadata::set_patient_name (const std::string& name)
{
    set_patient_name (name.c_str());
}

const std::string& 
Rt_study_metadata::get_patient_id ()
{
    return d_ptr->study_metadata->get_metadata (0x0010, 0x0020);
}

void
Rt_study_metadata::set_patient_id (const std::string& id)
{
    d_ptr->study_metadata->set_metadata (0x0010, 0x0020, id.c_str());
}


const std::string& 
Rt_study_metadata::get_patient_birth_date ()
{
    return d_ptr->study_metadata->get_metadata (0x0010, 0x0030);
}

void
Rt_study_metadata::set_patient_birth_date (const char* birth_date)
{
    d_ptr->study_metadata->set_metadata (0x0010, 0x0030, birth_date);
}

void
Rt_study_metadata::set_patient_birth_date (const std::string& birth_date)
{
    set_patient_birth_date (birth_date.c_str());
}

const std::string& 
Rt_study_metadata::get_patient_sex ()
{
    return d_ptr->study_metadata->get_metadata (0x0010, 0x0040);
}

void
Rt_study_metadata::set_patient_sex (const char* sex)
{
    d_ptr->study_metadata->set_metadata (0x0010, 0x0040, sex);
}

void
Rt_study_metadata::set_patient_sex (const std::string& sex)
{
    set_patient_sex (sex.c_str());
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

const std::string& 
Rt_study_metadata::get_image_metadata (
    unsigned short key1, 
    unsigned short key2
) {
    return d_ptr->image_metadata->get_metadata (key1, key2);
}

void
Rt_study_metadata::set_image_metadata (
    unsigned short key1, 
    unsigned short key2,
    const std::string& val
) {
    d_ptr->image_metadata->set_metadata (key1, key2, val);
}

Metadata::Pointer&
Rt_study_metadata::get_rtstruct_metadata ()
{
    return d_ptr->rtstruct_metadata;
}

const Metadata::Pointer&
Rt_study_metadata::get_rtstruct_metadata () const
{
    return d_ptr->rtstruct_metadata;
}

void
Rt_study_metadata::set_rtstruct_metadata (
    unsigned short key1, 
    unsigned short key2,
    const std::string& val
) {
    d_ptr->rtstruct_metadata->set_metadata (key1, key2, val);
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
Rt_study_metadata::set_dose_metadata (
    unsigned short key1, 
    unsigned short key2,
    const std::string& val
) {
    d_ptr->dose_metadata->set_metadata (key1, key2, val);
}

Metadata::Pointer&
Rt_study_metadata::get_rtplan_metadata ()
{
    return d_ptr->rtplan_metadata;
}

const Metadata::Pointer&
Rt_study_metadata::get_rtplan_metadata () const
{
    return d_ptr->rtplan_metadata;
}

void
Rt_study_metadata::set_rtplan_metadata (
    unsigned short key1, 
    unsigned short key2,
    const std::string& val
) {
    d_ptr->rtplan_metadata->set_metadata (key1, key2, val);
}

Metadata::Pointer&
Rt_study_metadata::get_sro_metadata ()
{
    return d_ptr->sro_metadata;
}

const Metadata::Pointer&
Rt_study_metadata::get_sro_metadata () const
{
    return d_ptr->sro_metadata;
}

void
Rt_study_metadata::set_sro_metadata (
    unsigned short key1, 
    unsigned short key2,
    const std::string& val
) {
    d_ptr->sro_metadata->set_metadata (key1, key2, val);
}

#if PLM_DCM_USE_DCMTK
const std::string&
Rt_study_metadata::get_study_metadata (const DcmTagKey& key) const
{
    return d_ptr->study_metadata->get_metadata (key);
}

void Rt_study_metadata::set_study_metadata (
    const DcmTagKey& key, const std::string& val)
{
    d_ptr->study_metadata->set_metadata (key, val);
}

const std::string&
Rt_study_metadata::get_image_metadata (const DcmTagKey& key) const
{
    return d_ptr->image_metadata->get_metadata (key);
}

void Rt_study_metadata::set_image_metadata (
    const DcmTagKey& key, const std::string& val)
{
    d_ptr->image_metadata->set_metadata (key, val);
}

const std::string&
Rt_study_metadata::get_sro_metadata (const DcmTagKey& key) const
{
    return d_ptr->sro_metadata->get_metadata (key);
}

void Rt_study_metadata::set_sro_metadata (
    const DcmTagKey& key, const std::string& val)
{
    d_ptr->sro_metadata->set_metadata (key, val);
}
#endif

void
Rt_study_metadata::generate_new_study_uids () 
{
    d_ptr->generate_new_study_uids ();
}

void
Rt_study_metadata::generate_new_series_uids () 
{
    d_ptr->generate_new_series_uids ();
}
