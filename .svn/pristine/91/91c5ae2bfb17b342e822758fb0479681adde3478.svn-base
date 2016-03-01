/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "gdcmDocEntry.h"
#include "gdcmDocEntrySet.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmUtil.h"
#include "gdcmValEntry.h"

#include "file_util.h"
#include "gdcm1_rtss.h"
#include "gdcm1_util.h"
#include "metadata.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "rt_study_metadata.h"
#include "rtss.h"
#include "rtss_contour.h"
#include "rtss_roi.h"

/* winbase.h defines GetCurrentTime which conflicts with gdcm function */
#if defined GetCurrentTime
# undef GetCurrentTime
#endif

/* This function probes whether or not the file is a dicom rtss format */
bool
gdcm_rtss_probe (const char *rtss_fn)
{
    gdcm::File *rtss_file = new gdcm::File;
    std::string tmp;

    rtss_file->SetMaxSizeLoadEntry (0xffffff);
    rtss_file->SetFileName (rtss_fn);
    rtss_file->SetLoadMode (0);
    rtss_file->Load();

    /* Modality -- better be RTSTRUCT */
    tmp = rtss_file->GetEntryValue (0x0008, 0x0060);
    delete rtss_file;
    if (strncmp (tmp.c_str(), "RTSTRUCT", strlen("RTSTRUCT"))) {
	return false;
    } else {
	return true;
    }
}

