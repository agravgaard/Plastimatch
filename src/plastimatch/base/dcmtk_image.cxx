/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <climits>
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
#include "dcmtk_series.h"
#include "dcmtk_slice_data.h"
#include "dcmtk_uid.h"
#include "file_util.h"
#include "logfile.h"
#include "rt_study_metadata.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_math.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "slice_list.h"
#include "string_util.h"
#include "volume.h"

void
Dcmtk_loader::image_load ()
{
    /* Set up outputs */
    Plm_image::Pointer pli = Plm_image::New();
    d_ptr->img = pli;

    /* Make abbreviations */
    Dcmtk_series *ds_image = d_ptr->ds_image;
    const Dcmtk_file_list& ds_flist = ds_image->get_flist ();

    /* Create a container to hold different groups of files */
    std::list<Dcmtk_file_list> group_list;

    /* Arrange files into groups according to direction cosines */
    for (Dcmtk_file_list::const_iterator it = ds_flist.begin();
         it != ds_flist.end(); ++it)
    {
        const Dcmtk_file::Pointer& df = (*it);
        bool match_found = false;
        std::list<Dcmtk_file_list>::iterator grit;
        for (grit = group_list.begin(); grit != group_list.end(); ++grit) {
            Dcmtk_file_list& flp = *grit;
            const Dcmtk_file::Pointer& flp_df = flp.front();

            if (flp_df->get_direction_cosines() 
                == df->get_direction_cosines())
            {
                /* Add logic to append to flp */
                //printf ("Match found.  :)\n");
                match_found = true;
                flp.push_back (df);
                break;
            }
        }
        if (match_found) {
            continue;
        }
        /* Else insert new element into group_list */
        group_list.push_back (Dcmtk_file_list());
        group_list.back().push_back (df);
    }

    /* If multiple groups, emit a warning.  Choose the largest group. */
    Dcmtk_file_list *flist = &group_list.front ();
    if (group_list.size() > 1) {
        lprintf ("Warning, DICOM series with multiple direction cosines\n");
        std::list<Dcmtk_file_list>::iterator grit;
        for (grit = group_list.begin(); grit != group_list.end(); ++grit) {
            if ((*grit).size() > flist->size()) {
                flist = &*grit;
            }
        }
    }
    
    /* Sort group in Z direction */
    flist->sort (dcmtk_file_compare_z_position);

    /* 
     * GCS FIX:
     * Remove minimum 2 slices requirement
     * Check consistency (dim, origin) of images w/in series
     * Different image types
     * Refine slice spacing based on entire chunk size
     */

    /* Check for minimum 2 slices */
    if (flist->size() < 2) {
        return;
    }

    /* Get first slice */
    const Dcmtk_file* df = (*flist->begin()).get();
    
    /* Store UIDs */
    if (d_ptr->rt_meta) {
        d_ptr->rt_meta->set_ct_series_uid (
            df->get_cstr (DCM_SeriesInstanceUID));
        d_ptr->rt_meta->set_frame_of_reference_uid (
            df->get_cstr (DCM_FrameOfReferenceUID));
        d_ptr->rt_meta->set_study_uid (
            df->get_cstr (DCM_StudyInstanceUID));
        d_ptr->rt_meta->set_study_date (
            df->get_cstr (DCM_StudyDate));
        d_ptr->rt_meta->set_study_time (
            df->get_cstr (DCM_StudyTime));

        /* Store remaining metadata */
        Metadata::Pointer& study_metadata = d_ptr->rt_meta->get_study_metadata ();
        dcmtk_copy_into_metadata (study_metadata, df, DCM_PatientName);
        dcmtk_copy_into_metadata (study_metadata, df, DCM_PatientID);
        dcmtk_copy_into_metadata (study_metadata, df, DCM_PatientSex);
        dcmtk_copy_into_metadata (study_metadata, df, DCM_PatientPosition);
        dcmtk_copy_into_metadata (study_metadata, df, DCM_StudyID);

        Metadata::Pointer& image_metadata = d_ptr->rt_meta->get_image_metadata ();
        dcmtk_copy_into_metadata (image_metadata, df, DCM_Modality);
    }

    /* Divine image type */
    uint16_t samp_per_pix, bits_alloc, bits_stored, high_bit, pixel_rep;
    const char* phot_interp;
    bool rc = df->get_uint16 (DCM_SamplesPerPixel, &samp_per_pix);
    if (!rc) {
        return;
    }
    phot_interp = df->get_cstr (DCM_PhotometricInterpretation);
    if (!phot_interp) {
        return;
    }
    rc = df->get_uint16 (DCM_BitsAllocated, &bits_alloc);
    if (!rc) {
        return;
    }
    rc = df->get_uint16 (DCM_BitsStored, &bits_stored);
    if (!rc) {
        return;
    }
    rc = df->get_uint16 (DCM_HighBit, &high_bit);
    if (!rc) {
        return;
    }
    rc = df->get_uint16 (DCM_PixelRepresentation, &pixel_rep);
    if (!rc) {
        return;
    }

    float rescale_slope, rescale_intercept;
    rc = df->get_ds_float (DCM_RescaleIntercept, &rescale_intercept);
    if (!rc) {
        rescale_intercept = 0;
    }
    rc = df->get_ds_float (DCM_RescaleSlope, &rescale_slope);
    if (!rc) {
        rescale_slope = 1;
    }

#if defined (commentout)
    lprintf ("Samp_per_pix: %d\n", (int) samp_per_pix);
    lprintf ("Phot_interp: %s\n", phot_interp);
    lprintf ("Bits_alloc: %d\n", (int) bits_alloc);
    lprintf ("Bits_stored: %d\n", (int) bits_stored);
    lprintf ("High_bit: %d\n", (int) high_bit);
    lprintf ("Pixel_rep: %d\n", (int) pixel_rep);
    lprintf ("S/I = %f/%f\n", rescale_slope, rescale_intercept);
#endif
    
    /* Some kinds of images we don't know how to deal with.  
       Don't load these. */
    if (samp_per_pix != 1) {
        lprintf ("Sorry, couldn't load image: samp_per_pix\n");
        return;
    }
    if (strcmp (phot_interp, "MONOCHROME2")) {
        lprintf ("Sorry, couldn't load image: phot_interp\n");
        return;
    }
    if (bits_alloc != 16 && bits_alloc != 8) {
        lprintf ("Sorry, couldn't load image: bits_alloc\n");
        return;
    }
    if (bits_stored != high_bit + 1) {
        lprintf ("Sorry, couldn't load image: bits_stored/high_bit\n");
        return;
    }
    if (pixel_rep != 0 && pixel_rep != 1) {
        lprintf ("Sorry, couldn't load image: pixel_rep\n");
        return;
    }

    /* If PLM_CONFIG_VOL_LIST is enabled, the image will be loaded 
       into a PLM_IMG_TYPE_GPUIT_LIST */
#if (PLM_CONFIG_VOL_LIST)

    /* Get first slice of group */
    Dcmtk_file_list::iterator it = flist->begin();
    df = it->get();

    /* Get next slice in group */
    float z_init, z_prev, z_diff, z_last;
    int slice_no = 0;
    float best_chunk_z_start = z_init = z_prev = df->get_z_position ();

    ++it; ++slice_no;
    df = (*it).get();
    z_diff = df->get_z_position() - z_prev;
    z_last = z_prev = df->get_z_position();

    /* We want to find the number and spacing for each chunk 
       within the group.  These are used to set the dim and 
       spacing of the volume. */
    int this_chunk_start = 0, best_chunk_start = 0;
    float this_chunk_diff = z_diff, best_chunk_diff = z_diff;
    int this_chunk_len = 2, best_chunk_len = 2;

    /* Loop through remaining slices */
    while (++it != flist->end())
    {
        ++slice_no;
        printf ("Slice no: %d\n", slice_no);
        df = (*it).get();
        z_diff = df->get_z_position() - z_prev;
        z_last = z_prev = df->get_z_position();

        if (fabs (this_chunk_diff - z_diff) > 0.11) {
            /* Start a new chunk if difference in thickness is 
               more than 0.1 millimeter */
            this_chunk_start = slice_no - 1;
            this_chunk_len = 2;
            this_chunk_diff = z_diff;
        } else {
            /* Same thickness, increase size of this chunk */
            this_chunk_diff = ((this_chunk_len * this_chunk_diff) + z_diff)
                / (this_chunk_len + 1);
            this_chunk_len++;

            /* Check if this chunk is now the best chunk */
            if (this_chunk_len > best_chunk_len) {
                best_chunk_start = this_chunk_start;
                best_chunk_len = this_chunk_len;
                best_chunk_diff = this_chunk_diff;
                best_chunk_z_start = z_prev 
                    - (best_chunk_len-1) * best_chunk_diff;
            }
        }
    }

#if defined (commentout)        
    Dcmtk_file_list& flp = *grit;
    const Dcmtk_file::Pointer dfp = grit->front();
    Volume::Pointer vol = Volume::New (
        const plm_long dim[3], 
        const float origin[3], 
        const float spacing[3], 
        &dfp->get_direction_cosines(),
        vh,
        PT_FLOAT, 1);
#endif

#else /* NOT VOL_LIST */
    /* Get next slice in first chunk */
    float z_init, z_prev, z_diff, z_last;
    int slice_no = 0;
    float best_chunk_z_start = z_init = z_prev = df->get_z_position ();

    std::list<Dcmtk_file::Pointer>::const_iterator it = flist->begin();
    ++it; ++slice_no;
    df = (*it).get();
    z_diff = df->get_z_position() - z_prev;
    z_last = z_prev = df->get_z_position();

    /* We want to find the largest chunk with equal spacing.  This will 
       be used to resample in the case of irregular spacing. */
    int this_chunk_start = 0, best_chunk_start = 0;
    float this_chunk_diff = z_diff, best_chunk_diff = z_diff;
    size_t this_chunk_len = 2, best_chunk_len = 2;

    /* Loop through remaining slices */
    while (++it != flist->end())
    {
	++slice_no;
	df = (*it).get();
	z_diff = df->get_z_position() - z_prev;
	z_last = z_prev = df->get_z_position();

	if (fabs (this_chunk_diff - z_diff) > 0.11) {
	    /* Start a new chunk if difference in thickness is 
	       more than 0.1 millimeter */
	    this_chunk_start = slice_no - 1;
	    this_chunk_len = 2;
	    this_chunk_diff = z_diff;
	} else {
	    /* Same thickness, increase size of this chunk */
	    this_chunk_diff = ((this_chunk_len * this_chunk_diff) + z_diff)
		/ (this_chunk_len + 1);
	    this_chunk_len++;

	    /* Check if this chunk is now the best chunk */
	    if (this_chunk_len > best_chunk_len) {
		best_chunk_start = this_chunk_start;
		best_chunk_len = this_chunk_len;
		best_chunk_diff = this_chunk_diff;
		best_chunk_z_start = z_prev 
		    - (best_chunk_len-1) * best_chunk_diff;
	    }
	}
    }

    /* Report information about best chunk */
    if (best_chunk_len != flist->size()) {
        lprintf ("** Warning, inequal slice spacing detected when loading DICOM.\n");
        lprintf ("Best chunck:\n  Slices %d to %d from (0 to %d)\n"
            "  Z_loc = %f %f\n" 
            "  Slice spacing = %f\n", 
            best_chunk_start, best_chunk_start + best_chunk_len - 1, slice_no, 
            best_chunk_z_start, 
            best_chunk_z_start + (best_chunk_len - 1) * best_chunk_diff, 
            best_chunk_diff);
    }
    
    /* Some debugging info */
#if defined (commentout)
    lprintf ("Slices: ");
    for (it = flist->begin(); it != flist->end(); ++it) {
        df = (*it).get();
	lprintf ("%f ", df->get_z_position());
    }
    lprintf ("\n");
#endif

    /* Create a Volume_header to hold the image geometry */
    Volume_header vh;
    plm_long *dim = vh.get_dim();

    /* Compute resampled volume header */
    int slices_before = 
	ROUND_INT ((best_chunk_z_start - z_init) / best_chunk_diff);
    int slices_after = 
	ROUND_INT ((z_last - best_chunk_z_start 
		- (best_chunk_len - 1) * best_chunk_diff) / best_chunk_diff);
    df = (*flist->begin()).get();
    vh.clone (df->get_volume_header());
    dim[2] = slices_before + best_chunk_len + slices_after;
    vh.get_origin()[2] = best_chunk_z_start - slices_before * best_chunk_diff;
    vh.get_spacing()[2] = best_chunk_diff;

    /* Store image header */
    if (d_ptr->rt_meta) {
        d_ptr->rt_meta->set_image_header (Plm_image_header (vh));
    }

    /* More debugging info */
    //vh.print ();

    /* Still more debugging info */
#if defined (commentout)
    lprintf ("Resamples slices: ");
    for (plm_long i = 0; i < dim[2]; i++) {
	lprintf ("%f ", vh.get_origin()[2] + i * vh.get_spacing()[2]);
    }
    lprintf ("\n");
#endif

    pli->m_type = PLM_IMG_TYPE_GPUIT_FLOAT;
    pli->m_original_type = PLM_IMG_TYPE_GPUIT_FLOAT;
    Volume* vol = new Volume (vh, PT_FLOAT, 1);

    pli->set_volume (vol);
    float* img = (float*) vol->img;

    for (plm_long i = 0; i < dim[2]; i++) {
	/* Find the best slice, using nearest neighbor interpolation */
	std::list<Dcmtk_file::Pointer>::const_iterator best_slice_it 
            = flist->begin();
	float best_z_dist = FLT_MAX;
	float z_pos = vh.get_origin()[2] + i * vh.get_spacing()[2];
	for (it = flist->begin(); it != flist->end(); ++it) {
	    float this_z_dist = fabs ((*it)->get_z_position() - z_pos);
	    if (this_z_dist < best_z_dist) {
		best_z_dist = this_z_dist;
		best_slice_it = it;
	    }
	}

	/* Load the slice image data into volume */
	df = (*best_slice_it).get();

#if defined (commentout)
	lprintf ("Loading slice z=%f at location z=%f\n",
	    (*best_slice_it)->get_z_position(), z_pos);
#endif

        /* GCS FIX: This should probably use DicomImage::getOutputData()
           cf. http://support.dcmtk.org/docs/mod_dcmimage.html */
	const uint8_t* pixel_data_8;
	const uint16_t* pixel_data_16;
	unsigned long length = 0;
        rc = 0;
        if (bits_alloc == 8) {
            rc = df->get_uint8_array (DCM_PixelData, &pixel_data_8, &length);
        } else if (bits_alloc == 16) {
            rc = df->get_uint16_array (DCM_PixelData, &pixel_data_16, &length);
        }
	if (!rc) {
	    print_and_exit ("Oops.  Error reading pixel data.  Punting.\n");
	}
	if (((long) length) != dim[0] * dim[1]) {
	    print_and_exit ("Oops.  Dicom image had wrong length "
		"(%d vs. %d x %d).\n", length, dim[0], dim[1]);
	}

        /* Apply slope and offset */
        if (bits_alloc == 8) {
            for (plm_long j = 0; j < (plm_long) length; j++) {
                img[j] = rescale_slope * (int8_t) pixel_data_8[j] 
                    + rescale_intercept;
            }
        } else if (bits_alloc == 16) {
            for (plm_long j = 0; j < (plm_long) length; j++) {
                img[j] = rescale_slope * (int16_t) pixel_data_16[j] 
                    + rescale_intercept;
            }
        }
	img += length;

	/* Store slice UID */
        if (d_ptr->rt_meta) {
            d_ptr->rt_meta->set_slice_uid (i, df->get_cstr (DCM_SOPInstanceUID));
        }
    }
#endif /* NOT VOL_LIST */
    if (d_ptr->rt_meta) {
        d_ptr->rt_meta->set_slice_list_complete ();
    }
}

