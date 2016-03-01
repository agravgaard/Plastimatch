/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "plm_math.h"
#include "ray_trace_probe.h"
#include "volume.h"
#include "volume_limit.h"

void
ray_trace_probe (
    Volume* vol,                  // INPUT: CT Volume
    Volume_limit* vol_limit,      // INPUT: CT volume bounding box
    Ray_trace_callback callback,  // INPUT: Step Action Function
    void* callback_data,          // INPUT: callback data
    double* ip1in,                // INPUT: Ray Starting Point
    double* ip2in,                // INPUT: Ray Ending Point
    float ray_depth,              // INPUT: depth along ray to probe vol (mm)
    float ray_idx                 // INPUT: z-index along ray cast
)
{
    double uv[3];
    double ipx[3];
    double ps[3];
    double ip1[3];
    double ip2[3];

    int ai[3];

    float pix_density;
    int idx;

    float* img = (float*) vol->img;

    /* Test if ray intersects volume */
    if (!volume_limit_clip_segment (vol_limit, ip1, ip2, ip1in, ip2in)) {
        return;
    }

    ps[0] = vol->spacing[0];
    ps[1] = vol->spacing[1];
    ps[2] = vol->spacing[2];

    // Get unit vector of ray
    vec3_sub3 (uv, ip2, ip1);
    vec3_normalize1 (uv);

    /* Probe a point along the ray */

    // Compute a point along the ray
    ipx[0] = ip1[0] + uv[0] * ray_depth;
    ipx[1] = ip1[1] + uv[1] * ray_depth;
    ipx[2] = ip1[2] + uv[2] * ray_depth;

    // Compute CT Volume indices @ point
    ai[0] = (int) floor ((ipx[0] - vol->origin[0] + 0.5 * ps[0]) / ps[0]);
    ai[1] = (int) floor ((ipx[1] - vol->origin[1] + 0.5 * ps[1]) / ps[1]);
    ai[2] = (int) floor ((ipx[2] - vol->origin[2] + 0.5 * ps[2]) / ps[2]);

    if (ai[0] > 0 && ai[0] < vol->dim[0] 
        && ai[1] > 0 && ai[1] < vol->dim[1] 
        && ai[2] > 0 && ai[2] < vol->dim[2])
    {
        idx = ((ai[2]*vol->dim[1] + ai[1]) * vol->dim[0]) + ai[0];
        pix_density = img[idx];
        (*callback) (callback_data, ray_idx, ray_depth, pix_density);
    } else {
        return;
    }
}
