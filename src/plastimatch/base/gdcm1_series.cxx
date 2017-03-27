/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmUtil.h"

#include "gdcm1_series.h"
#include "gdcm1_series_helper_2.h"
#include "metadata.h"
#include "plm_math.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "rt_study_metadata.h"

void
gdcm1_series_test (char *dicom_dir)
{
    Gdcm_series gs;

    gs.load (dicom_dir);
}

static void
digest_file_list (
    gdcm::FileList *file_list, 
    double origin[3], 
    int dim[3], 
    double spacing[3])
{
    int loop = 0;
    double prev_z = 0.0;

    // For all the files of a SingleSerieUID File set
    for (gdcm::FileList::iterator it =  file_list->begin();
	    it != file_list->end(); 
	    ++it)
    {
	if (loop == 0) {
	    spacing[0] = (*it)->GetXSpacing ();
	    spacing[1] = (*it)->GetYSpacing ();
	    origin[0] = (*it)->GetXOrigin ();
	    origin[1] = (*it)->GetYOrigin ();
	    prev_z = origin[2] = (*it)->GetZOrigin ();
	    dim[0] = (*it)->GetXSize ();
	    dim[1] = (*it)->GetYSize ();
	    loop ++;
	} else if (loop == 1) {
	    double z = (*it)->GetZOrigin ();
	    if (z - prev_z > 1e-5) {
		spacing[2] = z - origin[2];
		loop ++;
	    } else {
		printf ("Warning: duplicate slice locations (%g)\n", z);
	    }
	    prev_z = z;
	} else {
	    double z = (*it)->GetZOrigin ();
	    if (z - prev_z > 1e-5) {
		//printf (">> %g %g %g\n", z, prev_z, spacing[2]);
		/* XiO rounds IPP to nearest .1 mm */
		if (fabs (z - prev_z - spacing[2]) > 0.11) {
		    print_and_exit ("Error: irregular slice spacing in dicom series\n");
		}
		loop ++;
	    } else {
		print_and_exit ("Error: duplicate slice locations (%g)\n", z);
	    }
	    prev_z = z;
	}
    }
    dim[2] = loop;
}

Gdcm_series::Gdcm_series (void)
{
    this->m_gsh2 = 0;
    this->m_have_ct = 0;
    this->m_ct_file_list = 0;
    this->m_rtdose_file_list = 0;
    this->m_rtstruct_file_list = 0;
}

Gdcm_series::~Gdcm_series (void)
{
    if (this->m_gsh2) {
	delete this->m_gsh2;
    }
}

void
Gdcm_series::load (const char *dicom_dir)
{
    bool recursive = false;

    this->m_gsh2 = new gdcm::SerieHelper2();

    this->m_gsh2->Clear ();
    this->m_gsh2->SetUseSeriesDetails (true);

    // ---------------------------------------------------------------------
    // The below code is modified from CreateDefaultUniqueSeriesIdentifier.
    // If there was an API function called RemoveRestriction(), 
    // we could call CreateDefaultUniqueSeriesIdentifier() and then
    // call RemoveRestriction(0x0018, 0x0050).
    // ---------------------------------------------------------------------
    // 0020 0011 Series Number
    // A scout scan prior to a CT volume scan can share the same
    //   SeriesUID, but they will sometimes have a different Series Number
    this->m_gsh2->AddRestriction( 0x0020, 0x0011);
    // 0018 0024 Sequence Name
    // For T1-map and phase-contrast MRA, the different flip angles and
    //   directions are only distinguished by the Sequence Name
    this->m_gsh2->AddRestriction(0x0018, 0x0024);
    // 0018 0050 Slice Thickness
    // On some CT systems, scout scans and subsequence volume scans will
    //   have the same SeriesUID and Series Number - YET the slice 
    //   thickness will differ from the scout slice and the volume slices.
    // GCS: We don't want GDCM to use slice thickness to distinguish series.  
    //    CERR sets the slice thickness to different values within 
    //    a single series based on subtle differences in the Z position.
    // -- AddRestriction(0x0018, 0x0050); --
    // 0028 0010 Rows
    // If the 2D images in a sequence don't have the same number of rows,
    // then it is difficult to reconstruct them into a 3D volume.
    this->m_gsh2->AddRestriction(0x0028, 0x0010);
    // 0028 0011 Columns
    // If the 2D images in a sequence don't have the same number of columns,
    // then it is difficult to reconstruct them into a 3D volume.
    this->m_gsh2->AddRestriction(0x0028, 0x0011);

    this->m_gsh2->SetDirectory (dicom_dir, recursive);

#if defined (commentout)
    this->m_gsh2->Print ();
#endif

    gdcm::FileList *file_list = this->m_gsh2->GetFirstSingleSerieUIDFileSet ();
    while (file_list) {
	if (file_list->size()) {	
	    this->m_gsh2->OrderFileList (file_list);

#if defined (commentout)
	    /* Choose one file, and print the id */
	    gdcm::File *file = (*file_list)[0];
	    std::string id = this->m_gsh2->
		CreateUniqueSeriesIdentifier(file).c_str();
	    printf ("id = %s\n", id.c_str());
#endif
	}
	file_list = this->m_gsh2->GetNextSingleSerieUIDFileSet();
    }
}

