/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _translation_mse_h_
#define _translation_mse_h_

#include "plmregister_config.h"
#include "volume.h"

float
translation_mse (
    const Stage_parms *stage,
    const Metric_state::Pointer& ssi,
    const float dxyz[3]);

#endif
