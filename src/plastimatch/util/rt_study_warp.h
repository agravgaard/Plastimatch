/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_study_warp_h_
#define _rt_study_warp_h_

#include "plmutil_config.h"
#include "base/plm_file_format.h"

class Rt_study;
class Warp_parms;

PLMUTIL_API void rt_study_warp (
    Rt_study *rtds, Plm_file_format file_type, Warp_parms *parms);

#endif
