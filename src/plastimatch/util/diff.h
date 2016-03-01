/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _diff_h_
#define _diff_h_

#include "plmutil_config.h"
#include <string>

class PLMUTIL_API Diff_parms {
public:
    std::string img_in_1_fn;
    std::string img_in_2_fn;
    std::string img_out_fn;
public:
    Diff_parms ();
};

PLMUTIL_API void diff_main (Diff_parms* parms);

#endif
