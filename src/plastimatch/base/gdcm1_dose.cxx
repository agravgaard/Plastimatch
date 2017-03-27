/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "gdcmBinEntry.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmUtil.h"

#include "file_util.h"
#include "gdcm1_dose.h"
#include "gdcm1_util.h"
#include "gdcm1_series.h"
#include "itk_image_stats.h"
#include "logfile.h"
#include "print_and_exit.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_int.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "rt_study_metadata.h"
#include "volume.h"

/* winbase.h defines GetCurrentTime which conflicts with gdcm function */
#if defined GetCurrentTime
# undef GetCurrentTime
#endif

/* This is the tolerance on irregularity of the grid spacing (in mm) */
#define GFOV_SPACING_TOL (1e-1)

/* This function probes whether or not the file is a dicom dose format */
bool
gdcm1_dose_probe (const char *dose_fn)
{
    gdcm::File *gdcm_file = new gdcm::File;
    std::string tmp;

    gdcm_file->SetMaxSizeLoadEntry (0xffff);
    gdcm_file->SetFileName (dose_fn);
    gdcm_file->SetLoadMode (0);
    gdcm_file->Load();

    /* Modality -- better be RTDOSE */
    tmp = gdcm_file->GetEntryValue (0x0008, 0x0060);
    delete gdcm_file;
    if (strncmp (tmp.c_str(), "RTDOSE", strlen("RTDOSE"))) {
	return false;
    } else {
	return true;
    }
}

template <class T> 
static
void
gdcm1_dose_copy_raw (float *img_out, T *img_in, int nvox, float scale)
{
    for (int i = 0; i < nvox; i++) {
	img_out[i] = img_in[i] * scale;
    }
}

