/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>

#include "gaussian.h"
#include "volume.h"
#include "volume_conv.h"
#include "volume_gaussian.h"

PLMBASE_API Volume::Pointer 
volume_gaussian (
    const Volume::Pointer& vol_in,
    float sigma, 
    float truncation
)
{
    float *kerx, *kery, *kerz;
    int fw[3];

    /* Set the filter widths */
    for (int d = 0; d < 3; d++) {
        int half_width = ROUND_INT (truncation * sigma / vol_in->spacing[d]);
        if (half_width < 1) {
            half_width = 1;
        }
        fw[d] = 2 * half_width + 1;
    }

    /* Create the seperable smoothing kernels for the x, y, and z directions */
    kerx = create_ker (sigma / vol_in->spacing[0], fw[0]/2);
    kery = create_ker (sigma / vol_in->spacing[1], fw[1]/2);
    kerz = create_ker (sigma / vol_in->spacing[2], fw[2]/2);
    kernel_stats (kerx, kery, kerz, fw);

    Volume::Pointer vf_out = volume_convolve_separable (
        vol_in, kerx, fw[0], kery, fw[1], kerz, fw[2]);

    free (kerx);
    free (kery);
    free (kerz);

    return vf_out;
}
