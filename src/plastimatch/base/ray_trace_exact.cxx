/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "plm_math.h"
#include "ray_trace.h"
#include "volume.h"
#include "volume_limit.h"

void
ray_trace_exact_init_loopvars (
    int* ai,           /* Output: index */
    int* aidir,        /* Output: are indices moving up or down? */
    double* ao,        /* Output: absolute length to next voxel crossing */
    double* al,        /* Output: length between voxel crossings */
    double pt,         /* Input:  initial intersection of ray with volume */
    double ry,         /* Input:  normalized direction of ray */
    double origin,     /* Input:  origin of volume */
    double samp        /* Input:  pixel spacing of volume */
)
{
#if (DRR_VERBOSE)
    printf ("pt/ry/off/samp: %g %g %g %g\n", pt, ry, origin, samp);
#endif

    *aidir = SIGN (ry) * SIGN (samp);
    *ai = ROUND_INT ((pt - origin) / samp);
    *ao = SIGN (ry) 
	* (((*ai) * samp + origin) + (SIGN (ry) * 0.5 * fabs (samp)) - pt);

    if (fabs(ry) > DRR_STRIDE_TOLERANCE) {
	*ao = *ao / fabs(ry);
	*al = fabs(samp) / fabs(ry);
    } else {
	*ao = DRR_HUGE_DOUBLE;
	*al = DRR_HUGE_DOUBLE;
    }

#if defined (commentout)
    if (ry > 0) {
	*aidir = 1 * SIGN (samp);
        *ai = (int) floor ((pt - origin + 0.5 * samp) / samp);
        *ao = fabs(samp - ((pt - origin + 0.5 * samp) - (*ai) * samp));
    } else {
	*aidir = -1 * SIGN (samp);
        *ai = (int) floor ((pt - origin + 0.5 * samp) / samp);
        *ao = fabs(samp - ((*ai+1) * samp - (pt - origin + 0.5 * samp)));
    }
    if (fabs(ry) > DRR_STRIDE_TOLERANCE) {
	*ao = *ao / fabs(ry);
	*al = fabs(samp) / fabs(ry);
    } else {
	*ao = DRR_HUGE_DOUBLE;
	*al = DRR_HUGE_DOUBLE;
    }
#endif
}

/* Initialize loop variables.  Returns 1 if the segment intersects 
   the volume, and 0 if the segment does not intersect. */
int
ray_trace_exact_init (
    int *ai_x,
    int *ai_y,
    int *ai_z,
    int *aixdir, 
    int *aiydir, 
    int *aizdir,
    double *ao_x, 
    double *ao_y, 
    double *ao_z,
    double *al_x, 
    double *al_y, 
    double *al_z,
    double *len,
    Volume* vol, 
    Volume_limit *vol_limit,
    double* p1, 
    double* p2 
)
{
    double ray[3];
    double ip1[3];
    double ip2[3];
    //double ips[2][4];

    /* Test if ray intersects volume */
    if (!volume_limit_clip_segment (vol_limit, ip1, ip2, p1, p2)) {
	return 0;
    }

    /* Create the volume intersection points */
    vec3_sub3 (ray, p2, p1);
    vec3_normalize1 (ray);

#if defined (DRR_VERBOSE)
    printf ("ip1 = %g %g %g\n", ip1[0], ip1[1], ip1[2]);
    printf ("ip2 = %g %g %g\n", ip2[0], ip2[1], ip2[2]);
    printf ("ray = %g %g %g\n", ray[0], ray[1], ray[2]);
#endif

    /* We'll go from p1 to p2 */
    /* Variable notation:
       ai_x    // index of x
       aixdir  // x indices moving up or down?
       ao_x    // absolute length to next voxel crossing
       al_x    // length between voxel crossings
    */
    ray_trace_exact_init_loopvars (
	ai_x, aixdir, ao_x, al_x, 
	ip1[0],
	ray[0], 
	vol->origin[0], 
	vol->spacing[0]);
    ray_trace_exact_init_loopvars (
	ai_y, aiydir, ao_y, al_y, 
	ip1[1],
	ray[1], 
	vol->origin[1], 
	vol->spacing[1]);
    ray_trace_exact_init_loopvars (
	ai_z, aizdir, ao_z, al_z, 
	ip1[2], 
	ray[2], 
	vol->origin[2], 
	vol->spacing[2]);

#if defined (DRR_VERBOSE)
    printf ("aix = %d aixdir = %d aox = %g alx = %g\n", 
	*ai_x, *aixdir, *ao_x, *al_x);
    printf ("aiy = %d aiydir = %d aoy = %g aly = %g\n", 
	*ai_y, *aiydir, *ao_y, *al_y);
    printf ("aiz = %d aizdir = %d aoz = %g alz = %g\n", 
	*ai_z, *aizdir, *ao_z, *al_z);
#endif

    *len = vec3_dist (ip1, ip2);
    return 1;
}