static void
dcmtk_save_slice (const Rt_study_metadata::Pointer drs, Dcmtk_slice_data *dsd)
{
    std::string tmp;
    DcmFileFormat fileformat;
    DcmDataset *dataset = fileformat.getDataset();
    Metadata::Pointer image_metadata;
    if (drs) {
        image_metadata = drs->get_image_metadata ();
    }

    /* Get modality */
    std::string modality = "CT";
    if (image_metadata) {
        std::string metadata_modality = image_metadata->get_metadata (
            DCM_Modality.getGroup(), DCM_Modality.getElement());
        if (metadata_modality != "") {
            modality = metadata_modality;
        }
    }

    dataset->putAndInsertString (DCM_ImageType, 
        "DERIVED\\SECONDARY\\REFORMATTED");
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationDate, 
        drs->get_study_date());
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, 
        drs->get_study_time());

    /* The SOPClassUID depends on the modality */
    if (modality == "MR") {
        dataset->putAndInsertString (
            DCM_SOPClassUID, UID_MRImageStorage);
    }
    else if (modality == "PT") {
        dataset->putAndInsertString (
            DCM_SOPClassUID, UID_NuclearMedicineImageStorage);
    }
    else {
        dataset->putAndInsertString (
            DCM_SOPClassUID, UID_CTImageStorage);
    }
    dataset->putAndInsertString (DCM_SOPInstanceUID, dsd->slice_uid);

    /* General Study Module */
    dataset->putAndInsertString (DCM_StudyInstanceUID, drs->get_study_uid());
    dataset->putAndInsertOFStringArray (DCM_StudyDate, drs->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_StudyTime, drs->get_study_time());
    dcmtk_copy_from_metadata (dataset, image_metadata, 
        DCM_StudyID, "10001");
    dcmtk_copy_from_metadata (dataset, image_metadata, 
        DCM_StudyDescription, "");

    dataset->putAndInsertString (DCM_AccessionNumber, "");
    dataset->putAndInsertString (DCM_Modality, modality.c_str());
    dataset->putAndInsertString (DCM_Manufacturer, "Plastimatch");
    dataset->putAndInsertString (DCM_ReferringPhysicianName, "");
    dcmtk_copy_from_metadata (dataset, image_metadata, 
        DCM_SeriesDescription, "");
    dcmtk_copy_from_metadata (dataset, image_metadata, DCM_PatientName, "");
    dcmtk_copy_from_metadata (dataset, image_metadata, DCM_PatientID, "");
    dcmtk_copy_from_metadata (dataset, image_metadata, 
        DCM_PatientBirthDate, "");
    dcmtk_copy_from_metadata (dataset, image_metadata, DCM_PatientSex, "");
    dataset->putAndInsertString (DCM_SliceThickness, dsd->sthk.c_str());
    dataset->putAndInsertString (DCM_SoftwareVersions, 
        PLASTIMATCH_VERSION_STRING);
    dcmtk_copy_from_metadata (dataset, image_metadata, 
        DCM_PatientPosition, "HFS");

    dataset->putAndInsertString (DCM_SeriesInstanceUID, 
        drs->get_ct_series_uid());
    dcmtk_copy_from_metadata (dataset, image_metadata, DCM_SeriesNumber, "1");
    tmp = string_format ("%d", dsd->instance_no);
    dataset->putAndInsertString (DCM_InstanceNumber, tmp.c_str());
    //dataset->putAndInsertString (DCM_InstanceNumber, "0");
    /* DCM_PatientOrientation seems to be not required.  */
    // dataset->putAndInsertString (DCM_PatientOrientation, "L\\P");
    dataset->putAndInsertString (DCM_ImagePositionPatient, dsd->ipp.c_str());
    dataset->putAndInsertString (DCM_ImageOrientationPatient, 
        dsd->iop.c_str());
    dataset->putAndInsertString (DCM_FrameOfReferenceUID, 
        drs->get_frame_of_reference_uid());
    /* XVI 4.5 requires a DCM_PositionReferenceIndicator */
    dataset->putAndInsertString (DCM_PositionReferenceIndicator, "SP");
    dataset->putAndInsertString (DCM_SliceLocation, dsd->sloc.c_str());
    dataset->putAndInsertString (DCM_SamplesPerPixel, "1");
    dataset->putAndInsertString (DCM_PhotometricInterpretation, "MONOCHROME2");
    dataset->putAndInsertUint16 (DCM_Rows, (Uint16) dsd->vol->dim[1]);
    dataset->putAndInsertUint16 (DCM_Columns, (Uint16) dsd->vol->dim[0]);
    tmp = string_format ("%f\\%f", dsd->vol->spacing[0], dsd->vol->spacing[1]);
    dataset->putAndInsertString (DCM_PixelSpacing, tmp.c_str());
    dataset->putAndInsertString (DCM_BitsAllocated, "16");
    dataset->putAndInsertString (DCM_BitsStored, "16");
    dataset->putAndInsertString (DCM_HighBit, "15");
    dataset->putAndInsertString (DCM_PixelRepresentation, "1");

    tmp = string_format ("%f", dsd->intercept);
    dataset->putAndInsertString (DCM_RescaleIntercept, tmp.c_str());
    tmp = string_format ("%f", dsd->slope);
    dataset->putAndInsertString (DCM_RescaleSlope, tmp.c_str());

    //dataset->putAndInsertString (DCM_RescaleIntercept, "-1024");
    //dataset->putAndInsertString (DCM_RescaleSlope, "1");
    dataset->putAndInsertString (DCM_RescaleType, "HU");

    dcmtk_copy_from_metadata (dataset, image_metadata,
        DCM_WindowCenter, "40");
    dcmtk_copy_from_metadata (dataset, image_metadata,
        DCM_WindowWidth, "400");

    /* Convert to 16-bit signed int */
    for (size_t i = 0; i < dsd->slice_size; i++) {
        float f = dsd->slice_float[i];
        //dsd->slice_int16[i] = (int16_t) (f + 1024);
        dsd->slice_int16[i] = (int16_t) 
            ((f - dsd->intercept) / dsd->slope);
    }

    dataset->putAndInsertUint16Array (DCM_PixelData, 
        (Uint16*) dsd->slice_int16, dsd->slice_size);
    OFCondition status = fileformat.saveFile (dsd->fn.c_str(), 
        EXS_LittleEndianExplicit);
    if (status.bad()) {
        print_and_exit ("Error: cannot write DICOM file (%s)\n", 
            status.text());
    }
}

