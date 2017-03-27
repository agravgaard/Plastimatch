/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_module.h"
#include "dcmtk_metadata.h"
#include "dicom_util.h"
#include "metadata.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"

void
Dcmtk_module::set_patient (
    DcmDataset *dataset, const Metadata::Pointer& meta)
{
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientName, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientID, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientBirthDate, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientSex, "O");
}

void
Dcmtk_module::set_general_study (
    DcmDataset *dataset, 
    const Rt_study_metadata::Pointer& rsm)
{
    dataset->putAndInsertString (DCM_StudyInstanceUID, 
        rsm->get_study_uid());
    dataset->putAndInsertOFStringArray (DCM_StudyDate, 
        rsm->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_StudyTime, 
        rsm->get_study_time());
    dataset->putAndInsertString (DCM_ReferringPhysicianName, 
        rsm->get_referring_physician_name());
    dcmtk_copy_from_metadata (dataset, rsm->get_study_metadata(), 
        DCM_StudyID, "");
    dataset->putAndInsertString (DCM_AccessionNumber,rsm->get_accession_number() );
    dataset->putAndInsertString (DCM_StudyDescription,rsm->get_study_description() );
    // dcmtk_copy_from_metadata (dataset, rsm->get_study_metadata (),
    //     DCM_StudyDescription, "");
    dataset->putAndInsertOFStringArray (DCM_StudyID, 
        rsm->get_study_id());
 }

void
Dcmtk_module::set_general_series (
    DcmDataset *dataset, 
    const Metadata::Pointer& meta,
    const char* modality)
{
    dataset->putAndInsertOFStringArray (DCM_Modality, modality);
    dataset->putAndInsertString (DCM_SeriesInstanceUID, 
        dicom_uid(PLM_UID_PREFIX).c_str());
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesNumber, 0);
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesDate, 0);
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesTime, 0);
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesDescription, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_OperatorsName, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientPosition, "HFS");
}

void
Dcmtk_module::set_frame_of_reference (
    DcmDataset *dataset, 
    const Rt_study_metadata::Pointer& rsm)
{
    dataset->putAndInsertString (DCM_FrameOfReferenceUID, 
        rsm->get_frame_of_reference_uid());
}

void
Dcmtk_module::set_general_equipment (DcmDataset *dataset,  
    const Metadata::Pointer& meta)
{
    dcmtk_copy_from_metadata (dataset, meta, DCM_Manufacturer, "Plastimatch");
    //dcmtk_copy_from_metadata (dataset, meta, DCM_InstitutionName, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_StationName, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_ManufacturerModelName,
        "Plastimatch");
    dcmtk_copy_from_metadata (dataset, meta, DCM_DeviceSerialNumber, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_SoftwareVersions,
        PLASTIMATCH_VERSION_STRING);
}

void
Dcmtk_module::set_rt_series (
    DcmDataset *dataset,
    const Metadata::Pointer& meta,
    const char* modality)
{
    dataset->putAndInsertOFStringArray (DCM_Modality, modality);
    /* Series Instance UID, this gets copied from e.g. 
        d_ptr->rt_study_metadata->get_dose_series_uid(), 
        in order to correctly make cross references between series.
        It is safe to set here, and allow caller to override. */
    dataset->putAndInsertString (DCM_SeriesInstanceUID, 
        dicom_uid(PLM_UID_PREFIX).c_str());
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesNumber, 0);
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesDate, 0);
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesTime, 0);
    dcmtk_copy_from_metadata (dataset, meta, DCM_SeriesDescription, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_OperatorsName, "");
}