Plm_image*
gdcm1_dose_load (Plm_image *pli, const char *dose_fn)
{
    int d, rc;
    gdcm::File *gdcm_file = new gdcm::File;
    Gdcm_series gs;
    std::string tmp;
    float ipp[3];
    plm_long dim[3];
    float spacing[3];
    float *gfov;    /* gfov = GridFrameOffsetVector */
    plm_long gfov_len;
    const char *gfov_str;
    float dose_scaling;

    gdcm_file->SetMaxSizeLoadEntry (0xffff);
    gdcm_file->SetFileName (dose_fn);
    gdcm_file->SetLoadMode (0);
    gdcm_file->Load();

    std::cout << " loading dose " << std::endl;
    /* Modality -- better be RTDOSE */
    tmp = gdcm_file->GetEntryValue (0x0008, 0x0060);
    if (strncmp (tmp.c_str(), "RTDOSE", strlen("RTDOSE"))) {
	print_and_exit ("Error.  Input file not RTDOSE: %s\n",
	    dose_fn);
    }

    /* ImagePositionPatient */
    tmp = gdcm_file->GetEntryValue (0x0020, 0x0032);
    rc = sscanf (tmp.c_str(), "%f\\%f\\%f", &ipp[0], &ipp[1], &ipp[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing RTDOSE ipp.\n");
    }

    /* Rows */
    tmp = gdcm_file->GetEntryValue (0x0028, 0x0010);
    rc = sscanf (tmp.c_str(), "%d", &d);
    if (rc != 1) {
	print_and_exit ("Error parsing RTDOSE rows.\n");
    }
    dim[1] = d;

    /* Columns */
    tmp = gdcm_file->GetEntryValue (0x0028, 0x0011);
    rc = sscanf (tmp.c_str(), "%d", &d);
    if (rc != 1) {
	print_and_exit ("Error parsing RTDOSE columns.\n");
    }
    dim[0] = d;

    /* PixelSpacing */
    tmp = gdcm_file->GetEntryValue (0x0028, 0x0030);
    rc = sscanf (tmp.c_str(), "%g\\%g", &spacing[1], &spacing[0]);
	
    if (rc != 2) {
	print_and_exit ("Error parsing RTDOSE pixel spacing.\n");
    }

    /* GridFrameOffsetVector */
    tmp = gdcm_file->GetEntryValue (0x3004, 0x000C);
    gfov = 0;
    gfov_len = 0;
    gfov_str = tmp.c_str();
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
	    /* Nucletron seems to work by ignoring absolute offset (???) */
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

    /* DoseGridScaling */
    dose_scaling = 1.0;
    tmp = gdcm_file->GetEntryValue (0x3004, 0x000E);
    rc = sscanf (tmp.c_str(), "%f", &dose_scaling);
    /* If element doesn't exist, scaling is 1.0 */

    /* Create output pli if necessary */
    if (!pli) pli = new Plm_image;
    pli->free ();

    /* Create Volume */
    Volume *vol = new Volume (dim, ipp, spacing, 0, PT_FLOAT, 1);
    float *img = (float*) vol->img;

    /* PixelData */
    gdcm::FileHelper gdcm_file_helper (gdcm_file);

    //plm_long image_data_size = gdcm_file_helper.GetImageDataSize();
    if (strcmp (gdcm_file->GetPixelType().c_str(), "16U")==0) {
	unsigned short* image_data = (unsigned short*) gdcm_file_helper.GetImageData();
	gdcm1_dose_copy_raw (img, image_data, vol->npix, dose_scaling);
    } else if (strcmp(gdcm_file->GetPixelType().c_str(),"32U")==0){
	printf ("Got 32U.\n");
	uint32_t* image_data = (uint32_t*) gdcm_file_helper.GetImageData ();
	gdcm1_dose_copy_raw (img, image_data, vol->npix, dose_scaling);
    } else {
	print_and_exit ("Error RTDOSE not type 16U and 32U (type=%s)\n",gdcm_file->GetPixelType().c_str());
    }

    /* GCS FIX: Do I need to do something about endian-ness? */

    /* Bind volume to plm_image */
    pli->set_volume (vol);

#if defined (commentout)
    printf ("IPP = %f %f %f\n", ipp[0], ipp[1], ipp[2]);
    printf ("DIM = %d %d %d\n", dim[0], dim[1], dim[2]);
    printf ("SPC = %f %f %f\n", spacing[0], spacing[1], spacing[2]);
    printf ("NVX = %d\n", dim[0] * dim[1] * dim[2]);
    printf ("ID  = size %d, type %s\n", image_data_size, 
	gdcm_file->GetPixelType().c_str());
#endif

    free (gfov);
    delete gdcm_file;
    return pli;
}

void
gdcm1_dose_save (
    Plm_image *pli,                     /* Input: dose image */
    const Rt_study_metadata *rsm, 
    const char *dose_fn                 /* Input: file to write to */
)
{
    int i;
    gdcm::File *gf = new gdcm::File ();
    Gdcm_series gs;
    const std::string &current_date = gdcm::Util::GetCurrentDate();
    const std::string &current_time = gdcm::Util::GetCurrentTime();
    std::string s;
    Plm_image_header plh;

    plh.set_from_plm_image (pli);

    printf ("Hello from gdcm_dose_save: fn = %s\n", dose_fn);

    make_directory_recursive (dose_fn);

    const Metadata::Pointer& meta = rsm->get_dose_metadata ();

    /* ----------------------------------------------------------------- */
    /*     Part 1  -- General header                                     */
    /* ----------------------------------------------------------------- */
    /* InstanceCreationDate */
    gf->InsertValEntry (current_date, 0x0008, 0x0012);
    /* InstanceCreationTime */
    gf->InsertValEntry (current_time, 0x0008, 0x0013);
    /* InstanceCreatorUID */
    gf->InsertValEntry (PLM_UID_PREFIX, 0x0008, 0x0014);
    /* SOPClassUID = RTDoseStorage */
    gf->InsertValEntry ("1.2.840.10008.5.1.4.1.1.481.2", 0x0008, 0x0016);
    /* SOPInstanceUID */
    gf->InsertValEntry (gdcm::Util::CreateUniqueUID (PLM_UID_PREFIX), 
			0x0008, 0x0018);
    /* StudyDate */
    gf->InsertValEntry ("20000101", 0x0008, 0x0020);
    /* StudyTime */
    gf->InsertValEntry ("120000", 0x0008, 0x0030);
    /* AccessionNumber */
    gf->InsertValEntry ("", 0x0008, 0x0050);
    /* Modality */
    gf->InsertValEntry ("RTDOSE", 0x0008, 0x0060);
    /* Manufacturer */
    gf->InsertValEntry ("Plastimatch", 0x0008, 0x0070);
    /* ReferringPhysiciansName */
    gf->InsertValEntry ("", 0x0008, 0x0090);
    /* SeriesDescription */
    set_gdcm_file_from_metadata (gf, meta, 0x0008, 0x103e);
    /* ManufacturersModelName */
    gf->InsertValEntry ("Plastimatch", 0x0008, 0x1090);
    /* PatientsName */
    set_gdcm_file_from_metadata (gf, meta, 0x0010, 0x0010);
    /* PatientID */
    set_gdcm_file_from_metadata (gf, meta, 0x0010, 0x0020);
    /* PatientsBirthDate */
    gf->InsertValEntry ("", 0x0010, 0x0030);
    /* PatientsSex */
    set_gdcm_file_from_metadata (gf, meta, 0x0010, 0x0040);
    /* SliceThickness */
    gf->InsertValEntry ("", 0x0018, 0x0050);
    /* SoftwareVersions */
    gf->InsertValEntry (PLASTIMATCH_VERSION_STRING, 0x0018, 0x1020);
    /* StudyInstanceUID */
    gf->InsertValEntry ((const char*) rsm->get_study_uid(), 0x0020, 0x000d);
    /* SeriesInstanceUID */
    gf->InsertValEntry (gdcm::Util::CreateUniqueUID (PLM_UID_PREFIX), 
			0x0020, 0x000e);
    /* StudyID */
    set_gdcm_file_from_metadata (gf, meta, 0x0020, 0x0010);
    /* SeriesNumber */
    gf->InsertValEntry ("", 0x0020, 0x0011);
    /* InstanceNumber */
    gf->InsertValEntry ("1", 0x0020, 0x0013);
    /* ImagePositionPatient */
    s = gdcm::Util::Format ("%g\\%g\\%g", 
    plh.GetOrigin()[0], plh.GetOrigin()[1], plh.GetOrigin()[2]);
    gf->InsertValEntry (s, 0x0020, 0x0032);

    /* ImageOrientationPatient */
    itk::Matrix < double, 3, 3 > direction=plh.GetDirection();
    s = gdcm::Util::Format ("%g\\%g\\%g\\%g\\%g\\%g",
    direction[0][0],
    direction[0][1],
    direction[0][2],
    direction[1][0],
    direction[1][1],
    direction[1][2]);
    gf->InsertValEntry (s, 0x0020, 0x0037);

    /* FrameOfReferenceUID */
    gf->InsertValEntry (rsm->get_frame_of_reference_uid(), 0x0020, 0x0052);

    /* SamplesPerPixel */
    gf->InsertValEntry ("1", 0x0028, 0x0002);
    /* PhotometricInterpretation */
    gf->InsertValEntry ("MONOCHROME2", 0x0028, 0x0004);
    /* NumberOfFrames */
    s = gdcm::Util::Format ("%d", plh.GetSize()[2]);
    gf->InsertValEntry (s, 0x0028, 0x0008);

    /* FrameIncrementPointer */
    /* Note: InsertValEntry doesn't work for AT value representations
       gf->InsertValEntry ("3004,000c", 0x0028, 0x0009); */
    uint16_t fip[2] = { 0x3004, 0x000c };
    gf->InsertBinEntry ((uint8_t*)fip, 4, 0x0028, 0x0009, std::string("AT"));

    /* Rows */
    s = gdcm::Util::Format ("%d", plh.GetSize()[1]);
    gf->InsertValEntry (s, 0x0028, 0x0010);
    /* Columns */
    s = gdcm::Util::Format ("%d", plh.GetSize()[0]);
    gf->InsertValEntry (s, 0x0028, 0x0011);
    /* PixelSpacing */
    s = gdcm::Util::Format ("%g\\%g", plh.GetSpacing()[1], plh.GetSpacing()[0]);
    gf->InsertValEntry (s, 0x0028, 0x0030);

    /* BitsAllocated */
    gf->InsertValEntry ("32", 0x0028, 0x0100);
    /* BitsStored */
    gf->InsertValEntry ("32", 0x0028, 0x0101);
    /* HighBit */
    gf->InsertValEntry ("31", 0x0028, 0x0102);
    /* PixelRepresentation */
    if (meta->get_metadata(0x3004, 0x0004) != "ERROR") {
	gf->InsertValEntry ("0", 0x0028, 0x0103);
    } else {
	gf->InsertValEntry ("1", 0x0028, 0x0103);
    }

    /* Do I need SmallestImagePixelValue, LargestImagePixelValue? */

    /* DoseUnits */
    gf->InsertValEntry ("GY", 0x3004, 0x0002);
    /* DoseType */
    if (meta->get_metadata(0x3004, 0x0004) != "") {
	set_gdcm_file_from_metadata (gf, meta, 0x3004, 0x0004);
    } else {
	gf->InsertValEntry ("PHYSICAL", 0x3004, 0x0004);
    }

    /* DoseSummationType */
    gf->InsertValEntry ("PLAN", 0x3004, 0x000a);

    /* GridFrameOffsetVector */
    s = std::string ("0");
    for (i = 1; i < plh.GetSize()[2]; i++) {
    s += gdcm::Util::Format ("\\%g", i * plh.GetSpacing()[2]);
    }
    gf->InsertValEntry (s, 0x3004, 0x000c);

    /* GCS FIX:
       Leave ReferencedRTPlanSequence empty (until I can cross reference) */

    /* We need to convert image to uint16_t, but first we need to 
       scale it.  The maximum dose needs to fit in a 16-bit unsigned 
       integer.  Older versions of plastimatch set the dose_scale to a 
       fixed value of 0.04 (based on the fact that this number was found 
       in the XiO sample).  With this scaling, the maximum dose is 262 Gy. 
       Now we compute an appropriate scaling factor based on the maximum 
       dose. */

    /* Copy the image so we don't corrupt the original */
    Plm_image::Pointer tmp = pli->clone ();
    /* Find the maximum value in the image */
    double min_val, max_val, avg;
    int non_zero, num_vox;
    tmp->convert (PLM_IMG_TYPE_ITK_FLOAT);
    itk_image_stats (tmp->m_itk_float, &min_val, &max_val, &avg, 
	&non_zero, &num_vox);

    float dose_scale;

    if (meta->get_metadata(0x3004, 0x0004) != "ERROR") {
	/* Dose is unsigned integer */
	dose_scale = max_val / UINT32_T_MAX * 1.001;
    } else {
	/* Dose error is signed integer */
	float dose_scale_min = min_val / INT32_T_MIN * 1.001;
	float dose_scale_max = max_val / INT32_T_MAX * 1.001;
	dose_scale = std::max(dose_scale_min, dose_scale_max);
    }

    /* Scale the image */
    tmp->convert (PLM_IMG_TYPE_GPUIT_FLOAT);
    Volume *vol = tmp->get_vol ();
    vol->scale_inplace (1 / dose_scale);

    /* Convert to integer */
    if (meta->get_metadata(0x3004, 0x0004) != "ERROR") {
	tmp->convert (PLM_IMG_TYPE_GPUIT_UINT32);
    } else {
	tmp->convert (PLM_IMG_TYPE_GPUIT_INT32);
    }

    vol = tmp->get_vol ();

    /* DoseGridScaling */
    s = gdcm::Util::Format ("%g", dose_scale);
    gf->InsertValEntry (s, 0x3004, 0x000e);
    /* PixelData */
    gdcm::FileHelper gfh (gf);
    gfh.SetImageData ((uint8_t*) vol->img, vol->npix * vol->pix_size);

    /* Do the actual writing out to file */
    gfh.WriteDcmExplVR (dose_fn);
}

/* Explicit instantiations */
template void gdcm1_dose_copy_raw (float *img_out, unsigned short *img_in, int nvox, float scale);
template void gdcm1_dose_copy_raw (float *img_out, unsigned long int *img_in, int nvox, float scale);
