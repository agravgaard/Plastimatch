/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "logfile.h"
#include "print_and_exit.h"
#include "registration_resample.h"
#include "shared_parms.h"
#include "volume_resample.h"

Volume::Pointer
registration_resample_volume (
    const Volume::Pointer& vol,
    const Stage_parms* stage,
    const float resample_rate[3]
)
{
    const Shared_parms *shared = stage->get_shared_parms ();

    lprintf ("RESAMPLE %d %d: (%g %g %g), (%g %g %g)\n", 
        stage->resample_type,
        shared->legacy_subsampling,
        stage->resample_rate_fixed[0], stage->resample_rate_fixed[1], 
        stage->resample_rate_fixed[2], stage->resample_rate_moving[0], 
        stage->resample_rate_moving[1], stage->resample_rate_moving[2]
    );

    switch (stage->resample_type) {
    case RESAMPLE_AUTO:
    case RESAMPLE_VOXEL_RATE:
        if (shared->legacy_subsampling) {
            return volume_subsample_vox_legacy (vol, resample_rate);
        } else {
            return volume_subsample_vox (vol, resample_rate);
        }
        break;
    case RESAMPLE_MM:
        return volume_resample_spacing (vol, resample_rate);
        break;
    case RESAMPLE_PCT:
        return volume_resample_percent (vol, resample_rate);
        break;
    default:
        print_and_exit ("Unhandled resample_type %d "
            "in registration_resample_volume()\n",
            stage->resample_type);
        break;
    }

    /* Return null pointer on error */
    return Volume::Pointer();
}
