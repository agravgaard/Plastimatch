/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_stats_h_
#define _volume_stats_h_

#include "plmbase_config.h"

class Volume;

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
PLMBASE_API void 
volume_stats (
    const Volume *vol,
    double *min_val,
    double *max_val, 
    double *avg,
    int *non_zero,
    int *num_vox
);
PLMBASE_API void 
volume_stats (
    const Volume::Pointer vol,
    double *min_val,
    double *max_val, 
    double *avg,
    int *non_zero,
    int *num_vox
);

#endif
