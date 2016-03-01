/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _gdcm1_dose_h_
#define _gdcm1_dose_h_

#include "plmbase_config.h"

class Metadata;
class Plm_image;
class Rt_study_metadata;

#if PLM_DCM_USE_GDCM1
/* gdcm1_dose.cxx */
PLMBASE_C_API bool gdcm1_dose_probe (const char *dose_fn);
PLMBASE_C_API Plm_image* gdcm1_dose_load (
        Plm_image *pli,
        const char *dose_fn);
PLMBASE_C_API void gdcm1_dose_save (
        Plm_image *pli, 
        const Rt_study_metadata *rsm, 
        const char *dose_fn);

/* gdcm1_series.cxx */
PLMBASE_C_API void gdcm1_series_test (char *dicom_dir);

#endif

#endif /* _gdcm1_dose_h_ */