void
gdcm_rtss_load (
    Rtss *cxt,   /* Output: this gets loaded into */
    Rt_study_metadata *rsm,    /* Output: this gets updated too */
    const char *rtss_fn        /* Input: the file that gets read */
)
{
    gdcm::File *rtss_file = new gdcm::File;
    gdcm::SeqEntry *seq;
    gdcm::SQItem *item;
    std::string tmp;

    rtss_file->SetMaxSizeLoadEntry (0xffffff);
    rtss_file->SetFileName (rtss_fn);
    rtss_file->SetLoadMode (0);
    rtss_file->Load();

    /* Modality -- better be RTSTRUCT */
    tmp = rtss_file->GetEntryValue (0x0008, 0x0060);
    if (strncmp (tmp.c_str(), "RTSTRUCT", strlen("RTSTRUCT"))) {
	print_and_exit ("Error.  Input file not an RT structure set: %s\n",
	    rtss_fn);
    }

    Metadata::Pointer& meta = rsm->get_study_metadata ();

    /* PatientName */
    set_metadata_from_gdcm_file (meta, rtss_file, 0x0010, 0x0010);

    /* PatientID */
    set_metadata_from_gdcm_file (meta, rtss_file, 0x0010, 0x0020);

    /* PatientSex */
    set_metadata_from_gdcm_file (meta, rtss_file, 0x0010, 0x0040);

    /* StudyID */
    /* GCS FIX: It would be useful to distinguish a loaded value 
       from a generated one */
#if defined (commentout)
    if (rdd->m_study_id.empty()) {
	tmp = rtss_file->GetEntryValue (0x0020, 0x0010);
	if (tmp != gdcm::GDCM_UNFOUND) {
	    rdd->m_study_id = tmp.c_str();
	}
    }
#endif
    tmp = rtss_file->GetEntryValue (0x0020, 0x0010);
    if (tmp != gdcm::GDCM_UNFOUND) {
        meta->set_metadata (0x0020, 0x0010, tmp.c_str());
    }

    /* StudyInstanceUID */
    /* GCS FIX: Here is another case where it would be useful to distinguish
       a loaded UID from a generated UID. 
       Until I get a fix, I'll omit loading this, and presume it is 
       loaded through either image load or referenced-ct */
#if defined (commentout)
    if (rdd->m_ct_study_uid.empty()) {
	tmp = rtss_file->GetEntryValue (0x0020, 0x000d);
	rdd->m_ct_study_uid = tmp.c_str();
    }
#endif

    /* ReferencedFrameOfReferenceSequence */
    gdcm::SeqEntry *rfor_seq = rtss_file->GetSeqEntry (0x3006,0x0010);
    if (rfor_seq) {

	/* FrameOfReferenceUID */
	item = rfor_seq->GetFirstSQItem ();
	if (item) {
            /* GCS FIX: Ditto */
#if defined (commentout)
	    tmp = item->GetEntryValue (0x0020,0x0052);
	    if (rdd->m_ct_fref_uid.empty()) {
		if (tmp != gdcm::GDCM_UNFOUND) {
		    rdd->m_ct_fref_uid = tmp.c_str();
		}
	    }
#endif
	    /* RTReferencedStudySequence */
	    gdcm::SeqEntry *rtrstudy_seq 
		= item->GetSeqEntry (0x3006, 0x0012);
	    if (rtrstudy_seq) {
	
		/* RTReferencedSeriesSequence */
		item = rtrstudy_seq->GetFirstSQItem ();
		if (item) {
		    gdcm::SeqEntry *rtrseries_seq 
			= item->GetSeqEntry (0x3006, 0x0014);
		    if (rtrseries_seq) {
			item = rtrseries_seq->GetFirstSQItem ();

			/* SeriesInstanceUID */
			if (item) {
                            /* GCS FIX: Ditto */
#if defined (commentout)
			    tmp = item->GetEntryValue (0x0020, 0x000e);
			    if (rdd->m_ct_series_uid.empty()) {
				if (tmp != gdcm::GDCM_UNFOUND) {
				    rdd->m_ct_series_uid = tmp.c_str();
				}
			    }
#endif
			}
		    }
		}
	    }
	}
    }

    /* StructureSetROISequence */
    seq = rtss_file->GetSeqEntry (0x3006,0x0020);
    for (item = seq->GetFirstSQItem (); item; item = seq->GetNextSQItem ()) {
	int structure_id;
	std::string roi_number, roi_name;
	roi_number = item->GetEntryValue (0x3006,0x0022);
	roi_name = item->GetEntryValue (0x3006,0x0026);
	if (1 != sscanf (roi_number.c_str(), "%d", &structure_id)) {
	    continue;
	}
	cxt->add_structure (roi_name, std::string(), structure_id);
    }

    /* ROIContourSequence */
    seq = rtss_file->GetSeqEntry (0x3006,0x0039);
    for (item = seq->GetFirstSQItem (); item; item = seq->GetNextSQItem ()) {
	int structure_id;
	std::string roi_display_color, referenced_roi_number;
	gdcm::SeqEntry *c_seq;
	gdcm::SQItem *c_item;
	Rtss_roi *curr_structure;

	/* Get id and color */
	referenced_roi_number = item->GetEntryValue (0x3006,0x0084);
	roi_display_color = item->GetEntryValue (0x3006,0x002a);
	printf ("RRN = [%s], RDC = [%s]\n", referenced_roi_number.c_str(), roi_display_color.c_str());

	if (1 != sscanf (referenced_roi_number.c_str(), "%d", &structure_id)) {
	    printf ("Error parsing rrn...\n");
	    continue;
	}

	/* Look up the cxt structure for this id */
	curr_structure = cxt->find_structure_by_id (structure_id);
	if (!curr_structure) {
	    printf ("Couldn't reference structure with id %d\n", structure_id);
	    exit (-1);
	}
	curr_structure->set_color (roi_display_color.c_str());

	/* ContourSequence */
	c_seq = item->GetSeqEntry (0x3006,0x0040);
	if (c_seq) {
	    for (c_item = c_seq->GetFirstSQItem (); c_item; c_item = c_seq->GetNextSQItem ()) {
		int i, p, n, contour_data_len;
		int num_points;
		std::string contour_geometric_type;
		std::string contour_data;
		std::string number_of_contour_points;
		Rtss_contour *curr_polyline;

		/* Grab data from dicom */
		contour_geometric_type = c_item->GetEntryValue (0x3006,0x0042);
		if (strncmp (contour_geometric_type.c_str(), "CLOSED_PLANAR", strlen("CLOSED_PLANAR"))) {
		    /* Might be "POINT".  Do I want to preserve this? */
		    printf ("Skipping geometric type: [%s]\n", contour_geometric_type.c_str());
		    continue;
		}
		number_of_contour_points = c_item->GetEntryValue (0x3006,0x0046);
		if (1 != sscanf (number_of_contour_points.c_str(), "%d", &num_points)) {
		    printf ("Error parsing number_of_contour_points...\n");
		    continue;
		}
		if (num_points <= 0) {
		    /* Polyline with zero points?  Skip it. */
		    continue;
		}
		contour_data = c_item->GetEntryValue (0x3006,0x0050);
		if (contour_data == gdcm::GDCM_UNFOUND) {
		    printf ("Error grabbing contour data.\n");
		    continue;
		}

		/* Create a new polyline for this structure */
		curr_polyline = curr_structure->add_polyline ();
		curr_polyline->slice_no = -1;
		//curr_polyline->ct_slice_uid = "";
		curr_polyline->num_vertices = num_points;
		curr_polyline->x = (float*) malloc (num_points * sizeof(float));
		curr_polyline->y = (float*) malloc (num_points * sizeof(float));
		curr_polyline->z = (float*) malloc (num_points * sizeof(float));

		/* Parse dicom data string */
		i = 0;
		n = 0;
		contour_data_len = strlen (contour_data.c_str());
		for (p = 0; p < 3 * num_points; p++) {
		    float f;
		    int this_n;
		
		    /* Skip \\ */
		    if (n < contour_data_len) {
			if (contour_data.c_str()[n] == '\\') {
			    n++;
			}
		    }

		    /* Parse float value */
		    if (1 != sscanf (&contour_data[n], "%f%n", &f, &this_n)) {
			printf ("Error parsing data...\n");
                        printf ("%d points\n", num_points);
                        printf ("%s\n", contour_data.c_str());
			break;
		    }
		    n += this_n;

		    /* Put value into polyline */
		    switch (i) {
		    case 0:
			curr_polyline->x[p/3] = f;
			break;
		    case 1:
			curr_polyline->y[p/3] = f;
			break;
		    case 2:
			curr_polyline->z[p/3] = f;
			break;
		    }
		    i = (i + 1) % 3;
		}
	    }
	}
    }
    printf ("Loading complete.\n");
    delete rtss_file;
}

