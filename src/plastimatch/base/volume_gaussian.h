/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_gaussian_h_
#define _volume_gaussian_h_

#include "plmbase_config.h"

PLMBASE_API Volume::Pointer 
volume_gaussian (
    const Volume::Pointer& vol_in,
    float width,
    float truncation
);

#endif
