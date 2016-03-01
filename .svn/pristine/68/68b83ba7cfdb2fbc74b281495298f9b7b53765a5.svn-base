/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>

#include "logfile.h"
#include "plm_math.h"
#include "ray_trace.h"
#include "volume.h"
#include "volume_limit.h"

// TODO: All the #defines in base/ray_trace.h need to
//       be rethought/goaway.

static Point_location
test_boundary (Volume_limit* vol_limit, int d, double x)
{
    if (x < vol_limit->lower_limit[d]) {
        return POINTLOC_LEFT;
    } else if (x > vol_limit->upper_limit[d]) {
        return POINTLOC_RIGHT;
    } else {
        return POINTLOC_INSIDE;
    }
}

/* Return 1 if ray intersects volume */
int
volume_limit_clip_ray (
    Volume_limit *vol_limit,    /* INPUT:  The bounding box to clip to */
    double *ip1,                /* OUTPUT: Intersection point 1 */
    double *ip2,                /* OUTPUT: Intersection point 2 */
    const double *p1,           /* INPUT:  Starting point of ray */
    const double *ray           /* INPUT:  Direction of ray */
)
{
    Point_location ploc[3];
    double p2[3];
    double alpha[3][2];
    double alpha_in, alpha_out;
    int d;

    /* Make a second point in direction of ray */
    vec3_add3 (p2, p1, ray);
    
    /* Compute point location */
    for (d = 0; d < 3; d++) {
        ploc[d] = test_boundary (vol_limit, d, p1[d]);
    }

    /* Compute alphas */
    for (d = 0; d < 3; d++) {
        /* If ray is parallel to grid, location must be inside */
        if (fabs(ray[d]) < DRR_LEN_TOLERANCE) {
            if (ploc[d] != POINTLOC_INSIDE) {
                return 0;
            }
            alpha[d][0] = - DBL_MAX;
            alpha[d][1] = + DBL_MAX;
            continue;
        }

        /* General configuration */
        alpha[d][0] = (vol_limit->lower_limit[d] - p1[d]) / ray[d];
        alpha[d][1] = (vol_limit->upper_limit[d] - p1[d]) / ray[d];

        /* Sort alpha */
        if (alpha[d][0] > alpha[d][1]) {
            double temp = alpha[d][1];
            alpha[d][1] = alpha[d][0];
            alpha[d][0] = temp;
        }
    }

    /* Check if alpha values overlap in all three dimensions.
       alpha_in is the minimum alpha, where the ray enters the volume.
       alpha_out is where it exits the volume. */
    alpha_in = alpha[0][0];
    alpha_out = alpha[0][1];
    for (d = 1; d < 3; d++) {
        if (alpha_in < alpha[d][0]) alpha_in = alpha[d][0];
        if (alpha_out > alpha[d][1]) alpha_out = alpha[d][1];
    }
#if defined (DRR_VERBOSE)
    printf ("alpha[*][0] = %g %g %g\n", alpha[0][0], alpha[1][0], alpha[2][0]);
    printf ("alpha[*][1] = %g %g %g\n", alpha[0][1], alpha[1][1], alpha[2][1]);
    printf ("alpha in/out = %g %g\n", alpha_in, alpha_out);
#endif

    /* If exit is before entrance, the segment does not intersect the volume */
    if (alpha_out - alpha_in < DRR_LEN_TOLERANCE) {
        return 0;
    }

    /* Compute the volume intersection points */
    for (d = 0; d < 3; d++) {
        ip1[d] = p1[d] + alpha_in * ray[d];
        ip2[d] = p1[d] + alpha_out * ray[d];
    }

    return 1;
}

