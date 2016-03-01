/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_stats_h_
#define _itk_image_stats_h_

#include "plmbase_config.h"
#include "itk_image.h"

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
template<class T> PLMBASE_API void itk_image_stats (
        T img,
        double *min_val,
        double *max_val, 
        double *avg,
        int *non_zero,
        int *num_vox
);

#endif
