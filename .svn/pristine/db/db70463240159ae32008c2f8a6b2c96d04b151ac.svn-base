/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dicom_util.h"
#include "make_string.h"

#if PLM_DCM_USE_DCMTK
#include "dcmtk_rdd.h"
#include "dcmtk_uid.h"
#include "dcmtk_util.h"
#elif PLM_DCM_USE_GDCM1
#include "gdcm1_rdd.h"
#include "gdcm1_util.h"
#else
#include "gdcm2_util.h"
#endif
#include "plm_uid_prefix.h"

void
dicom_get_date_time (
    std::string *date,
    std::string *time
)
{
#if PLM_DCM_USE_DCMTK
    dcmtk_get_date_time (date, time);
#elif PLM_DCM_USE_GDCM1
    gdcm1_get_date_time (date, time);
#else
    gdcm2_get_date_time (date, time);
#endif
}

std::string 
dicom_anon_patient_id (void)
{
    int i;
    unsigned char uuid[16];
    std::string patient_id = "PL";

    srand (time (0));
    for (i = 0; i < 16; i++) {
       int r = (int) (10.0 * rand() / RAND_MAX);
       uuid[i] = '0' + r;
    }
    uuid [15] = '\0';
    patient_id = patient_id + make_string (uuid);
    return patient_id;
}

void
dicom_load_rdd (Rt_study_metadata::Pointer rsm, const char* dicom_dir)
{
#if PLM_DCM_USE_DCMTK
    dcmtk_load_rdd (rsm, dicom_dir);
#elif PLM_DCM_USE_GDCM1
    gdcm1_load_rdd (rsm.get(), dicom_dir);
#else
    /* Do nothing */
#endif
}

char*
dicom_uid (char *uid, const char *uid_root)
{
#if PLM_DCM_USE_DCMTK
    return dcmtk_uid (uid, uid_root);
#else
    return gdcm_uid (uid, uid_root);
#endif
}

std::string
dicom_uid (const char *uid_root)
{
    char uid[100];
    dicom_uid (uid, uid_root);
    return std::string (uid);
}

std::string
dicom_uid ()
{
    return dicom_uid (PLM_UID_PREFIX);
}
