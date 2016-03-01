/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _gdcm1_rtss_h_
#define _gdcm1_rtss_h_

#include "plmbase_config.h"
#include "rt_study_metadata.h"

#if PLM_DCM_USE_GDCM1

class Metadata;
class Rtss;

PLMBASE_API bool gdcm_rtss_probe (const char *rtss_fn);
PLMBASE_API void gdcm_rtss_load (
    Rtss *cxt,                 /* Output: this gets loaded into */
    Rt_study_metadata *rsm,    /* Output: this gets updated too */
    const char *rtss_fn        /* Input: the file that gets read */
);
PLMBASE_API void gdcm_rtss_save (
    Rtss *cxt,                 /* Input: this is what gets saved */
    const Rt_study_metadata::Pointer& rsm, /* In: need to look at this too */
    const char *rtss_fn        /* Input: name of file to write to */
);

#endif
#endif