void
Gdcm_series::digest_files (void)
{
    int d;
    for (d = 0; d < 3; d++) {
	this->m_origin[d] = 0.0;
	this->m_dim[d] = 0;
	this->m_spacing[d] = 0.0;
    }

    if (!this->m_gsh2) {
	return;
    }

    gdcm::FileList *file_list = this->m_gsh2->GetFirstSingleSerieUIDFileSet ();
    while (file_list) {
	if (file_list->size()) {	
	    this->m_gsh2->OrderFileList (file_list);

	    /* Get the USI */
	    gdcm::File *file = (*file_list)[0];
	    std::string id = this->m_gsh2->
		CreateUniqueSeriesIdentifier(file).c_str();

#if defined (commentout)
	    printf ("id = %s\n", id.c_str());
#endif

	    /* Is this a CT? */
	    std::string modality = file->GetEntryValue (0x0008, 0x0060);
	    if (modality == std::string ("CT")) {
		int dim[3] = {0, 0, 0};
		double origin[3] = {0., 0., 0.};
		double spacing[3] = {0., 0., 0.};
	    
		/* OK, I guess we have a CT */
		this->m_have_ct = 1;

		/* Digest the USI */
		digest_file_list (file_list, origin, dim, spacing);
#if defined (commentout)
		printf ("---- %s\n", id.c_str());
		printf ("DIM = %d %d %d\n", dim[0], dim[1], dim[2]);
		printf ("OFF = %g %g %g\n", origin[0], origin[1], origin[2]);
		printf ("SPA = %g %g %g\n", spacing[0], 
		    spacing[1], spacing[2]);
#endif
		
		/* Pick the CT with the largest dim[2] */
		if (dim[2] > this->m_dim[2]) {
		    this->m_ct_file_list = file_list;
		    for (d = 0; d < 3; d++) {
			this->m_origin[d] = origin[d];
			this->m_dim[d] = dim[d];
			this->m_spacing[d] = spacing[d];
		    }
		    
		}
	    }
	    else if (modality == std::string ("RTDOSE")) {
		printf ("Found RTDOSE!\n");
		this->m_rtdose_file_list = file_list;
	    }
	    else if (modality == std::string ("RTPLAN")) {
		//printf ("Found RTPLAN!\n");
	    }
	    else if (modality == std::string ("RTSTRUCT")) {
		printf ("Found RTSTRUCT!\n");
		this->m_rtstruct_file_list = file_list;
	    }
	    else {
		printf ("Found unknown modality %s\n", modality.c_str());
	    }
	}
	file_list = this->m_gsh2->GetNextSingleSerieUIDFileSet();
    }
}

void
Gdcm_series::get_slice_info (
    int *slice_no,                  /* Output */
    std::string *ct_slice_uid,      /* Output */
    float z                         /* Input */
)
{
    if (!this->m_have_ct) {
	return;
    }

    /* NOTE: This algorithm doesn't work if there are duplicate slices */
    *slice_no = ROUND_INT ((z - this->m_origin[2]) / this->m_spacing[2]);
    if (*slice_no < 0 || *slice_no >= this->m_dim[2]) {
	*slice_no = -1;
	return;
    }

    gdcm::File *file = (*this->m_ct_file_list)[*slice_no];
    if (!file) {
	print_and_exit ("Error finding slice %d in volume\n", *slice_no);
    }
    
    *ct_slice_uid = file->GetEntryValue (0x0008, 0x0018);
}

void
Gdcm_series::get_slice_uids (Rt_study_metadata *rsm)
{
    rsm->reset_slice_uids ();
    if (!this->m_have_ct) {
	return;
    }

    int i = 0;
    for (gdcm::FileList::iterator it = this->m_ct_file_list->begin();
	 it != this->m_ct_file_list->end(); 
	 ++it)
    {
	std::string slice_uid = (*it)->GetEntryValue (0x0008, 0x0018);
        rsm->set_slice_uid (i, slice_uid.c_str());
        i++;
    }
}

gdcm::File*
Gdcm_series::get_ct_slice (void)
{
    if (!this->m_have_ct) {
	return 0;
    }
    
    return (*this->m_ct_file_list)[0];
}

const std::string&
Gdcm_series::get_rtdose_filename ()
{
    gdcm::File *file = (*(m_rtdose_file_list))[0];
    return file->GetFileName();
}

const std::string&
Gdcm_series::get_rtstruct_filename ()
{
    gdcm::File *file = (*(m_rtstruct_file_list))[0];
    return file->GetFileName();
}

std::string
Gdcm_series::get_patient_position ()
{
    gdcm::File* file = this->get_ct_slice ();
    std::string tmp;

    /* Get patient position */
    tmp = file->GetEntryValue (0x0018, 0x5100);
    if (tmp == gdcm::GDCM_UNFOUND) {
	tmp = "";
    }

    return tmp;
}

void
Gdcm_series::get_metadata (const Metadata::Pointer& meta)
{
    if (m_have_ct) {
	gdcm::File *file = (*this->m_ct_file_list)[0];
#if defined (commentout)
	meta->m_patient_name = 
	    file->GetEntryValue(0x0010, 0x0010).c_str();
	meta->m_patient_id = 
	    file->GetEntryValue(0x0010, 0x0020).c_str();
	meta->m_patient_sex = 
	    file->GetEntryValue(0x0010, 0x0040).c_str();
#endif
	meta->set_metadata (
	    meta->make_key (0x0010, 0x0010),
	    file->GetEntryValue(0x0010, 0x0010));
	meta->set_metadata (
	    meta->make_key (0x0010, 0x0020),
	    file->GetEntryValue(0x0010, 0x0020));
	meta->set_metadata (
	    meta->make_key (0x0010, 0x0040),
	    file->GetEntryValue(0x0010, 0x0040));
    }
}
