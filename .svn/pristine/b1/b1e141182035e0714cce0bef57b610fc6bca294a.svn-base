/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_conv_h_
#define _volume_conv_h_

#include "plmbase_config.h"

PLMBASE_API Volume::Pointer 
volume_conv (
    const Volume::Pointer& vol_in,
    const Volume::Pointer& ker_in
);

PLMBASE_API 
Volume::Pointer
volume_convolve_separable
(
    const Volume::Pointer& vol_in,
    float *ker_i,
    int width_i,
    float *ker_j,
    int width_j,
    float *ker_k,
    int width_k
);

#endif
