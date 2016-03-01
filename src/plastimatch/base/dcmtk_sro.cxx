/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_module_general_series.h"
#include "dcmtk_module_general_study.h"
#include "dcmtk_module_patient.h"
#include "dcmtk_sro.h"
#include "dicom_util.h"
#include "file_util.h"
#include "logfile.h"
#include "plm_uid_prefix.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "xform.h"

void
Dcmtk_sro::save (
    const Xform::Pointer& xf,
    const Rt_study_metadata::Pointer& rsm_src,   /* Fixed image */
    const Rt_study_metadata::Pointer& rsm_reg,   /* Moving image */
    const std::string& dicom_dir,
    bool filenames_with_uid)
{
    /* Prepare xform */
    Xform xf_aff;
    xform_to_aff (&xf_aff, xf.get(), 0);
    AffineTransformType::Pointer itk_aff = xf_aff.get_aff();

    /* Prepare dcmtk */
    OFCondition ofc;
    DcmFileFormat fileformat;
    DcmDataset *dataset = fileformat.getDataset();

    Rt_study_metadata::Pointer rsm;
    Metadata::Pointer study_meta;
    if (!rsm_src || !rsm_reg) {
        print_and_exit ("Sorry, anonymous spatial registration objects "
            "are not yet supported.\n");
    }

    /* Not sure about this... */
    rsm = rsm_src;
    study_meta = rsm_src->get_study_metadata ();

    /* Patient module, general study module */
    Dcmtk_module_patient::set (dataset, study_meta);
    Dcmtk_module_general_study::set (dataset, rsm);

    /* General series module */
    Dcmtk_module_general_series::set_sro (dataset, rsm);

    /* Spatial registration specific items */
    std::string sro_sop_instance_uid = dicom_uid(PLM_UID_PREFIX);
    dataset->putAndInsertString (DCM_Modality, "REG");
    dataset->putAndInsertString (DCM_SOPClassUID, 
        UID_SpatialRegistrationStorage);
    dataset->putAndInsertString (DCM_SOPInstanceUID, 
        sro_sop_instance_uid.c_str());

    /* Content time is again tricky.  Which reference image should be used?
       The below is correct for xvi_archive program, but could be made 
       more general. */
    dataset->putAndInsertOFStringArray (DCM_ContentDate, 
        rsm_reg->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_ContentTime, 
        rsm_reg->get_study_time());

    /* ReferencedSeriesSequence */
    DcmItem *rss_item = 0;
    DcmItem *ris_item = 0;
    /* fixed */
    dataset->findOrCreateSequenceItem (
        DCM_ReferencedSeriesSequence, rss_item, -2);
    rss_item->findOrCreateSequenceItem (
        DCM_ReferencedInstanceSequence, ris_item, -2);
    ris_item->putAndInsertString (DCM_ReferencedSOPClassUID,
        UID_CTImageStorage);
    ris_item->putAndInsertString (DCM_ReferencedSOPInstanceUID,
        rsm_reg->get_slice_uid (0));
    rss_item->putAndInsertString (DCM_SeriesInstanceUID,
        rsm_reg->get_ct_series_uid ());
    /* moving */
    dataset->findOrCreateSequenceItem (
        DCM_ReferencedSeriesSequence, rss_item, -2);
    rss_item->findOrCreateSequenceItem (
        DCM_ReferencedInstanceSequence, ris_item, -2);
    ris_item->putAndInsertString (DCM_ReferencedSOPClassUID,
        UID_CTImageStorage);
    ris_item->putAndInsertString (DCM_ReferencedSOPInstanceUID,
        rsm_src->get_slice_uid (0));
    rss_item->putAndInsertString (DCM_SeriesInstanceUID,
        rsm_src->get_ct_series_uid ());

    /* FrameOfReferenceUID -- of fixed image */
    dataset->putAndInsertString (
        DCM_FrameOfReferenceUID, 
        rsm_reg->get_frame_of_reference_uid());

    /* Spatial registration module -- fixed image */
    DcmItem *reg_item = 0;
    dataset->findOrCreateSequenceItem (
        DCM_RegistrationSequence, reg_item, -2);
    reg_item->putAndInsertString (
        DCM_FrameOfReferenceUID, 
        rsm_reg->get_frame_of_reference_uid());
    DcmItem *mr_item = 0;
    reg_item->findOrCreateSequenceItem (
        DCM_MatrixRegistrationSequence, mr_item, -2);
    DcmItem *rtc_item = 0;
    mr_item->findOrCreateSequenceItem (
        DCM_RegistrationTypeCodeSequence, rtc_item, -2);
    rtc_item->putAndInsertString (DCM_CodeValue, "125025");
    rtc_item->putAndInsertString (DCM_CodingSchemeDesignator, "DCM");
    rtc_item->putAndInsertString (DCM_CodeMeaning, "Visual Alignment");
    DcmItem *m_item = 0;
    mr_item->findOrCreateSequenceItem (DCM_MatrixSequence, m_item, -2);
    m_item->putAndInsertString (DCM_FrameOfReferenceTransformationMatrix,
        "1.0\\0.0\\0.0\\0.0\\0.0\\1.0\\0.0\\0.0\\"
        "0.0\\0.0\\1.0\\0.0\\0.0\\0.0\\0.0\\1.0");
    m_item->putAndInsertString (DCM_FrameOfReferenceTransformationMatrixType,
        "RIGID");

    /* Spatial registration module -- moving image */
    dataset->findOrCreateSequenceItem (
        DCM_RegistrationSequence, reg_item, -2);
    reg_item->putAndInsertString (
        DCM_FrameOfReferenceUID, 
        rsm_src->get_frame_of_reference_uid());
    reg_item->findOrCreateSequenceItem (
        DCM_MatrixRegistrationSequence, mr_item, -2);
    mr_item->findOrCreateSequenceItem (
        DCM_RegistrationTypeCodeSequence, rtc_item, -2);
    rtc_item->putAndInsertString (DCM_CodeValue, "125025");
    rtc_item->putAndInsertString (DCM_CodingSchemeDesignator, "DCM");
    rtc_item->putAndInsertString (DCM_CodeMeaning, "Visual Alignment");
    mr_item->findOrCreateSequenceItem (DCM_MatrixSequence, m_item, -2);
    std::string matrix_string;
    const AffineTransformType::MatrixType& itk_aff_mat 
        = itk_aff->GetMatrix ();
    const AffineTransformType::OutputVectorType& itk_aff_off 
        = itk_aff->GetOffset ();
    matrix_string = string_format (
        "%f\\%f\\%f\\%f\\"
        "%f\\%f\\%f\\%f\\"
        "%f\\%f\\%f\\%f\\"
        "0.0\\0.0\\0.0\\1.0",
        itk_aff_mat[0][0],
        itk_aff_mat[0][1],
        itk_aff_mat[0][2],
        itk_aff_off[0],
        itk_aff_mat[1][0],
        itk_aff_mat[1][1],
        itk_aff_mat[1][2],
        itk_aff_off[1],
        itk_aff_mat[2][0],
        itk_aff_mat[2][1],
        itk_aff_mat[2][2],
        itk_aff_off[2]);
    m_item->putAndInsertString (DCM_FrameOfReferenceTransformationMatrix,
        matrix_string.c_str());
    m_item->putAndInsertString (DCM_FrameOfReferenceTransformationMatrixType,
        "RIGID");

    /* Prepare output file */
    std::string sro_fn;
    if (filenames_with_uid) {
        sro_fn = string_format ("%s/sro_%s.dcm", dicom_dir.c_str(),
            sro_sop_instance_uid.c_str());
    } else {
        sro_fn = string_format ("%s/sro.dcm", dicom_dir.c_str());
    }
    make_parent_directories (sro_fn);

    /* ----------------------------------------------------------------- *
     *  Write the output file
     * ----------------------------------------------------------------- */
    lprintf ("Trying to save SRO: %s\n", sro_fn.c_str());
    ofc = fileformat.saveFile (sro_fn.c_str(), EXS_LittleEndianExplicit);
    if (ofc.bad()) {
        print_and_exit (
            "Error: cannot write DICOM Spatial Registration (%s) (%s)\n", 
            sro_fn.c_str(),
            ofc.text());
    }
}
