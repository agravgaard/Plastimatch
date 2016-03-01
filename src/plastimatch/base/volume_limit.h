/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_limit_h_
#define _volume_limit_h_

#include "plmbase_config.h"

enum point_location {
    POINTLOC_LEFT,
    POINTLOC_INSIDE,
    POINTLOC_RIGHT,
};
typedef enum point_location Point_location;

class Volume_limit {
public:
    /* upper and lower limits of volume, including tolerances */
    double lower_limit[3];
    double upper_limit[3];

    /* dir == 0 if lower_limit corresponds to lower index */
    int dir[3];
public:
    void print ();
};

PLMBASE_C_API int volume_limit_clip_ray (
        Volume_limit *vol_limit,    /* INPUT:  The bounding box to clip to */
        double *ip1,                /* OUTPUT: Intersection point 1 */
        double *ip2,                /* OUTPUT: Intersection point 2 */
        const double *p1,           /* INPUT:  Starting point of ray */
        const double *ray           /* INPUT:  Direction of ray */
);
PLMBASE_C_API int volume_limit_clip_segment (
        Volume_limit *vol_limit,    /* INPUT:  The bounding box to clip to */
        double *ip1,                /* OUTPUT: Intersection point 1 */
        double *ip2,                /* OUTPUT: Intersection point 2 */
        double *p1,                 /* INPUT:  Line segment point 1 */
        double *p2                  /* INPUT:  Line segment point 2 */
);
PLMBASE_API void volume_limit_set (Volume_limit *vol_limit, 
    const Volume *vol);
PLMBASE_API void volume_limit_set (Volume_limit *vol_limit, 
    const Volume::Pointer& volume);


#endif
