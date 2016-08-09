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

#include "interpolate.h"
#include "plm_math.h"
#include "ray_trace.h"
#include "volume.h"
#include "volume_limit.h"

void
ray_trace_uniform (
    Volume* vol,                  // INPUT: CT Volume
    Volume_limit* vol_limit,      // INPUT: CT volume bounding box
    Ray_trace_callback callback,  // INPUT: Step Action Function
    void* callback_data,          // INPUT: callback data
    double* ip1in,                // INPUT: Ray Starting Point
    double* ip2in,                // INPUT: Ray Ending Point
    float ray_step                // INPUT: Uniform ray step size
)
{
    double uv[3];
    double ipx[3];
    double ps[3];
    double ip1[3];
    double ip2[3];
    double phy_step[3];

    int ai[3]={0,0,0};
    double frac[3]={0.0, 0.0, 0.0};

    float pix_density;
    double pt;  
    double rlen;
    int idx;
    size_t z;

    float mijk[3];
    float li_frac1[3];
    float li_frac2[3];
    plm_long mijk_f[3];
    plm_long mijk_r[3];

    float* img = (float*) vol->img;

    /* Test if ray intersects volume */
    if (!volume_limit_clip_segment (vol_limit, ip1, ip2, ip1in, ip2in)) {
	return;
    }
	

    ps[0] = vol->spacing[0];
    ps[1] = vol->spacing[1];
    ps[2] = vol->spacing[2];

    // Get ray length
    rlen = vec3_dist (ip1, ip2);

    // Get unit vector of ray
    vec3_sub3 (uv, ip2, ip1);
    vec3_normalize1 (uv);

    phy_step[0] = uv[0] * ray_step;
    phy_step[1] = uv[1] * ray_step;
    phy_step[2] = uv[2] * ray_step;

    // Trace the ray
    z = 0;
    for (pt = 0; pt < rlen; pt += ray_step)
    {
        // Compute a point along the ray
        ipx[0] = ip1[0] + phy_step[0] * z;
        ipx[1] = ip1[1] + phy_step[1] * z;
        ipx[2] = ip1[2] + phy_step[2] * z;

        // NEW VERSION - Compute CT volume indices and their fraction @ point + interpolation

        mijk[0] = (float) ((ipx[0] - vol->origin[0])/ps[0]);
        mijk[1] = (float) ((ipx[1] - vol->origin[1])/ps[1]);
        mijk[2] = (float) ((ipx[2] - vol->origin[2])/ps[2]);

        li_clamp_3d(mijk, mijk_f, mijk_r, li_frac1, li_frac2, vol);
        idx = volume_index (vol->dim, mijk_f);
        pix_density = li_value (li_frac1[0], li_frac2[0], li_frac1[1], li_frac2[1], li_frac1[2], li_frac2[2], idx, img, vol);

        // I am passing the current step along the ray (z) through
        // vox_index here... not exactly great but not horrible.
        (*callback) (callback_data, z++, ray_step, pix_density);
    }
}
