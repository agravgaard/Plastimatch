/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcm_util_h_
#define _dcm_util_h_

#include "plmbase_config.h"
#include <string>
#include "rt_study_metadata.h"

class Rt_study_metadata;

std::string dicom_anon_patient_id (void);

void dicom_load_rdd (Rt_study_metadata::Pointer rsm, const char* dicom_dir);

PLMBASE_C_API void 
dicom_get_date_time (
    std::string *date,
    std::string *time
);

PLMBASE_C_API char*
dicom_uid (char *uid, const char *uid_root);

PLMBASE_API std::string dicom_uid (const char *uid_root);
PLMBASE_API std::string dicom_uid ();

#endif