void
ray_trace_exact (
    Volume *vol,                  /* Input: volume */
    Volume_limit *vol_limit,      /* Input: min/max coordinates of volume */
    Ray_trace_callback callback,  /* Input: callback function */
    void *callback_data,          /* Input: callback function private data */
    double *p1in,                 /* Input: start point for ray */
    double *p2in                  /* Input: end point for ray */
)
{
    /* Variable notation:
       ai_x     index of x
       aixdir   x indices moving up or down?
       ao_x     absolute length to next voxel crossing
       al_x     length between voxel crossings
    */
    int ai_x, ai_y, ai_z;
    int aixdir, aiydir, aizdir;
    double ao_x, ao_y, ao_z;
    double al_x, al_y, al_z;
    double len;                       /* Total length of ray within volume */
    double aggr_len = 0.0;            /* Length traced so far */
    float* img = (float*) vol->img;

#if defined (DRR_VERBOSE)
    printf ("p1in: %f %f %f\n", p1in[0], p1in[1], p1in[2]);
    printf ("p2in: %f %f %f\n", p2in[0], p2in[1], p2in[2]);
#endif

    if (!ray_trace_exact_init (
	    &ai_x,
	    &ai_y,
	    &ai_z,
	    &aixdir, 
	    &aiydir, 
	    &aizdir,
	    &ao_x, 
	    &ao_y, 
	    &ao_z,
	    &al_x, 
	    &al_y, 
	    &al_z,
	    &len,
	    vol, 
	    vol_limit, 
	    p1in, 
	    p2in))
    {
	return;
    }

    /* We'll go from p1 to p2 */
    do {
	float pix_density;
	double pix_len;
	int index = 
	    ai_z*vol->dim[0]*vol->dim[1] 
	    + ai_y*vol->dim[0] 
	    + ai_x;

#if defined (DRR_ULTRA_VERBOSE)
	printf ("(%d %d %d) (%g,%g,%g)\n",ai_x,ai_y,ai_z,ao_x,ao_y,ao_z);
	printf ("aggr_len = %g, len = %g\n", aggr_len, len);
	fflush (stdout);
#endif
	pix_density = img[index];
	if ((ao_x < ao_y) && (ao_x < ao_z)) {
	    pix_len = ao_x;
	    aggr_len += ao_x;
	    ao_y -= ao_x;
	    ao_z -= ao_x;
	    ao_x = al_x;
	    ai_x += aixdir;
	} else if ((ao_y < ao_z)) {
	    pix_len = ao_y;
	    aggr_len += ao_y;
	    ao_x -= ao_y;
	    ao_z -= ao_y;
	    ao_y = al_y;
	    ai_y += aiydir;
	} else {
	    pix_len = ao_z;
	    aggr_len += ao_z;
	    ao_x -= ao_z;
	    ao_y -= ao_z;
	    ao_z = al_z;
	    ai_z += aizdir;
	}
	(*callback) (callback_data, index, pix_len, pix_density);
	
    } while (aggr_len+DRR_LEN_TOLERANCE < len);
}
