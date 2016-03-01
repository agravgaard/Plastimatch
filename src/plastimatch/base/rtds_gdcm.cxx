/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

#if PLM_DCM_USE_GDCM1
#include "gdcm1_dose.h"
#include "gdcm1_series.h"
#endif
#include "plm_image.h"
#include "rt_study.h"
#include "rt_study_p.h"
#include "segmentation.h"
#include "string_util.h"

void
Rt_study::load_gdcm (const char *dicom_dir)
{
    if (!dicom_dir) {
	return;
    }

#if PLM_DCM_USE_GDCM1
    Gdcm_series *gdcm_series = new Gdcm_series;
    gdcm_series->load (dicom_dir);
    gdcm_series->digest_files ();

     if (gdcm_series->m_rtdose_file_list) {
	const std::string& filename = gdcm_series->get_rtdose_filename();
	d_ptr->m_dose.reset(gdcm1_dose_load (0, filename.c_str()));
    }
    if (gdcm_series->m_rtstruct_file_list) {
	const std::string& filename = gdcm_series->get_rtstruct_filename();
	d_ptr->m_rtss = Segmentation::New ();
	d_ptr->m_rtss->load_gdcm_rtss (filename.c_str(), d_ptr->m_drs.get());
    }
#endif

    /* Use existing itk reader for the image.
       This is required because the native dicom reader doesn't yet 
       handle things like MR. */
    d_ptr->m_img = Plm_image::New (new Plm_image(dicom_dir));

#if PLM_DCM_USE_GDCM1
    /* Use native reader to set meta */
    gdcm_series->get_metadata (this->get_metadata ());
    delete gdcm_series;
#endif
}

void
Rt_study::save_gdcm (const char *output_dir)
{
    if (d_ptr->m_img) {
	printf ("Rt_study::save_dicom: save_short_dicom()\n");
	d_ptr->m_img->save_short_dicom (output_dir, d_ptr->m_drs.get());
    }
#if PLM_DCM_USE_GDCM1
    if (d_ptr->m_rtss) {
	printf ("Rt_study::save_dicom: save_gdcm_rtss()\n");
	d_ptr->m_rtss->save_gdcm_rtss (output_dir, d_ptr->m_drs);
    }
    if (this->has_dose()) {
        std::string fn;
	printf ("Rt_study::save_dicom: gdcm_save_dose()\n");
	fn = string_format ("%s/%s", output_dir, "dose.dcm");
	gdcm1_dose_save (d_ptr->m_dose.get(), d_ptr->m_drs.get(), fn.c_str());
    }
#endif
}
