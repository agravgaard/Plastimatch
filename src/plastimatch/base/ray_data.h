/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _ray_data_h_
#define _ray_data_h_

#include "plmbase_config.h"

class Ray_data {
public:
    int ap_idx;
    bool intersects_volume;
    double ip1[3];       /* Front intersection with volume */
    double ip2[3];       /* Back intersection with volume */
    double p2[3];        /* Intersection with aperture plane */
    double ray[3];       /* Unit vector in direction of ray */
    double front_dist;   /* Distance from aperture to ip1 */
    double back_dist;    /* Distance from aperture to ip2 */
    double cp[3];        /* Intersection with front clipping plane */
    int step_offset;	 /* Number of steps before reaching k = 0 */
};

#endif