/* Return 1 if line segment intersects volume */
int
volume_limit_clip_segment (
    Volume_limit *vol_limit,    /* INPUT:  The bounding box to clip to */
    double *ip1,                /* OUTPUT: Intersection point 1 */
    double *ip2,                /* OUTPUT: Intersection point 2 */
    double *p1,                 /* INPUT:  Line segment point 1 */
    double *p2                  /* INPUT:  Line segment point 2 */
)
{
    Point_location ploc[3][2];
    double ray[3];
    double alpha_lo[3], alpha_hi[3];
    double alpha_in, alpha_out;
    int d;

    /* Compute the ray */
    vec3_sub3 (ray, p2, p1);

    for (d = 0; d < 3; d++) {
        ploc[d][0] = test_boundary (vol_limit, d, p1[d]);
        ploc[d][1] = test_boundary (vol_limit, d, p2[d]);
        /* Immediately reject segments which don't intersect the volume in 
           this dimension */
        if (ploc[d][0] == POINTLOC_LEFT && ploc[d][1] == POINTLOC_LEFT) {
            return 0;
        }
        if (ploc[d][0] == POINTLOC_RIGHT && ploc[d][1] == POINTLOC_RIGHT) {
            return 0;
        }
    }

    /* If we made it here, all three dimensions have some range of alpha
       where they intersects the volume.  However, these alphas might 
       not overlap.  We compute the alphas, then test overlapping 
       alphas to find the segment range within the volume.  */
    for (d = 0; d < 3; d++)
    {
        /* If ray is parallel to grid, location must be inside */
        if (fabs(ray[d]) < DRR_LEN_TOLERANCE) {
            if (ploc[d][0] != POINTLOC_INSIDE) {
                return 0;
            }
            alpha_lo[d] = - DBL_MAX;
            alpha_hi[d] = + DBL_MAX;
            continue;
        }

        alpha_lo[d] = (vol_limit->lower_limit[d] - p1[d]) / ray[d];
        alpha_hi[d] = (vol_limit->upper_limit[d] - p1[d]) / ray[d];

        /* Sort alphas */
        if (alpha_hi[d] < alpha_lo[d]) {
            double tmp = alpha_hi[d];
            alpha_hi[d] = alpha_lo[d];
            alpha_lo[d] = tmp;
        }

        /* Clip alphas to segment */
        if (alpha_lo[d] < 0.0) alpha_lo[d] = 0.0;
        if (alpha_lo[d] > 1.0) alpha_lo[d] = 1.0;
        if (alpha_hi[d] < 0.0) alpha_hi[d] = 0.0;
        if (alpha_hi[d] > 1.0) alpha_hi[d] = 1.0;
    }

    /* alpha_in is the alpha where the segment enters the boundary, and 
       alpha_out is where it exits the boundary.  */
    alpha_in = alpha_lo[0];
    alpha_out = alpha_hi[0];
    for (d = 1; d < 3; d++) {
        if (alpha_in < alpha_lo[d]) alpha_in = alpha_lo[d];
        if (alpha_out > alpha_hi[d]) alpha_out = alpha_hi[d];
    }
#if defined (DRR_VERBOSE)
    printf ("p1 = %g %g %g\n", p1[0], p1[1], p1[2]);
    printf ("p2 = %g %g %g\n", p2[0], p2[1], p2[2]);
    printf ("ray = %g %g %g\n", ray[0], ray[1], ray[2]);
    printf ("lower_lim = %g %g %g\n", vol_limit->lower_limit[0], 
        vol_limit->lower_limit[1], vol_limit->lower_limit[2]);
    printf ("upper_lim = %g %g %g\n", vol_limit->upper_limit[0], 
        vol_limit->upper_limit[1], vol_limit->upper_limit[2]);
    printf ("alpha_lo = %g %g %g\n", alpha_lo[0], alpha_lo[1], alpha_lo[2]);
    printf ("alpha_hi = %g %g %g\n", alpha_hi[0], alpha_hi[1], alpha_hi[2]);
    printf ("alpha in/out = %g %g\n", alpha_in, alpha_out);
#endif

    /* If exit is before entrance, the segment does not intersect the volume */
    if (alpha_out - alpha_in < DRR_LEN_TOLERANCE) {
        return 0;
    }

    /* Create the volume intersection points */
    for (d = 0; d < 3; d++) {
        ip1[d] = p1[d] + alpha_in * ray[d];
        ip2[d] = p1[d] + alpha_out * ray[d];
    }

    return 1;
}

void
volume_limit_set (Volume_limit *vol_limit, const Volume *vol)
{
    int d;

    /* Compute volume boundary box */
    for (d = 0; d < 3; d++) {
        vol_limit->lower_limit[d] = vol->origin[d] - 0.5 * vol->spacing[d];
        vol_limit->upper_limit[d] = vol_limit->lower_limit[d]
            + vol->dim[d] * vol->spacing[d];
        if (vol_limit->lower_limit[d] <= vol_limit->upper_limit[d]) {
            vol_limit->dir[d] = 1;
        } else {
            double tmp;
            vol_limit->dir[d] = -1;
            /* Swap limits */
            tmp = vol_limit->lower_limit[d];
            vol_limit->lower_limit[d] = vol_limit->upper_limit[d];
            vol_limit->upper_limit[d] = tmp;
        }
        vol_limit->lower_limit[d] += DRR_BOUNDARY_TOLERANCE;
        vol_limit->upper_limit[d] -= DRR_BOUNDARY_TOLERANCE;
    }
}

void
volume_limit_set (Volume_limit *vol_limit, const Volume::Pointer& vol)
{
    volume_limit_set (vol_limit, vol.get());
}

void
Volume_limit::print ()
{
    lprintf ("Volume_limit:\n%g %g / %g %g / %g %g\n",
        lower_limit[0], upper_limit[0], 
        lower_limit[1], upper_limit[1], 
        lower_limit[2], upper_limit[2]);
}
