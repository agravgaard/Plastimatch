/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_rt_study.h"
#include "dcmtk_rt_study_p.h"
#include "dcmtk_rtss.h"
#include "dcmtk_series.h"
#include "dcmtk_slice_data.h"
#include "dicom_util.h"
#include "file_util.h"
#include "logfile.h"
#include "path_util.h"
#include "plm_image.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "rt_study_metadata.h"
#include "rtss.h"
#include "smart_pointer.h"
#include "volume.h"

Dcmtk_rt_study::Dcmtk_rt_study ()
{
    this->d_ptr = new Dcmtk_rt_study_private;

    /* GCS FIX: Need a way to turn this on via configuration. 
       But for now, just unilaterally disable logging. 
       http://support.dcmtk.org/wiki/dcmtk/howto/logprogram */
    OFLog::configure (OFLogger::FATAL_LOG_LEVEL);
}

Dcmtk_rt_study::Dcmtk_rt_study (const char* dicom_path)
{
    this->d_ptr = new Dcmtk_rt_study_private;

    /* GCS FIX: Need a way to turn this on via configuration. 
       But for now, just unilaterally disable logging. 
       http://support.dcmtk.org/wiki/dcmtk/howto/logprogram */
    OFLog::configure (OFLogger::FATAL_LOG_LEVEL);

    this->load (dicom_path);
}

Dcmtk_rt_study::~Dcmtk_rt_study ()
{
    delete this->d_ptr;
}

const char*
Dcmtk_rt_study::get_ct_series_uid () const
{
    return d_ptr->ct_series_uid;
}

const char*
Dcmtk_rt_study::get_dose_instance_uid () const
{
    return d_ptr->dose_instance_uid;
}

const char*
Dcmtk_rt_study::get_dose_series_uid () const
{
    return d_ptr->dose_series_uid;
}

const char*
Dcmtk_rt_study::get_frame_of_reference_uid () const
{
    return d_ptr->for_uid;
}

const char*
Dcmtk_rt_study::get_plan_instance_uid () const
{
    return d_ptr->plan_instance_uid;
}

const char*
Dcmtk_rt_study::get_rtss_instance_uid () const
{
    return d_ptr->rtss_instance_uid;
}

const char*
Dcmtk_rt_study::get_rtss_series_uid () const
{
    return d_ptr->rtss_series_uid;
}

const char*
Dcmtk_rt_study::get_study_date () const
{
    return d_ptr->date_string.c_str();
}

const char*
Dcmtk_rt_study::get_study_time () const
{
    return d_ptr->time_string.c_str();
}

const char*
Dcmtk_rt_study::get_study_uid () const
{
    return d_ptr->study_uid;
}

std::vector<Dcmtk_slice_data>* 
Dcmtk_rt_study::get_slice_data ()
{
    return d_ptr->slice_data;
}

Plm_image::Pointer&
Dcmtk_rt_study::get_image ()
{
    return d_ptr->img;
}

Volume::Pointer
Dcmtk_rt_study::get_image_volume_float ()
{
    return d_ptr->img->get_volume_float ();
}

void 
Dcmtk_rt_study::set_image (const Plm_image::Pointer& image)
{
    d_ptr->img = image;
}

Rtss::Pointer&
Dcmtk_rt_study::get_rtss ()
{
    return d_ptr->rtss;
}

void 
Dcmtk_rt_study::set_rtss (const Rtss::Pointer& rtss)
{
    d_ptr->rtss = rtss;
}

Rtplan::Pointer&
Dcmtk_rt_study::get_rtplan()
{
    return d_ptr->rtplan;
}

void
Dcmtk_rt_study::set_rtplan (const Rtplan::Pointer& rtplan)
{
    d_ptr->rtplan = rtplan;
}

Plm_image::Pointer&
Dcmtk_rt_study::get_dose ()
{
    return d_ptr->dose;
}

void 
Dcmtk_rt_study::set_dose (const Plm_image::Pointer& image)
{
    d_ptr->dose = image;
}

void 
Dcmtk_rt_study::set_rt_study_metadata (
    const Rt_study_metadata::Pointer& rt_study_metadata)
{
    d_ptr->rt_study_metadata = rt_study_metadata;
}

void 
Dcmtk_rt_study::set_filenames_with_uid (bool filenames_with_uid)
{
    d_ptr->filenames_with_uid = filenames_with_uid;
}

void 
Dcmtk_rt_study::load (const char *dicom_path)
{
    if (is_directory (dicom_path)) {
        this->insert_directory (dicom_path);
    } else {
        this->insert_file (dicom_path);
    }
    this->parse_directory ();
}

void 
Dcmtk_rt_study::save (const char *dicom_dir)
{
    /* GCS FIX: If we're writing an image, we always want new metadata;
       but this should probably be handled by somewhere else in the 
       code. 
       GCS FIX MORE: Slicer-RT retains the option to set the study UID.
       There is, I think (?!), no need to regenerate those at this time.
    */
    if (d_ptr->img) {
        d_ptr->rt_study_metadata->generate_new_series_uids ();
    }
    if (d_ptr->img) {
        this->save_image (dicom_dir);
    }
    if (d_ptr->rtss) {
        this->save_rtss (dicom_dir);
    }
    if (d_ptr->dose) {
        this->save_dose (dicom_dir);
    }
    if (d_ptr->rtplan) {
        this->save_rtplan (dicom_dir);
    }
}

