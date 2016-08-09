/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_file.h"
#include "dcmtk_loader.h"
#include "dcmtk_loader_p.h"
#include "dcmtk_metadata.h"
#include "dcmtk_rt_study.h"
#include "dcmtk_rt_study_p.h"
#include "dcmtk_rtdose.h"
#include "dcmtk_series.h"
#include "file_util.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_math.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"
#include "volume_stats.h"

bool 
dcmtk_dose_probe (const char *fn)
{
    DcmFileFormat dfile;

    /* Suppress warning messages */
    OFLog::configure(OFLogger::FATAL_LOG_LEVEL);

    OFCondition ofrc = dfile.loadFile (fn, EXS_Unknown, EGL_noChange);

    /* Restore error messages -- n.b. dcmtk doesn't have a way to 
       query current setting, so I just set to default */
    OFLog::configure(OFLogger::WARN_LOG_LEVEL);

    if (ofrc.bad()) {
        return false;
    }

    const char *c;
    DcmDataset *dset = dfile.getDataset();
    ofrc = dset->findAndGetString (DCM_Modality, c);
    if (ofrc.bad() || !c) {
        return false;
    }

    if (strncmp (c, "RTDOSE", strlen("RTDOSE"))) {
	return false;
    } else {
	return true;
    }
}


/* This is the tolerance on irregularity of the grid spacing (in mm) */
#define GFOV_SPACING_TOL (1e-1)

template <class T> 
void
dcmtk_dose_copy (float *img_out, T *img_in, int nvox, float scale)
{
    for (int i = 0; i < nvox; i++) {
	img_out[i] = img_in[i] * scale;
    }
}

