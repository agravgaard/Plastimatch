/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _landmark_diff_h_
#define _landmark_diff_h_

#include "plmutil_config.h"

typedef struct raw_pointset Raw_pointset;

PLMUTIL_C_API int landmark_diff (
    Raw_pointset *rps0,
    Raw_pointset *rps1
);

#endif /* _landmark_diff_h_ */
