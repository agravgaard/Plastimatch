/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_resample_h_
#define _volume_resample_h_

#include "plmbase_config.h"

PLMBASE_API Volume::Pointer volume_resample (
    const Volume::Pointer& vol_in,
    const plm_long* dim,
    const float* offset,
    const float* spacing
);
PLMBASE_API Volume::Pointer volume_resample (
    const Volume::Pointer& vol_in, const Volume_header *vh);
PLMBASE_API Volume::Pointer volume_resample_nn (
    const Volume::Pointer& vol_in,
    const plm_long* dim,
    const float* offset,
    const float* spacing
);
PLMBASE_API Volume::Pointer volume_subsample_vox (
    const Volume::Pointer& vol_in, const float* sampling_rate);
PLMBASE_API Volume::Pointer volume_subsample_vox_nn (
    const Volume::Pointer& vol_in, const float* sampling_rate);
PLMBASE_API Volume::Pointer volume_subsample_vox_legacy (
    const Volume::Pointer& vol_in, const float* sampling_rate);
PLMBASE_API Volume::Pointer volume_subsample_vox_legacy_nn (
    const Volume::Pointer& vol_in, const float* sampling_rate);

PLMBASE_API Volume::Pointer volume_resample_spacing (
    const Volume::Pointer& vol_in, const float spacing[3]);

PLMBASE_API Volume::Pointer volume_resample_percent (
    const Volume::Pointer& vol_in, const float percent[3]);

#endif
