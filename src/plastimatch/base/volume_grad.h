/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_grad_h_
#define _volume_grad_h_

#include "plmbase_config.h"

#include "volume.h"

PLMBASE_API Volume* volume_make_gradient (Volume* ref);
//PLMBASE_API Volume::Pointer volume_make_gradient (Volume* ref);
PLMBASE_API Volume::Pointer volume_gradient_magnitude (
    const Volume::Pointer& ref);

#endif