/* GCS: I had to copy from gdcm::Document because the function is protected. */
int
plm_ComputeGroup0002Length (gdcm::File *gf)
{
    uint16_t gr;
    std::string vr;
   
    int groupLength = 0;
    bool found0002 = false;   
  
    // for each zero-level Tag in the DCM Header
    gdcm::DocEntry *entry = gf->GetFirstEntry();
    while( entry )
    {
	gr = entry->GetGroup();

	if ( gr == 0x0002 )
	{
	    found0002 = true;

	    if ( entry->GetElement() != 0x0000 )
	    {
		vr = entry->GetVR();

		//if ( (vr == "OB")||(vr == "OW")||(vr == "UT")||(vr == "SQ"))
		// (no SQ, OW, UT in group 0x0002;)
		if ( vr == "OB" ) 
		{
		    // explicit VR AND (OB, OW, SQ, UT) : 4 more bytes
		    groupLength +=  4;
		}
		groupLength += 2 + 2 + 4 + entry->GetLength();   
	    }
	}
	else if (found0002 )
	    break;

	entry = gf->GetNextEntry();
    }
    return groupLength; 
}

void
gdcm_rtss_save (
    Rtss *cxt,   /* Input: this is what gets saved */
    const Rt_study_metadata::Pointer& rsm,    /* In: save this too */
    const char *rtss_fn   /* Input: name of file to write to */
)
{
    int k;
    gdcm::File *gf = new gdcm::File ();
    const std::string &current_date = gdcm::Util::GetCurrentDate();
    const std::string &current_time = gdcm::Util::GetCurrentTime();

    printf ("Hello from gdcm_rtss_save (%s)\n", rtss_fn);

    /* Due to a bug in gdcm, it is not possible to create a gdcmFile 
       which does not have a (7fe0,0000) PixelDataGroupLength element.
       Therefore we have to write using Document::WriteContent() */
    make_parent_directories (rtss_fn);
    std::ofstream *fp;
    fp = new std::ofstream (rtss_fn, std::ios::out | std::ios::binary);
    if (*fp == NULL) {
	fprintf (stderr, "Error opening file for write: %s\n", rtss_fn);
	return;
    }
    
    /* ----------------------------------------------------------------- */
    /*     Part 1  -- General header                                     */
    /* ----------------------------------------------------------------- */

    /* FIX: DICOM file meta information cannot be stored when writing with
       Document::WriteContent(). So there's no TransferSyntaxUID etc.
       We need a better workaround for the gdcm bug. */

    Metadata::Pointer& meta = rsm->get_study_metadata();

    /* InstanceCreationDate */
    gf->InsertValEntry (current_date, 0x0008, 0x0012);
    /* InstanceCreationTime */
    gf->InsertValEntry (current_time, 0x0008, 0x0013);
    /* InstanceCreatorUID */
    gf->InsertValEntry (PLM_UID_PREFIX, 0x0008, 0x0014);
    /* SOPClassUID = RTStructureSetStorage */
    gf->InsertValEntry ("1.2.840.10008.5.1.4.1.1.481.3", 0x0008, 0x0016);
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
    gf->InsertValEntry ("RTSTRUCT", 0x0008, 0x0060);
    /* Manufacturer */
    gf->InsertValEntry ("Plastimatch", 0x0008, 0x0070);
    /* InstitutionName */
    gf->InsertValEntry ("", 0x0008, 0x0080);
    /* ReferringPhysiciansName */
    gf->InsertValEntry ("", 0x0008, 0x0090);
    /* StationName */
    gf->InsertValEntry ("", 0x0008, 0x1010);
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
    /* SoftwareVersions */
    gf->InsertValEntry (PLASTIMATCH_VERSION_STRING, 0x0018, 0x1020);
    /* PatientPosition */
    // gf->InsertValEntry (xxx, 0x0018, 0x5100);
    /* StudyInstanceUID */
    gf->InsertValEntry ((const char*) rsm->get_study_uid(), 0x0020, 0x000d);
    /* SeriesInstanceUID */
    gf->InsertValEntry (gdcm::Util::CreateUniqueUID (PLM_UID_PREFIX), 
	0x0020, 0x000e);
    /* StudyID */
    set_gdcm_file_from_metadata (gf, meta, 0x0020, 0x0010);
    /* SeriesNumber */
    gf->InsertValEntry ("103", 0x0020, 0x0011);
    /* InstanceNumber */
    gf->InsertValEntry ("1", 0x0020, 0x0013);
    /* StructureSetLabel */
    gf->InsertValEntry ("AutoSS", 0x3006, 0x0002);
    /* StructureSetName */
    gf->InsertValEntry ("AutoSS", 0x3006, 0x0004);
    /* StructureSetDate */
    gf->InsertValEntry (current_date, 0x3006, 0x0008);
    /* StructureSetTime */
    gf->InsertValEntry (current_time, 0x3006, 0x0009);

    /* ----------------------------------------------------------------- */
    /*     Part 2  -- UID's for CT series                                */
    /* ----------------------------------------------------------------- */

    /* ReferencedFrameOfReferenceSequence */
    gdcm::SeqEntry *rfor_seq = gf->InsertSeqEntry (0x3006, 0x0010);
    gdcm::SQItem *rfor_item = new gdcm::SQItem (rfor_seq->GetDepthLevel());
    rfor_seq->AddSQItem (rfor_item, 1);
    /* FrameOfReferenceUID */
    rfor_item->InsertValEntry (rsm->get_frame_of_reference_uid(),
	0x0020, 0x0052);
    /* RTReferencedStudySequence */
    gdcm::SeqEntry *rtrstudy_seq = rfor_item->InsertSeqEntry (0x3006, 0x0012);
    gdcm::SQItem *rtrstudy_item 
	= new gdcm::SQItem (rtrstudy_seq->GetDepthLevel());
    rtrstudy_seq->AddSQItem (rtrstudy_item, 1);
    /* ReferencedSOPClassUID = DetachedStudyManagementSOPClass */
    rtrstudy_item->InsertValEntry ("1.2.840.10008.3.1.2.3.1", 0x0008, 0x1150);
    /* ReferencedSOPInstanceUID */
    rtrstudy_item->InsertValEntry (rsm->get_study_uid(),
	0x0008, 0x1155);
    /* RTReferencedSeriesSequence */
    gdcm::SeqEntry *rtrseries_seq 
	= rtrstudy_item->InsertSeqEntry (0x3006, 0x0014);
    gdcm::SQItem *rtrseries_item 
	= new gdcm::SQItem (rtrseries_seq->GetDepthLevel());
    rtrseries_seq->AddSQItem (rtrseries_item, 1);
    /* SeriesInstanceUID */
    rtrseries_item->InsertValEntry (rsm->get_ct_series_uid(),
	0x0020, 0x000e);
    /* ContourImageSequence */
    gdcm::SeqEntry *ci_seq = rtrseries_item->InsertSeqEntry (0x3006, 0x0016);
    if (!rsm->slice_list_complete()) {
	printf ("Warning: CT UIDs not found. "
	    "ContourImageSequence not generated.\n");
    }
    for (int slice = 0, sqi = 1; slice < rsm->num_slices(); slice++, sqi++) {
	/* Get SOPInstanceUID of CT slice */
	std::string tmp = rsm->get_slice_uid (slice);
	/* Put item into sequence */
	gdcm::SQItem *ci_item = new gdcm::SQItem (ci_seq->GetDepthLevel());
	ci_seq->AddSQItem (ci_item, sqi++);
	/* ReferencedSOPClassUID = CTImageStorage */
	ci_item->InsertValEntry ("1.2.840.10008.5.1.4.1.1.2", 
	    0x0008, 0x1150);
	/* Put ReferencedSOPInstanceUID into item */
	ci_item->InsertValEntry (tmp, 0x0008, 0x1155);
    }

    /* ----------------------------------------------------------------- */
    /*     Part 3  -- Structure info                                     */
    /* ----------------------------------------------------------------- */

    /* StructureSetROISequence */
    gdcm::SeqEntry *ssroi_seq = gf->InsertSeqEntry (0x3006, 0x0020);
    for (size_t i = 0; i < cxt->num_structures; i++) {
	gdcm::SQItem *ssroi_item 
	    = new gdcm::SQItem (ssroi_seq->GetDepthLevel());
	ssroi_seq->AddSQItem (ssroi_item, i+1);
	/* ROINumber */
	ssroi_item->InsertValEntry (gdcm::Util::Format 
	    ("%d", cxt->slist[i]->id),
	    0x3006, 0x0022);
	/* ReferencedFrameOfReferenceUID */
	ssroi_item->InsertValEntry (
            rsm->get_frame_of_reference_uid(), 0x3006, 0x0024);
	/* ROIName */
	ssroi_item->InsertValEntry (
	    cxt->slist[i]->name.c_str(), 0x3006, 0x0026);
	/* ROIGenerationAlgorithm */
	ssroi_item->InsertValEntry ("", 0x3006, 0x0036);
    }

    /* ----------------------------------------------------------------- */
    /*     Part 4  -- Contour info                                       */
    /* ----------------------------------------------------------------- */

    /* ROIContourSequence */
    gdcm::SeqEntry *roic_seq = gf->InsertSeqEntry (0x3006, 0x0039);
    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_roi *curr_structure = cxt->slist[i];
	gdcm::SQItem *roic_item 
	    = new gdcm::SQItem (roic_seq->GetDepthLevel());
	roic_seq->AddSQItem (roic_item, i+1);
	
	/* ROIDisplayColor */
        std::string dcm_color;
	dcm_color = curr_structure->get_dcm_color_string ();
	roic_item->InsertValEntry (dcm_color.c_str(), 0x3006, 0x002a);

	/* ContourSequence */
	gdcm::SeqEntry *c_seq = roic_item->InsertSeqEntry (0x3006, 0x0040);
	for (size_t j = 0; j < curr_structure->num_contours; j++) {
	    Rtss_contour *curr_contour = curr_structure->pslist[j];
	    if (curr_contour->num_vertices <= 0) continue;

#if defined (commentout)
            /* GCS 2013-07-02:  DICOM standard allows contours without 
               an associated slice UID.  Maybe this bug is now 
               fixed in XiO??? */
	    /* GE -> XiO transfer does not work if contour does not have 
	       corresponding slice uid */
	    if (curr_contour->ct_slice_uid.empty()) {
		printf ("Warning: Omitting contour (%ld,%ld)\n", 
                    (long) i, (long) j);
		continue;
	    }
#endif

            /* Add item to ContourSequence */
            gdcm::SQItem *c_item = new gdcm::SQItem (
                c_seq->GetDepthLevel());
            c_seq->AddSQItem (c_item, j+1);

            /* ContourImageSequence */
            if (curr_contour->ct_slice_uid != "") {
                gdcm::SeqEntry *ci_seq 
                    = c_item->InsertSeqEntry (0x3006, 0x0016);
                gdcm::SQItem *ci_item 
                    = new gdcm::SQItem (ci_seq->GetDepthLevel());
                ci_seq->AddSQItem (ci_item, 1);
                /* ReferencedSOPClassUID = CTImageStorage */
                ci_item->InsertValEntry ("1.2.840.10008.5.1.4.1.1.2", 
                    0x0008, 0x1150);
                /* ReferencedSOPInstanceUID */
                ci_item->InsertValEntry (
                    curr_contour->ct_slice_uid.c_str(),
                    0x0008, 0x1155);
            }

	    /* ContourGeometricType */
	    c_item->InsertValEntry ("CLOSED_PLANAR", 0x3006, 0x0042);
	    /* NumberOfContourPoints */
	    c_item->InsertValEntry (gdcm::Util::Format 
		("%d", curr_contour->num_vertices),
		0x3006, 0x0046);
	    /* ContourData */
	    std::string contour_string 
		= gdcm::Util::Format ("%g\\%g\\%g",
		    curr_contour->x[0],
		    curr_contour->y[0],
		    curr_contour->z[0]);
	    for (k = 1; k < curr_contour->num_vertices; k++) {
		contour_string += gdcm::Util::Format ("\\%g\\%g\\%g",
		    curr_contour->x[k],
		    curr_contour->y[k],
		    curr_contour->z[k]);
	    }
	    c_item->InsertValEntry (contour_string, 0x3006, 0x0050);
	}
	/* ReferencedROINumber */
	roic_item->InsertValEntry (gdcm::Util::Format 
	    ("%d", curr_structure->id),
	    0x3006, 0x0084);
    }

    /* ----------------------------------------------------------------- */
    /*     Part 5  -- More structure info                                */
    /* ----------------------------------------------------------------- */

    /* RTROIObservationsSequence */
    gdcm::SeqEntry *rtroio_seq = gf->InsertSeqEntry (0x3006, 0x0080);
    for (size_t i = 0; i < cxt->num_structures; i++) {
	Rtss_roi *curr_structure = cxt->slist[i];
	gdcm::SQItem *rtroio_item 
	    = new gdcm::SQItem (rtroio_seq->GetDepthLevel());
	rtroio_seq->AddSQItem (rtroio_item, i+1);
	/* ObservationNumber */
	rtroio_item->InsertValEntry (gdcm::Util::Format 
	    ("%d", curr_structure->id),
	    0x3006, 0x0082);
	/* ReferencedROINumber */
	rtroio_item->InsertValEntry (gdcm::Util::Format 
	    ("%d", curr_structure->id),
	    0x3006, 0x0084);
	/* ROIObservationLabel */
        if (curr_structure->name.length() <= 16) {
            rtroio_item->InsertValEntry (
                curr_structure->name.c_str(), 0x3006, 0x0085);
        } else {
            /* VR is SH, max length 16 */
            std::string tmp_name = curr_structure->name.substr (0, 16);
            rtroio_item->InsertValEntry (
                tmp_name.c_str(), 0x3006, 0x0085);
        }
	/* RTROIInterpretedType */
	rtroio_item->InsertValEntry ("", 0x3006, 0x00a4);
	/* ROIInterpreter */
	rtroio_item->InsertValEntry ("", 0x3006, 0x00a6);
    }

    /* Create DICOM meta-information header -- gdcm suxxors :P */
    gf->InsertValEntry ("0", 0x0002, 0x0000);
    uint8_t fmiv[2] = { 0x00, 0x01 };
    gf->InsertBinEntry (fmiv, 2, 0x0002, 0x0001, std::string("OB"));
    gf->InsertValEntry ("1.2.840.10008.5.1.4.1.1.481.3", 0x0002, 0x0002);
    gf->InsertValEntry (gf->GetEntryValue (0x0008, 0x0018), 0x0002, 0x0003);
    // Implicit VR Little Endian = "1.2.840.10008.1.2"
    // Explicit VR Little Endian = "1.2.840.10008.1.2.1"
    gf->InsertValEntry ("1.2.840.10008.1.2.1", 0x0002, 0x0010);
    gf->InsertValEntry (std::string (PLM_UID_PREFIX) + ".101" , 
	0x0002, 0x0012);
    /* NB: (0002,0013) only allows up to 16 characters */
    gf->InsertValEntry (std::string("Plastimatch 1.4"), 0x0002, 0x0013);

    /* Calculate size of meta-information header */
    /* GCS: I copied this from gdcm::File::Write */
    gdcm::ValEntry *e0000 = gf->GetValEntry (0x0002,0x0000);
    if (e0000) {
	itksys_ios::ostringstream sLen;
	sLen << plm_ComputeGroup0002Length (gf);
	e0000->SetValue(sLen.str());
    }

    /* Do the actual writing out to file */
    gf->WriteContent (fp, gdcm::ExplicitVR);
    fp->close();
    delete fp;
}