void
Dcmtk_rt_study::insert_file (const char* fn)
{
    Dcmtk_file::Pointer df = Dcmtk_file::New (fn);

    /* Discard non-dicom files */
    if (!df->is_valid()) {
        return;
    }

    /* Get the SeriesInstanceUID */
    const char *c = NULL;
    std::string series_uid;
    c = df->get_cstr (DCM_SeriesInstanceUID);
    if (c) {
        series_uid = std::string (c);
    } else {
	/* 2014-12-17.  Oncentra data missing SeriesInstanceUID? 
           If that happens, make something up. */
        series_uid = dicom_uid ();
    }

    /* Look for the SeriesInstanceUID in the map */
    Dcmtk_series_map::iterator it;
    it = d_ptr->m_smap.find (series_uid);

    /* If we didn't find the UID, add a new entry into the map */
    if (it == d_ptr->m_smap.end()) {
	std::pair<Dcmtk_series_map::iterator,bool> ret 
	    = d_ptr->m_smap.insert (Dcmtk_series_map_pair (series_uid, 
		    new Dcmtk_series()));
	if (ret.second == false) {
	    print_and_exit (
		"Error inserting UID %s into dcmtk_series_map.\n", c);
	}
	it = ret.first;
    }

    /* Add the file to the Dcmtk_series object for this UID */
    Dcmtk_series *ds = (*it).second;
    ds->insert (df);
}

void
Dcmtk_rt_study::insert_directory (const char* dir)
{
    OFBool recurse = OFFalse;
    OFList<OFString> input_files;

    /* On windows, searchDirectoryRecursively doesn't work 
       if the path is like c:/dir/dir; instead it must be c:\dir\dir */
    std::string fixed_path = make_windows_slashes (std::string(dir));

    OFStandard::searchDirectoryRecursively (
	fixed_path.c_str(), input_files, "", "", recurse);

    OFListIterator(OFString) if_iter = input_files.begin();
    OFListIterator(OFString) if_last = input_files.end();
    while (if_iter != if_last) {
	const char *current = (*if_iter++).c_str();
	this->insert_file (current);
    }
}

void
Dcmtk_rt_study::sort_all (void) 
{
    Dcmtk_series_map::iterator it;
    for (it = d_ptr->m_smap.begin(); it != d_ptr->m_smap.end(); ++it) {
	const std::string& key = (*it).first;
	Dcmtk_series *ds = (*it).second;
	UNUSED_VARIABLE (key);
	ds->sort ();
    }
}

void
Dcmtk_rt_study::debug (void) const
{
    Dcmtk_series_map::const_iterator it;
    for (it = d_ptr->m_smap.begin(); it != d_ptr->m_smap.end(); ++it) {
	const std::string& key = (*it).first;
	const Dcmtk_series *ds = (*it).second;
	UNUSED_VARIABLE (key);
	UNUSED_VARIABLE (ds);
	ds->debug ();
    }
}

Volume *
Dcmtk_rt_study::get_volume ()
{
    if (!d_ptr->img) {
        this->parse_directory ();
    }
    if (!d_ptr->img) {
        return 0;
    }
    return d_ptr->img->get_vol();
}

void
Dcmtk_rt_study::parse_directory (void)
{
    Dcmtk_series_map::iterator it;
    d_ptr->ds_image = 0;
    d_ptr->ds_rtss = 0;
    d_ptr->ds_rtdose = 0;
    d_ptr->ds_rtplan = 0;

    /* Loop through all series in directory, and find image, ss, dose */
    size_t best_image_slices = 0;
    for (it = d_ptr->m_smap.begin(); it != d_ptr->m_smap.end(); ++it) {
	const std::string& key = (*it).first;
	Dcmtk_series *ds = (*it).second;
	UNUSED_VARIABLE (key);

	/* Check for rtstruct */
	if (!d_ptr->ds_rtss && ds->get_modality() == "RTSTRUCT") {
	    printf ("Found RTSTUCT, UID=%s\n", key.c_str());
	    d_ptr->ds_rtss = ds;
	    continue;
	}

	/* Check for rtdose */
	if (!d_ptr->ds_rtdose && ds->get_modality() == "RTDOSE") {
	    printf ("Found RTDOSE, UID=%s\n", key.c_str());
	    d_ptr->ds_rtdose = ds;
	    continue;
	}

        /* Check for rtplan */
        if (!d_ptr->ds_rtplan && ds->get_modality() == "RTPLAN") {
            printf("Found RTPLAN, UID=%s\n", key.c_str());
            d_ptr->ds_rtplan = ds;
            continue;
        }

	/* Check for image.  An image is anything with a PixelData.
           Current heuristic: load the image with the most slices
           (as determined by the number of files) */
	bool rc = ds->get_uint16_array (DCM_PixelData, 0, 0);
        if (rc) {
            size_t num_slices = ds->get_number_of_files ();
            if (num_slices > best_image_slices) {
                best_image_slices = num_slices;
                d_ptr->ds_image = ds;
            }
	    continue;
	}
    }

    /* GCS FIX: need additional logic that checks if ss & dose 
       refer to the image.  The below logic doesn't do anything. */
    std::string referenced_uid = "";
    if (d_ptr->ds_rtss) {
	referenced_uid = d_ptr->ds_rtss->get_referenced_uid ();
    }

    /* Load image */
    if (d_ptr->ds_image) {
        d_ptr->ds_image->set_rt_study_metadata (d_ptr->rt_study_metadata);
        this->image_load ();
    }

    /* Load rtss */
    if (d_ptr->ds_rtss) {
        this->rtss_load ();
    }

    /* Load dose */
    if (d_ptr->ds_rtdose) {
        this->rtdose_load ();
    }

    /* Load plan */
    if (d_ptr->ds_rtplan) {        
        this->rtplan_load();
    }
}