void
Dcmtk_rt_study::save_image (
    const char *dicom_dir)
{
    Dcmtk_slice_data dsd;
    dsd.vol = this->get_image_volume_float();
    dsd.slice_size = dsd.vol->dim[0] * dsd.vol->dim[1];
    dsd.slice_int16 = new int16_t[dsd.slice_size];
    float *dc = dsd.vol->get_direction_matrix();
    dsd.iop = string_format ("%f\\%f\\%f\\%f\\%f\\%f",
        dc[0], dc[3], dc[6], dc[1], dc[4], dc[7]);

    Plm_image_header pih (dsd.vol.get());
    d_ptr->rt_study_metadata->set_image_header (pih);

    /* Find slope / offset on a per-volume basis */
    float vol_min = FLT_MAX;
    float vol_max = - FLT_MAX;
    float *img = (float*) dsd.vol->img;
    bool all_integers = true;
    for (plm_long v = 0; v < dsd.vol->npix; v++) {
        if (vol_min > img[v]) {
            vol_min = img[v];
        }
        else if (vol_max < img[v]) {
            vol_max = img[v];
        }
        if (img[v] != floorf(img[v])) {
            all_integers = false;
        }
    }
    /* Use a heuristic to determine intercept and offset.
       The heuristic is designed around the following principles:
       - prevent underflow when using low-precision DICOM string-encoded
         floating point numbers
       - map integers to integers
    */
    dsd.intercept = floorf (vol_min);
    if (all_integers) {
        dsd.slope = 1;
    }
    else {
        float range = vol_max - dsd.intercept;
        if (range < 1) {
            range = 1;
        }
        dsd.slope = range / (SHRT_MAX - 100);
        std::string tmp = string_format ("%f", dsd.slope);
        sscanf (tmp.c_str(), "%f", &dsd.slope);
    }

    for (plm_long k = 0; k < dsd.vol->dim[2]; k++) {
        /* GCS FIX #2:  This is possibly correct.  Not 100% sure. */
        float z_loc = dsd.vol->origin[2] + dc[8] * k * dsd.vol->spacing[2];
        dsd.instance_no = k;
        dsd.sthk = string_format ("%f", dsd.vol->spacing[2]);
        dsd.sloc = string_format ("%f", z_loc);
        /* GCS FIX #2:  "Ditto" */
        dsd.ipp = string_format ("%f\\%f\\%f", 
            dsd.vol->origin[0] + dc[2] * k * dsd.vol->spacing[2],
            dsd.vol->origin[1] + dc[5] * k * dsd.vol->spacing[2],
            dsd.vol->origin[2] + dc[8] * k * dsd.vol->spacing[2]);
        dcmtk_uid (dsd.slice_uid, PLM_UID_PREFIX);

        dsd.slice_float = &((float*)dsd.vol->img)[k*dsd.slice_size];

        /* Format filename and prepare output directory */
        if (d_ptr->filenames_with_uid) {
            dsd.fn = string_format ("%s/image%04d_%s.dcm", dicom_dir, (int) k,
                dsd.slice_uid);
        } else {
            dsd.fn = string_format ("%s/image%04d.dcm", dicom_dir, (int) k);
        }
        make_parent_directories (dsd.fn);

        /* Fix the uid into the metadata */
        d_ptr->rt_study_metadata->set_slice_uid (k, dsd.slice_uid);

        /* Save the file to disk */
        dcmtk_save_slice (d_ptr->rt_study_metadata, &dsd);
    }
    delete[] dsd.slice_int16;
    d_ptr->rt_study_metadata->set_slice_list_complete ();
}