void
Dcmtk_loader::rtdose_load ()
{
    int rc;
    const char *val;
    uint16_t val_u16;
    plm_long dim[3];
    float ipp[3];
    float spacing[3];
    float *gfov;    /* gfov = GridFrameOffsetVector */
    plm_long gfov_len;
    const char *gfov_str;
    OFCondition ofrc;

    /* Modality -- better be RTDOSE */
    std::string modality = d_ptr->ds_rtdose->get_modality();
    if (modality == "RTDOSE") {
        printf ("Trying to load rt dose.\n");
    } else {
        print_and_exit ("Oops.\n");
    }

    /* FIX: load metadata such as patient name, etc. */

    /* ImagePositionPatient */
    val = d_ptr->ds_rtdose->get_cstr (DCM_ImagePositionPatient);
    if (!val) {
        print_and_exit ("Couldn't find DCM_ImagePositionPatient in rtdose\n");
    }
    rc = sscanf (val, "%f\\%f\\%f", &ipp[0], &ipp[1], &ipp[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing RTDOSE ipp.\n");
    }

    /* ImageOrientationPatient */
    float direction_cosines[9] = {
        1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f };
    val = d_ptr->ds_rtdose->get_cstr (DCM_ImageOrientationPatient);
    if (val) {
	int rc = parse_dicom_float6 (direction_cosines, val);
	if (!rc) {
	    direction_cosines[6] 
		= direction_cosines[1]*direction_cosines[5] 
		- direction_cosines[2]*direction_cosines[4];
	    direction_cosines[7] 
		= direction_cosines[2]*direction_cosines[3] 
		- direction_cosines[0]*direction_cosines[5];
	    direction_cosines[8] 
		= direction_cosines[0]*direction_cosines[4] 
		- direction_cosines[1]*direction_cosines[3];
	}
    }
    
    /* Rows */
    if (!d_ptr->ds_rtdose->get_uint16 (DCM_Rows, &val_u16)) {
        print_and_exit ("Couldn't find DCM_Rows in rtdose\n");
    }
    dim[1] = val_u16;

    /* Columns */
    if (!d_ptr->ds_rtdose->get_uint16 (DCM_Columns, &val_u16)) {
        print_and_exit ("Couldn't find DCM_Columns in rtdose\n");
    }
    dim[0] = val_u16;

    /* PixelSpacing */
    val = d_ptr->ds_rtdose->get_cstr (DCM_PixelSpacing);
    if (!val) {
        print_and_exit ("Couldn't find DCM_PixelSpacing in rtdose\n");
    }
    rc = sscanf (val, "%g\\%g", &spacing[1], &spacing[0]);
    if (rc != 2) {
	print_and_exit ("Error parsing RTDOSE pixel spacing.\n");
    }

    /* GridFrameOffsetVector */
    val = d_ptr->ds_rtdose->get_cstr (DCM_GridFrameOffsetVector);
    if (!val) {
        print_and_exit ("Couldn't find DCM_GridFrameOffsetVector in rtdose\n");
    }
    gfov = 0;
    gfov_len = 0;
    gfov_str = val;
    while (1) {
	int len;
	gfov = (float*) realloc (gfov, (gfov_len + 1) * sizeof(float));
	rc = sscanf (gfov_str, "%g%n", &gfov[gfov_len], &len);
	if (rc != 1) {
	    break;
	}
	gfov_len ++;
	gfov_str += len;
	if (gfov_str[0] == '\\') {
	    gfov_str ++;
	}
    }
    dim[2] = gfov_len;
    if (gfov_len == 0) {
	print_and_exit ("Error parsing RTDOSE gfov.\n");
    }

    /* --- Analyze GridFrameOffsetVector --- */

    /* (1) Make sure first element is 0. */
    if (gfov[0] != 0.) {
	if (gfov[0] == ipp[2]) {
	    /* In this case, gfov values are absolute rather than relative 
	       positions, but we process the same way. */
	} else {
	    /* This is wrong.  But Nucletron does it. */
	    logfile_printf (
		"Warning: RTDOSE gfov[0] is neither 0 nor ipp[2].\n"
		"This violates the DICOM standard.  Proceeding anyway...\n");
	    /* Nucletron seems to work by ignoring absolute origin (???) */
	}
    }

    /* (2) Handle case where gfov_len == 1 (only one slice). */
    if (gfov_len == 1) {
	spacing[2] = spacing[0];
    }

    /* (3) Check to make sure spacing is regular. */
    for (plm_long i = 1; i < gfov_len; i++) {
	if (i == 1) {
	    spacing[2] = gfov[1] - gfov[0];
	} else {
	    float sp = gfov[i] - gfov[i-1];
	    if (fabs(sp - spacing[2]) > GFOV_SPACING_TOL) {
		print_and_exit ("Error RTDOSE grid has irregular spacing:"
		    "%f vs %f.\n", sp, spacing[2]);
	    }
	}
    }
    free (gfov);

    /* DoseGridScaling -- if element doesn't exist, scaling is 1.0 */
    float dose_scaling = 1.0;
    val = d_ptr->ds_rtdose->get_cstr (DCM_DoseGridScaling);
    if (val) {
        /* No need to check for success, let scaling be 1.0 if failure */
        sscanf (val, "%f", &dose_scaling);
    }

    printf ("RTDOSE: dim = %d %d %d\n  ipp = %f %f %f\n  spc = %f %f %f\n"
        "  dc  = %f %f %f %f %f %f\n",
        (int) dim[0], (int) dim[1], (int) dim[2],
        ipp[0], ipp[1], ipp[2], 
        spacing[0], spacing[1], spacing[2],
        direction_cosines[0], direction_cosines[1], direction_cosines[2], 
        direction_cosines[3], direction_cosines[4], direction_cosines[5]
    );

    uint16_t bits_alloc, bits_stored, high_bit, pixel_rep;
    rc = d_ptr->ds_rtdose->get_uint16 (DCM_BitsAllocated, &bits_alloc);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_BitsAllocated in rtdose\n");
    }
    rc = d_ptr->ds_rtdose->get_uint16 (DCM_BitsStored, &bits_stored);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_BitsStored in rtdose\n");
    }
    rc = d_ptr->ds_rtdose->get_uint16 (DCM_HighBit, &high_bit);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_HighBit in rtdose\n");
    }
    rc = d_ptr->ds_rtdose->get_uint16 (DCM_PixelRepresentation, &pixel_rep);
    if (!rc) {
        print_and_exit ("Couldn't find DCM_PixelRepresentation in rtdose\n");
    }

    printf ("Bits_alloc: %d\n", (int) bits_alloc);
    printf ("Bits_stored: %d\n", (int) bits_stored);
    printf ("High_bit: %d\n", (int) high_bit);
    printf ("Pixel_rep: %d\n", (int) pixel_rep);

    /* Create output dose image */
    Plm_image::Pointer dose = Plm_image::New();
    this->set_dose (dose);

    /* Create Volume */
    Volume *vol = new Volume (dim, ipp, spacing, direction_cosines, PT_FLOAT, 1);
    float *img = (float*) vol->img;

    /* Bind volume to plm_image */
    dose->set_volume (vol);

    /* PixelData */
    unsigned long length = 0;
    if (pixel_rep == 0) {
        const uint16_t* pixel_data;
        rc = d_ptr->ds_rtdose->get_uint16_array (
            DCM_PixelData, &pixel_data, &length);
        printf ("rc = %d, length = %lu, npix = %ld\n", 
            rc, length, (long) vol->npix);
        if (bits_stored == 16) {
            dcmtk_dose_copy (img, (const uint16_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else if (bits_stored == 32) {
            dcmtk_dose_copy (img, (const uint32_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else {
            d_ptr->dose.reset();
            print_and_exit ("Unknown pixel representation (%d %d)\n",
                bits_stored, pixel_rep);
        }
    } else {
        const int16_t* pixel_data;
        rc = d_ptr->ds_rtdose->get_int16_array (
            DCM_PixelData, &pixel_data, &length);
        if (bits_stored == 16) {
            dcmtk_dose_copy (img, (const int16_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else if (bits_stored == 32) {
            dcmtk_dose_copy (img, (const int32_t*) pixel_data, 
                vol->npix, dose_scaling);
        } else {
            d_ptr->dose.reset();
            print_and_exit ("Unknown pixel representation (%d %d)\n",
                bits_stored, pixel_rep);
        }
    }
}

void
Dcmtk_rt_study::save_dose (const char *dicom_dir)
{
    OFCondition ofc;
    std::string s;
    Metadata::Pointer dose_metadata;
    if (d_ptr->rt_study_metadata) {
        dose_metadata = d_ptr->rt_study_metadata->get_dose_metadata ();
    }
    Volume::Pointer dose_volume = d_ptr->dose->get_volume_float ();

    /* Prepare dcmtk */
    DcmFileFormat fileformat;
    DcmDataset *dataset = fileformat.getDataset();
    DcmItem *dcm_item = 0;

    /* ----------------------------------------------------------------- */
    /*     Part 1  -- General header                                     */
    /* ----------------------------------------------------------------- */
    dataset->putAndInsertString (DCM_ImageType, 
        "DERIVED\\SECONDARY\\REFORMATTED");
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationDate, 
        d_ptr->rt_study_metadata->get_study_date());
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, 
        d_ptr->rt_study_metadata->get_study_time());
    dataset->putAndInsertOFStringArray(DCM_InstanceCreatorUID, 
        PLM_UID_PREFIX);
    dataset->putAndInsertString (DCM_SOPClassUID, UID_RTDoseStorage);
    dataset->putAndInsertString (DCM_SOPInstanceUID, 
        d_ptr->rt_study_metadata->get_dose_instance_uid());
    dataset->putAndInsertOFStringArray (DCM_StudyDate, 
        d_ptr->rt_study_metadata->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_StudyTime, 
        d_ptr->rt_study_metadata->get_study_time());
    dcmtk_copy_from_metadata (dataset, dose_metadata, 
        DCM_StudyDescription, "");

    dataset->putAndInsertOFStringArray (DCM_AccessionNumber, "");
    dataset->putAndInsertOFStringArray (DCM_Modality, "RTDOSE");
    dataset->putAndInsertString (DCM_Manufacturer, "Plastimatch");
    dataset->putAndInsertString (DCM_InstitutionName, "");
    dataset->putAndInsertString (DCM_ReferringPhysicianName, "");
    dataset->putAndInsertString (DCM_StationName, "");
    dcmtk_copy_from_metadata (dataset, dose_metadata, 
        DCM_SeriesDescription, "");
    dataset->putAndInsertString (DCM_ManufacturerModelName, "Plastimatch");

#if defined (commentout)
    /* (0008,1110) DCM_ReferencedStudySequence -- probably not needed */
    dcm_item = 0;
    dataset->findOrCreateSequenceItem (
        DCM_ReferencedStudySequence, dcm_item, -2);
    dcm_item->putAndInsertString (DCM_ReferencedSOPClassUID,
        UID_RETIRED_StudyComponentManagementSOPClass);
    dcm_item->putAndInsertString (DCM_ReferencedSOPInstanceUID,
        d_ptr->rt_study_metadata->get_study_uid());
#endif

    /* (0008,1140) DCM_ReferencedImageSequence -- MIM likes this */
    dcm_item = 0;
    dataset->findOrCreateSequenceItem (
        DCM_ReferencedImageSequence, dcm_item, -2);
    dcm_item->putAndInsertString (DCM_ReferencedSOPClassUID,
        UID_CTImageStorage);
    dcm_item->putAndInsertString (DCM_ReferencedSOPInstanceUID,
        d_ptr->rt_study_metadata->get_ct_series_uid());

    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_PatientName, "");
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_PatientID, "");
    dataset->putAndInsertString (DCM_PatientBirthDate, "");
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_PatientSex, "O");
    dataset->putAndInsertString (DCM_SliceThickness, "");
    dataset->putAndInsertString (DCM_SoftwareVersions,
        PLASTIMATCH_VERSION_STRING);
    dataset->putAndInsertString (DCM_StudyInstanceUID, 
        d_ptr->rt_study_metadata->get_study_uid());
    dataset->putAndInsertString (DCM_SeriesInstanceUID, 
        d_ptr->rt_study_metadata->get_dose_series_uid());
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_StudyID, "10001");
    dcmtk_copy_from_metadata (dataset, dose_metadata, DCM_SeriesNumber, "1");
    dataset->putAndInsertString (DCM_InstanceNumber, "1");
    
    s = string_format ("%g\\%g\\%g", dose_volume->origin[0], 
        dose_volume->origin[1], dose_volume->origin[2]);
    /* GCS FIX: PatientOrientation */
    dataset->putAndInsertString (DCM_PatientOrientation, "L/P");
    dataset->putAndInsertString (DCM_ImagePositionPatient, s.c_str());
    s = string_format ("%g\\%g\\%g\\%g\\%g\\%g",
    dose_volume->direction_cosines[0],
    dose_volume->direction_cosines[3],
    dose_volume->direction_cosines[6],
    dose_volume->direction_cosines[1],
    dose_volume->direction_cosines[4],
    dose_volume->direction_cosines[7]);
    dataset->putAndInsertString (DCM_ImageOrientationPatient, s.c_str());
    dataset->putAndInsertString (DCM_FrameOfReferenceUID, 
        d_ptr->rt_study_metadata->get_frame_of_reference_uid());

    dataset->putAndInsertString (DCM_SamplesPerPixel, "1");
    dataset->putAndInsertString (DCM_PhotometricInterpretation, "MONOCHROME2");
    s = string_format ("%d", (int) dose_volume->dim[2]);
    dataset->putAndInsertString (DCM_NumberOfFrames, s.c_str());

    /* GCS FIX: Add FrameIncrementPointer */
    dataset->putAndInsertString (DCM_FrameIncrementPointer, 
        "(3004,000c)");

    dataset->putAndInsertUint16 (DCM_Rows, dose_volume->dim[1]);
    dataset->putAndInsertUint16 (DCM_Columns, dose_volume->dim[0]);
    s = string_format ("%g\\%g", 
        dose_volume->spacing[1], dose_volume->spacing[0]);
    dataset->putAndInsertString (DCM_PixelSpacing, s.c_str());

    dataset->putAndInsertString (DCM_BitsAllocated, "32");
    dataset->putAndInsertString (DCM_BitsStored, "32");
    dataset->putAndInsertString (DCM_HighBit, "31");
    if (dose_metadata 
        && dose_metadata->get_metadata(0x3004, 0x0004) == "ERROR")
    {
        dataset->putAndInsertString (DCM_PixelRepresentation, "1");
    } else {
        dataset->putAndInsertString (DCM_PixelRepresentation, "0");
    }

    dataset->putAndInsertString (DCM_DoseUnits, "GY");
    dcmtk_copy_from_metadata (dataset, dose_metadata, 
        DCM_DoseType, "PHYSICAL");
    dataset->putAndInsertString (DCM_DoseSummationType, "PLAN");

    s = std::string ("0");
    for (int i = 1; i < dose_volume->dim[2]; i++) {
	s += string_format ("\\%g", dose_volume->direction_cosines[8] * i * dose_volume->spacing[2]);
    }
    dataset->putAndInsertString (DCM_GridFrameOffsetVector, s.c_str());
    
    /* GCS FIX:
       Leave ReferencedRTPlanSequence empty (until I can cross reference) */

    /* We need to convert image to uint16_t, but first we need to 
       scale it so that the maximum dose fits in a 16-bit unsigned 
       integer.  Compute an appropriate scaling factor based on the 
       maximum dose. */

    /* Copy the image so we don't corrupt the original */
    Volume::Pointer dose_copy = dose_volume->clone();

    /* Find the maximum value in the image */
    double min_val, max_val, avg;
    int non_zero, num_vox;
    dose_copy->convert (PT_FLOAT);
    volume_stats (dose_copy, &min_val, &max_val, &avg, &non_zero, &num_vox);

    /* Find scale factor */
    float dose_scale;
    if (dose_metadata 
        && dose_metadata->get_metadata(0x3004, 0x0004) == "ERROR")
    {
	/* Dose error is signed integer */
	float dose_scale_min = min_val / INT32_T_MIN * 1.001;
	float dose_scale_max = max_val / INT32_T_MAX * 1.001;
	dose_scale = std::max(dose_scale_min, dose_scale_max);
    } else {
        /* Dose is unsigned integer */
        dose_scale = max_val / UINT32_T_MAX * 1.001;
    }

    /* Scale the image and add scale factor to dataset */
    dose_copy->scale_inplace (1 / dose_scale);
    s = string_format ("%g", dose_scale);
    dataset->putAndInsertString (DCM_DoseGridScaling, s.c_str());

    /* (300c,0002) ReferencedRTPlanSequence -- for future expansion */
    dcm_item = 0;
    dataset->findOrCreateSequenceItem (
        DCM_ReferencedRTPlanSequence, dcm_item, -2);
    dcm_item->putAndInsertString (DCM_ReferencedSOPClassUID,
        UID_RTPlanStorage);
    dcm_item->putAndInsertString (DCM_ReferencedSOPInstanceUID,
        d_ptr->rt_study_metadata->get_plan_instance_uid());

    /* (300c,0060) DCM_ReferencedStructureSetSequence -- MIM likes this */
    dcm_item = 0;
    dataset->findOrCreateSequenceItem (
        DCM_ReferencedStructureSetSequence, dcm_item, -2);
    dcm_item->putAndInsertString (DCM_ReferencedSOPClassUID,
        UID_RTStructureSetStorage);
    dcm_item->putAndInsertString (DCM_ReferencedSOPInstanceUID,
        d_ptr->rt_study_metadata->get_rtss_instance_uid());

    /* Convert image bytes to integer, then add to dataset */
    if (dose_metadata 
        && dose_metadata->get_metadata(0x3004, 0x0004) == "ERROR")
    {
	dose_copy->convert (PT_INT32);
        dataset->putAndInsertUint16Array (DCM_PixelData, 
            (Uint16*) dose_copy->img, 2*dose_copy->npix);
    } else {
	dose_copy->convert (PT_UINT32);
        dataset->putAndInsertUint16Array (DCM_PixelData, 
            (Uint16*) dose_copy->img, 2*dose_copy->npix);
    }

    /* ----------------------------------------------------------------- */
    /*     Write the output file                                         */
    /* ----------------------------------------------------------------- */
    std::string filename;
    if (d_ptr->filenames_with_uid) {
        filename = string_format ("%s/dose_%s.dcm", dicom_dir,
            d_ptr->rt_study_metadata->get_dose_series_uid());
    } else {
        filename = string_format ("%s/dose.dcm", dicom_dir);
    }
    make_parent_directories (filename);

    ofc = fileformat.saveFile (filename.c_str(), EXS_LittleEndianExplicit);
    if (ofc.bad()) {
        print_and_exit ("Error: cannot write DICOM RTDOSE (%s)\n", 
            ofc.text());
    }
}
