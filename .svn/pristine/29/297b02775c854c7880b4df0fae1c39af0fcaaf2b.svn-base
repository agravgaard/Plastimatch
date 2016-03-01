/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmreconstruct_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "drr_trilin.h"
#include "plm_math.h"

#define ULTRA_VERBOSE 1
//#define VERBOSE 1

#if defined (commentout)
/* Output is stored in ips (ips = intersection points) */
/* This version computes 1/2 a voxel within the volume, as used 
   for the interpolation methods */
int
drr_compute_boundary_intersections_2 (double ips[2][4], Volume* vol, 
				      double* p1, double* p2)
{
    /* Intersect ray with volume boundaries -- may not intersect in 
       degenerate cases 
       Boundaries are described by implicit eqn for plane:
       tau = [a b c d], where plane is (tau dot point) = 0
    */
    double ctx0[4] = {1,0,0,-(vol->xmin+vol->pix_spacing[0]/2.0)};
    double ctx1[4] = {1,0,0,-(vol->xmax-vol->pix_spacing[0]/2.0)};
    double cty0[4] = {0,1,0,-(vol->ymin+vol->pix_spacing[1]/2.0)};
    double cty1[4] = {0,1,0,-(vol->ymax-vol->pix_spacing[1]/2.0)};
    double ctz0[4] = {0,0,1,-(vol->zmin+vol->pix_spacing[2]/2.0)};
    double ctz1[4] = {0,0,1,-(vol->zmax-vol->pix_spacing[2]/2.0)};
    int ctx_bi[2] = {1,2};
    int cty_bi[2] = {0,2};
    int ctz_bi[2] = {0,1};
    double ctx_bd[4] = {vol->ymin,vol->ymax,vol->zmin,vol->zmax};
    double cty_bd[4] = {vol->xmin,vol->xmax,vol->zmin,vol->zmax};
    double ctz_bd[4] = {vol->xmin,vol->xmax,vol->ymin,vol->ymax};

    double rayh[4], p1h[4], p2h[4];
    int ipidx = 0;

    vec3_copy (p1h, p1);
    p1h[3] = 1.0;
    vec3_copy (p2h, p2);
    p2h[3] = 1.0;

    /* rayh = p2 - p1 */
    vec3_copy (rayh, p2);
    vec3_sub2 (rayh, p1);
    vec3_normalize1 (rayh);
    rayh[3] = 0.0;

#if defined (commentout)
    tr = drr_boundary_intersection_test (ip, ctx0, ctx_bi, ctx_bd, rayh, p1h, p2h);
    if (tr>0 && ipidx < 2) vec4_copy(ips[ipidx++],ip);
    tr = drr_boundary_intersection_test (ip, ctx1, ctx_bi, ctx_bd, rayh, p1h, p2h);
    if (tr>0 && ipidx < 2) vec4_copy(ips[ipidx++],ip);
    tr = drr_boundary_intersection_test (ip, cty0, cty_bi, cty_bd, rayh, p1h, p2h);
    if (tr>0 && ipidx < 2) vec4_copy(ips[ipidx++],ip);
    tr = drr_boundary_intersection_test (ip, cty1, cty_bi, cty_bd, rayh, p1h, p2h);
    if (tr>0 && ipidx < 2) vec4_copy(ips[ipidx++],ip);
    tr = drr_boundary_intersection_test (ip, ctz0, ctz_bi, ctz_bd, rayh, p1h, p2h);
    if (tr>0 && ipidx < 2) vec4_copy(ips[ipidx++],ip);
    tr = drr_boundary_intersection_test (ip, ctz1, ctz_bi, ctz_bd, rayh, p1h, p2h);
    if (tr>0 && ipidx < 2) vec4_copy(ips[ipidx++],ip);
#endif
    printf ("This function is unfinished");
    exit (-1);

    /* No intersection */
    if (ipidx < 2)
        return 0;
    else
	return 1;
}

void
drr_trace_init_loopvars_interp (int* ai, int* aidir, double* ao, double* al, 
				double pt, double ry, double samp)
{
    if (ry > 0) {
	*aidir = 1;
        *ai = (int) floor((pt+DRR_TOPLANE_TOLERANCE) / samp * 2.0);
	*ai = (*ai + 1) / 2;
        *ao = samp - (pt - (2*(*ai)-1) * samp / 2.0);
    } else {
	*aidir = -1;
        *ai = (int) floor((pt-DRR_TOPLANE_TOLERANCE) / samp * 2.0);
	*ai = (*ai + 1) / 2;
        *ao = samp - (((2*(*ai+1))-1) * samp / 2.0 - pt);
    }
    *al = samp;
    if (fabs(ry) > DRR_STRIDE_TOLERANCE) {
	*ao = *ao / fabs(ry);
	*al = *al / fabs(ry);
    } else {
	*ao = DRR_HUGE_DOUBLE;
	*al = DRR_HUGE_DOUBLE;
    }
}

void
drr_trace_init_loopvars_interp_2 (int* ai, int* aidir, double* ao, 
				  double* al, double pt, double ry, 
				  double samp)
{
    if (ry > 0) {
	*aidir = 1;
        *ai = (int) floor((pt-(samp/2.0)+DRR_TOPLANE_TOLERANCE) / samp);
        *ao = samp - ((pt+(samp/2.0)) - (*ai) * samp);
    } else {
	*aidir = -1;
        *ai = (int) floor((pt+(samp/2.0)-DRR_TOPLANE_TOLERANCE) / samp);
        *ao = samp - ((*ai+1) * samp - (pt-(samp/2.0)));
    }
    *al = samp;
    if (fabs(ry) > DRR_STRIDE_TOLERANCE) {
	*ao = *ao / fabs(ry);
	*al = *al / fabs(ry);
    } else {
	*ao = DRR_HUGE_DOUBLE;
	*al = DRR_HUGE_DOUBLE;
    }
}

double
interp_coefficient (double u0, double UU, double v0, double VV, 
		    double w0, double WW, int x, int y, int z)
{
    if (x == 1) {
	u0 = 1.0 - u0;
	UU = - UU;
    }
    if (y == 1) {
	v0 = 1.0 - v0;
	VV = - VV;
    }
    if (z == 1) {
	w0 = 1.0 - w0;
	WW = - WW;
    }
    return u0*v0*w0 
	    + (u0*v0*WW + u0*VV*w0 + UU*v0*w0) / 2.0
	    + (u0*VV*WW + UU*v0*WW + UU*VV*w0) / 3.0
	    + UU*VV*WW / 4.0;
}

/* GCS FIX: The last slice of voxels is wrong */
double
drr_trace_ray_trilin_exact (Volume* vol, double* p1in, double* p2in)
{
    double ips[2][4];
    double *p1 = &ips[0][0], *p2 = &ips[1][0];
    int ai_x, ai_y, ai_z;
    int aixdir, aiydir, aizdir;
    double ao_x, ao_y, ao_z;
    double ao_x0, ao_x1, ao_y0, ao_y1, ao_z0, ao_z1;
    double al_x, al_y, al_z;
    double rayh[4];
    double len;
    double aggr_len = 0.0;
    double accum = 0.0;
    int num_pix = 0;
    float* img = (float*) vol->img;

    if (!drr_compute_boundary_intersections (ips, vol, p1in, p2in)) {
	return 0.0;
    }
    len = vec3_dist(p1,p2);

#if defined (VERBOSE)
    printf ("p1i: ");
    vec3_print_eol (stdout,p1);
    printf ("p2i: ");
    vec3_print_eol (stdout,p2);
#endif

    vec3_sub3 (rayh, p2, p1);
    vec3_normalize1 (rayh);
    rayh[3] = 0.0;

    /* We'll go from p1 to p2 */
    /* Variable notation:
       ai_x    // index of x
       aixdir  // x indices moving up or down?
       ao_x    // absolute length to next voxel crossing
       al_x    // length between voxel crossings
    */
    drr_trace_init_loopvars_interp (&ai_x, &aixdir, &ao_x, &al_x, 
				    p1[0] - vol->xmin,
				    rayh[0], 
				    vol->pix_spacing[0]);
    drr_trace_init_loopvars_interp (&ai_y, &aiydir, &ao_y, &al_y, 
				    p1[1] - vol->ymin,
				    rayh[1], 
				    vol->pix_spacing[1]);
    drr_trace_init_loopvars_interp (&ai_z, &aizdir, &ao_z, &al_z, 
				    p1[2] - vol->zmin,
				    rayh[2], 
				    vol->pix_spacing[2]);
    do {
	int x1, x2, y1, y2, z1, z2;
	float pix111, pix112, pix121, pix122, 
		pix211, pix212, pix221, pix222;
	double pix_len;

	if (ai_x==0) {
	    x1 = ai_x;
	    x2 = ai_x;
	} else if (ai_x == vol->dim[2]) {
	    x1 = ai_x-1;
	    x2 = ai_x-1;
	} else {
	    x1 = ai_x-1;
	    x2 = ai_x;
	}
	if (ai_y==0) {
	    y1 = ai_y;
	    y2 = ai_y;
	} else if (ai_y == vol->dim[2]) {
	    y1 = ai_y-1;
	    y2 = ai_y-1;
	} else {
	    y1 = ai_y-1;
	    y2 = ai_y;
	}
	if (ai_z==0) {
	    z1 = ai_z;
	    z2 = ai_z;
	} else if (ai_z == vol->dim[2]) {
	    z1 = ai_z-1;
	    z2 = ai_z-1;
	} else {
	    z1 = ai_z-1;
	    z2 = ai_z;
	}

	pix111 = img[(z1*vol->dim[1]+y1)*vol->dim[0]+x1];
	pix112 = img[(z2*vol->dim[1]+y1)*vol->dim[0]+x1];
	pix121 = img[(z1*vol->dim[1]+y2)*vol->dim[0]+x1];
	pix122 = img[(z2*vol->dim[1]+y2)*vol->dim[0]+x1];
	pix211 = img[(z1*vol->dim[1]+y1)*vol->dim[0]+x2];
	pix212 = img[(z2*vol->dim[1]+y1)*vol->dim[0]+x2];
	pix221 = img[(z1*vol->dim[1]+y2)*vol->dim[0]+x2];
	pix222 = img[(z2*vol->dim[1]+y2)*vol->dim[0]+x2];

#if defined (ULTRA_VERBOSE)
	printf ("(%d %d %d) (%d %d %d) (%g,%g,%g) (%g,%g,%g)\n",
		ai_x,ai_y,ai_z,
		aixdir,aiydir,aizdir,
		ao_x,ao_y,ao_z,
		al_x,al_y,al_z);
#endif

	if (aixdir == +1) {
	    ao_x0 = al_x - ao_x;
	} else {
	    ao_x0 = ao_x;
	}
	if (aiydir == +1) {
	    ao_y0 = al_y - ao_y;
	} else {
	    ao_y0 = ao_y;
	}
	if (aizdir == +1) {
	    ao_z0 = al_z - ao_z;
	} else {
	    ao_z0 = ao_z;
	}

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
	    ao_z -= ao_y;
	    ao_x -= ao_y;
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
#if defined (commentout)
	if (aixdir == +1) {
	    ao_x1 = ao_x0 + pix_len;
	} else {
	    ao_x1 = ao_x0 - pix_len;
	}
	if (aiydir == +1) {
	    ao_y1 = ao_y0 + pix_len;
	} else {
	    ao_y1 = ao_y0 - pix_len;
	}
	if (aizdir == +1) {
	    ao_z1 = ao_z0 + pix_len;
	} else {
	    ao_z1 = ao_z0 - pix_len;
	}
#endif
	ao_x1 = ao_x0 + aixdir * pix_len;
	ao_y1 = ao_y0 + aiydir * pix_len;
	ao_z1 = ao_z0 + aizdir * pix_len;

#if defined (ULTRA_VERBOSE)
	printf ("AOXYZ = %g %g, %g %g, %g %g\n", ao_x0, ao_x1, 
		ao_y0, ao_y1, ao_z0, ao_z1);

#endif
	{
	    double u0 = ao_x0 / al_x;
	    double u1 = ao_x1 / al_x;
	    double UU = (u1 - u0);
	    double v0 = ao_y0 / al_y;
	    double v1 = ao_y1 / al_y;
	    double VV = (v1 - v0);
	    double w0 = ao_z0 / al_z;
	    double w1 = ao_z1 / al_z;
	    double WW = (w1 - w0);
#if defined (ULTRA_VERBOSE)
	    double tmp_accum;
#endif

#if defined (PREPROCESS_ATTENUATION)
	    accum += pix_len *
		    (interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1)
		     * pix111
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2)
		     * pix112
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1)
		     * pix121
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2)
		     * pix122
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1)
		     * pix211
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2)
		     * pix212
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1)
		     * pix221
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2)
		     * pix222);
#else
	    accum += pix_len *
		    (interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1)
		     * attenuation_lookup (pix111)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2)
		     * attenuation_lookup (pix112)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1)
		     * attenuation_lookup (pix121)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2)
		     * attenuation_lookup (pix122)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1)
		     * attenuation_lookup (pix211)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2)
		     * attenuation_lookup (pix212)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1)
		     * attenuation_lookup (pix221)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2)
		     * attenuation_lookup (pix222));
#endif
#if defined (ULTRA_VERBOSE)
	    printf ("UVW = (%g,%g,%g, * %g,%g,%g, * %g,%g,%g)\n", 
		    u0,u1,UU,v0,v1,VV,w0,w1,WW);
	    tmp_accum = interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2);
	    printf ("ACCUM = %g (%g,%g,%g,%g,%g,%g,%g,%g)\n", 
		    tmp_accum,
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2));
#endif
	    if (interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1) < 0 
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2) < 0) {
		printf ("Error with interp_coefficient()\n");
	    }
	}
	num_pix++;
    } while (aggr_len+DRR_LEN_TOLERANCE < len);
    return accum;
}


double
drr_trace_ray_trilin_exact_under_development (Volume* vol, double* p1in, 
					      double* p2in)
{
    double ips[2][4];
    double *p1 = &ips[0][0], *p2 = &ips[1][0];
    int ai_x, ai_y, ai_z;
    int aixdir, aiydir, aizdir;
    double ao_x, ao_y, ao_z;
//    double ao_x0, ao_x1, ao_y0, ao_y1, ao_z0, ao_z1;
    double al_x, al_y, al_z;
    double rayh[4];
    double len;
    double aggr_len = 0.0;
    double accum = 0.0;
    double ap_x0, ap_x1, ap_x, ap_y0, ap_y1, ap_y, ap_z0, ap_z1, ap_z;

    int num_pix = 0;
    float* img = (float*) vol->img;

    if (!drr_compute_boundary_intersections (ips, vol, p1in, p2in)) {
	return 0.0;
    }
    len = vec3_dist(p1,p2);

#if defined (VERBOSE)
    vec_set_fmt ("%12.10g ");
    printf ("p1i: ");
    vec3_print_eol (stdout,p1);
    printf ("p2i: ");
    vec3_print_eol (stdout,p2);
#endif

    if (!drr_compute_boundary_intersections_2 (ips, vol, p1in, p2in)) {
	return 0.0;
    }
    len = vec3_dist(p1,p2);

#if defined (VERBOSE)
    printf ("p1i: ");
    vec3_print_eol (stdout,p1);
    printf ("p2i: ");
    vec3_print_eol (stdout,p2);
#endif

    vec3_sub3 (rayh, p2, p1);
    vec3_normalize1 (rayh);
    rayh[3] = 0.0;

    /* We'll go from p1 to p2 */
    /* Variable notation:
       ai_x    // index of x
       aixdir  // x indices moving up or down?
       ao_x    // absolute length to next voxel crossing
       al_x    // length between voxel crossings
    */
    drr_trace_init_loopvars_interp_2 (&ai_x, &aixdir, &ao_x, &al_x, 
				      p1[0] - vol->xmin,
				      rayh[0], 
				      vol->pix_spacing[0]);
    drr_trace_init_loopvars_interp_2 (&ai_y, &aiydir, &ao_y, &al_y, 
				      p1[1] - vol->ymin,
				      rayh[1], 
				      vol->pix_spacing[1]);
    drr_trace_init_loopvars_interp_2 (&ai_z, &aizdir, &ao_z, &al_z, 
				      p1[2] - vol->zmin,
				      rayh[2], 
				      vol->pix_spacing[2]);

    /* 
       ap_x     // intra-voxel position of x
       ap_x0    // intra-voxel position of x after a crossing
     */
    if (aixdir == +1) {
	ap_x0 = 0.0;
	ap_x1 = 1.0;
	ap_x = ao_x;
    } else {
	ap_x0 = 1.0;
	ap_x1 = 0.0;
	ap_x = al_x - ao_x;
    }
    if (aiydir == +1) {
	ap_y0 = 0.0;
	ap_y1 = 1.0;
	ap_y = ao_y;
    } else {
	ap_y0 = 1.0;
	ap_y1 = 0.0;
	ap_y = al_y - ao_y;
    }
    if (aizdir == +1) {
	ap_z0 = 0.0;
	ap_z1 = 1.0;
	ap_z = ao_z;
    } else {
	ap_z0 = 1.0;
	ap_z1 = 0.0;
	ap_z = al_z - ao_z;
    }
    do {
	int x1, x2, y1, y2, z1, z2;
	float pix111, pix112, pix121, pix122, 
		pix211, pix212, pix221, pix222;
	double pix_len;
	double ap_x_init = ap_x;
	double ap_y_init = ap_y;
	double ap_z_init = ap_z;
	double ap_x_end;
	double ap_y_end;
	double ap_z_end;

#if defined (ULTRA_VERBOSE)
	printf ("(%d %d %d) (%d %d %d) (%g,%g,%g) (%g,%g,%g)\n",
		ai_x,ai_y,ai_z,
		aixdir,aiydir,aizdir,
		ao_x,ao_y,ao_z,
		al_x,al_y,al_z);
#endif

	x1 = ai_x;
	x2 = ai_x + 1;
	y1 = ai_y;
	y2 = ai_y + 1;
	z1 = ai_z;
	z2 = ai_z + 1;

	pix111 = img[(z1*vol->dim[1]+y1)*vol->dim[0]+x1];
	pix112 = img[(z2*vol->dim[1]+y1)*vol->dim[0]+x1];
	pix121 = img[(z1*vol->dim[1]+y2)*vol->dim[0]+x1];
	pix122 = img[(z2*vol->dim[1]+y2)*vol->dim[0]+x1];
	pix211 = img[(z1*vol->dim[1]+y1)*vol->dim[0]+x2];
	pix212 = img[(z2*vol->dim[1]+y1)*vol->dim[0]+x2];
	pix221 = img[(z1*vol->dim[1]+y2)*vol->dim[0]+x2];
	pix222 = img[(z2*vol->dim[1]+y2)*vol->dim[0]+x2];

	if ((ao_x < ao_y) && (ao_x < ao_z)) {
	    pix_len = ao_x;
	    aggr_len += ao_x;
	    ao_y -= ao_x;
	    ao_z -= ao_x;
	    ao_x = al_x;
	    ai_x += aixdir;
	    ap_x = ap_x0;
	    ap_y += aiydir * ao_x / al_y;
	    ap_z += aizdir * ao_x / al_z;
	    ap_x_end = ap_x1;
	    ap_y_end = ap_y;
	    ap_z_end = ap_z;
	} else if ((ao_y < ao_z)) {
	    pix_len = ao_y;
	    aggr_len += ao_y;
	    ao_z -= ao_y;
	    ao_x -= ao_y;
	    ao_y = al_y;
	    ap_y = ap_y0;
	    ai_y += aiydir;
	    ap_x += aixdir * ao_y / al_x;
	    ap_y = ap_y0;
	    ap_z += aizdir * ao_y / al_z;
	    ap_x_end = ap_x;
	    ap_y_end = ap_y1;
	    ap_z_end = ap_z;
	} else {
	    pix_len = ao_z;
	    aggr_len += ao_z;
	    ao_x -= ao_z;
	    ao_y -= ao_z;
	    ao_z = al_z;
	    ap_z = ap_z0;
	    ai_z += aizdir;
	    ap_x += aixdir * ao_z / al_x;
	    ap_y += aiydir * ao_z / al_y;
	    ap_z = ap_z0;
	    ap_x_end = ap_x;
	    ap_y_end = ap_y;
	    ap_z_end = ap_z1;
	}

#if defined (ULTRA_VERBOSE)
	printf ("LEN = %g AGG = %g\n", len, aggr_len);
	printf ("AOXYZ = %g %g, %g %g, %g %g\n", 
		ap_x_init, ap_x_end, 
		ap_y_init, ap_y_end, 
		ap_z_init, ap_z_end);
#endif
	{
	    double u0 = ap_x_init;
	    double UU = ap_x_end - ap_x_init;
	    double v0 = ap_y_init;
	    double VV = ap_y_end - ap_y_init;
	    double w0 = ap_z_init;
	    double WW = ap_y_end - ap_y_init;
#if defined (ULTRA_VERBOSE)
	    double tmp_accum;
#endif

#if defined (PREPROCESS_ATTENUATION)
	    accum += pix_len *
		    (interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1)
		     * pix111
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2)
		     * pix112
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1)
		     * pix121
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2)
		     * pix122
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1)
		     * pix211
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2)
		     * pix212
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1)
		     * pix221
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2)
		     * pix222);
#else
	    accum += pix_len *
		    (interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1)
		     * attenuation_lookup (pix111)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2)
		     * attenuation_lookup (pix112)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1)
		     * attenuation_lookup (pix121)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2)
		     * attenuation_lookup (pix122)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1)
		     * attenuation_lookup (pix211)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2)
		     * attenuation_lookup (pix212)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1)
		     * attenuation_lookup (pix221)
		     + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2)
		     * attenuation_lookup (pix222));
#endif
#if defined (ULTRA_VERBOSE)
	    printf ("UVW = (%g,%g, * %g,%g * %g,%g)\n", 
		    u0,UU,v0,VV,w0,WW);
	    tmp_accum = interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1)
		    + interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2);
	    printf ("ACCUM = %g (%g,%g,%g,%g,%g,%g,%g,%g)\n", 
		    tmp_accum,
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1),
		    interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2));
	    if (interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,1) < 0 
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,1,1,2) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,1) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,1,2,2) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,1) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,1,2) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,1) < 0
		|| interp_coefficient (u0,UU,v0,VV,w0,WW,2,2,2) < 0) {
		printf ("Error with interp_coefficient()\n");
	    }
#endif
	}
	num_pix++;
    } while (aggr_len+DRR_LEN_TOLERANCE < len);
    return accum;
}

double
approx_coefficient (double UU, double VV, double WW, int x, int y, int z)
{
    if (x == 1) {
	UU = 1.0 - UU;
    }
    if (y == 1) {
	VV = 1.0 - VV;
    }
    if (z == 1) {
	WW = 1.0 - WW;
    }
    return UU * VV * WW;
}

double
drr_trace_ray_trilin_approx (Volume* vol, double* p1in, double* p2in)
{
    double ips[2][4];
    double *p1 = &ips[0][0], *p2 = &ips[1][0];
    int ai_x, ai_y, ai_z;
    int aixdir, aiydir, aizdir;
    double ao_x, ao_y, ao_z;
    double ao_x0, ao_x1, ao_y0, ao_y1, ao_z0, ao_z1;
    double al_x, al_y, al_z;
    double rayh[4];
    double len;
    double aggr_len = 0.0;
    double accum = 0.0;
    int num_pix = 0;
    float* img = (float*) vol->img;

    if (!drr_compute_boundary_intersections (ips, vol, p1in, p2in)) {
	return 0.0;
    }
    len = vec3_dist(p1,p2);

#if defined (VERBOSE)
    printf ("p1i: ");
    vec3_print_eol (stdout,p1);
    printf ("p2i: ");
    vec3_print_eol (stdout,p2);
#endif

    vec3_sub3 (rayh, p2, p1);
    vec3_normalize1 (rayh);
    rayh[3] = 0.0;

    /* We'll go from p1 to p2 */
    /* Variable notation:
       ai_x    // index of x
       aixdir  // x indices moving up or down?
       ao_x    // absolute length to next voxel crossing
       al_x    // length between voxel crossings
    */
    drr_trace_init_loopvars_interp (&ai_x, &aixdir, &ao_x, &al_x, 
				    p1[0] - vol->xmin, rayh[0], 
				    vol->pix_spacing[0]);
    drr_trace_init_loopvars_interp (&ai_y, &aiydir, &ao_y, &al_y, 
				    p1[1] - vol->ymin, rayh[1], 
				    vol->pix_spacing[1]);
    drr_trace_init_loopvars_interp (&ai_z, &aizdir, &ao_z, &al_z, 
				    p1[2] - vol->zmin, rayh[2], 
				    vol->pix_spacing[2]);
    do {
	int x1, x2, y1, y2, z1, z2;
	float pix111, pix112, pix121, pix122, 
		pix211, pix212, pix221, pix222;
	double pix_len;

	if (ai_x==0) {
	    x1 = ai_x;
	    x2 = ai_x;
	} else if (ai_x == vol->dim[2]) {
	    x1 = ai_x-1;
	    x2 = ai_x-1;
	} else {
	    x1 = ai_x-1;
	    x2 = ai_x;
	}
	if (ai_y==0) {
	    y1 = ai_y;
	    y2 = ai_y;
	} else if (ai_y == vol->dim[2]) {
	    y1 = ai_y-1;
	    y2 = ai_y-1;
	} else {
	    y1 = ai_y-1;
	    y2 = ai_y;
	}
	if (ai_z==0) {
	    z1 = ai_z;
	    z2 = ai_z;
	} else if (ai_z == vol->dim[2]) {
	    z1 = ai_z-1;
	    z2 = ai_z-1;
	} else {
	    z1 = ai_z-1;
	    z2 = ai_z;
	}
	pix111 = img[(z1*vol->dim[1]+y1)*vol->dim[0]+x1];
	pix112 = img[(z2*vol->dim[1]+y1)*vol->dim[0]+x1];
	pix121 = img[(z1*vol->dim[1]+y2)*vol->dim[0]+x1];
	pix122 = img[(z2*vol->dim[1]+y2)*vol->dim[0]+x1];
	pix211 = img[(z1*vol->dim[1]+y1)*vol->dim[0]+x2];
	pix212 = img[(z2*vol->dim[1]+y1)*vol->dim[0]+x2];
	pix221 = img[(z1*vol->dim[1]+y2)*vol->dim[0]+x2];
	pix222 = img[(z2*vol->dim[1]+y2)*vol->dim[0]+x2];
#if defined (ULTRA_VERBOSE)
	printf ("(%d %d %d) (%d %d %d) (%g,%g,%g) (%g,%g,%g)\n",
		ai_x,ai_y,ai_z,
		aixdir,aiydir,aizdir,
		ao_x,ao_y,ao_z,
		al_x,al_y,al_z);
#endif
	if (aixdir == +1) {
	    ao_x0 = al_x - ao_x;
	} else {
	    ao_x0 = ao_x;
	}
	if (aiydir == +1) {
	    ao_y0 = al_y - ao_y;
	} else {
	    ao_y0 = ao_y;
	}
	if (aizdir == +1) {
	    ao_z0 = al_z - ao_z;
	} else {
	    ao_z0 = ao_z;
	}

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
	    ao_z -= ao_y;
	    ao_x -= ao_y;
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

	if (aixdir == +1) {
	    ao_x1 = ao_x0 + pix_len;
	} else {
	    ao_x1 = ao_x0 - pix_len;
	}
	if (aiydir == +1) {
	    ao_y1 = ao_y0 + pix_len;
	} else {
	    ao_y1 = ao_y0 - pix_len;
	}
	if (aizdir == +1) {
	    ao_z1 = ao_z0 + pix_len;
	} else {
	    ao_z1 = ao_z0 - pix_len;
	}
	
#if defined (ULTRA_VERBOSE)
	printf ("AOXYZ = %g %g, %g %g, %g %g\n", ao_x0, ao_x1, 
		ao_y0, ao_y1, ao_z0, ao_z1);

#endif
	{
	    double u0 = ao_x0 / al_x;
	    double u1 = ao_x1 / al_x;
	    double UU = (u1 + u0) / 2.0;
	    double v0 = ao_y0 / al_y;
	    double v1 = ao_y1 / al_y;
	    double VV = (v1 + v0) / 2.0;
	    double w0 = ao_z0 / al_z;
	    double w1 = ao_z1 / al_z;
	    double WW = (w1 + w0) / 2.0;
#if defined (ULTRA_VERBOSE)
	    double tmp_accum;
#endif

#if defined (PREPROCESS_ATTENUATION)
	    accum += pix_len * 
		    (approx_coefficient (UU,VV,WW,1,1,1)
		     * pix111
		     + approx_coefficient (UU,VV,WW,1,1,2)
		     * pix112
		     + approx_coefficient (UU,VV,WW,1,2,1)
		     * pix121
		     + approx_coefficient (UU,VV,WW,1,2,2)
		     * pix122
		     + approx_coefficient (UU,VV,WW,2,1,1)
		     * pix211
		     + approx_coefficient (UU,VV,WW,2,1,2)
		     * pix212
		     + approx_coefficient (UU,VV,WW,2,2,1)
		     * pix221
		     + approx_coefficient (UU,VV,WW,2,2,2)
		     * pix222);
#else
	    accum += pix_len * 
		    (approx_coefficient (UU,VV,WW,1,1,1)
		     * attenuation_lookup (pix111)
		     + approx_coefficient (UU,VV,WW,1,1,2)
		     * attenuation_lookup (pix112)
		     + approx_coefficient (UU,VV,WW,1,2,1)
		     * attenuation_lookup (pix121)
		     + approx_coefficient (UU,VV,WW,1,2,2)
		     * attenuation_lookup (pix122)
		     + approx_coefficient (UU,VV,WW,2,1,1)
		     * attenuation_lookup (pix211)
		     + approx_coefficient (UU,VV,WW,2,1,2)
		     * attenuation_lookup (pix212)
		     + approx_coefficient (UU,VV,WW,2,2,1)
		     * attenuation_lookup (pix221)
		     + approx_coefficient (UU,VV,WW,2,2,2)
		     * attenuation_lookup (pix222));
#endif

#if defined (ULTRA_VERBOSE)
	    printf ("UVW = (%g,%g,%g, * %g,%g,%g, * %g,%g,%g)\n", 
		    u0,u1,UU,v0,v1,VV,w0,w1,WW);
	    tmp_accum = approx_coefficient (UU,VV,WW,1,1,1)
		    + approx_coefficient (UU,VV,WW,1,1,2)
		    + approx_coefficient (UU,VV,WW,1,2,1)
		    + approx_coefficient (UU,VV,WW,1,2,2)
		    + approx_coefficient (UU,VV,WW,2,1,1)
		    + approx_coefficient (UU,VV,WW,2,1,2)
		    + approx_coefficient (UU,VV,WW,2,2,1)
		    + approx_coefficient (UU,VV,WW,2,2,2);
	    printf ("ACCUM = %g (%g,%g,%g,%g,%g,%g,%g,%g)\n", 
		    tmp_accum,
		    approx_coefficient (UU,VV,WW,1,1,1),
		    approx_coefficient (UU,VV,WW,1,1,2),
		    approx_coefficient (UU,VV,WW,1,2,1),
		    approx_coefficient (UU,VV,WW,1,2,2),
		    approx_coefficient (UU,VV,WW,2,1,1),
		    approx_coefficient (UU,VV,WW,2,1,2),
		    approx_coefficient (UU,VV,WW,2,2,1),
		    approx_coefficient (UU,VV,WW,2,2,2));
	    if (approx_coefficient (UU,VV,WW,1,1,1) < 0 
		|| approx_coefficient (UU,VV,WW,1,1,2) < 0
		|| approx_coefficient (UU,VV,WW,1,2,1) < 0
		|| approx_coefficient (UU,VV,WW,1,2,2) < 0
		|| approx_coefficient (UU,VV,WW,2,1,1) < 0
		|| approx_coefficient (UU,VV,WW,2,1,2) < 0
		|| approx_coefficient (UU,VV,WW,2,2,1) < 0
		|| approx_coefficient (UU,VV,WW,2,2,2) < 0) {
		printf ("Error with approx_coefficient()\n");
	    }
#endif
	}
	num_pix++;
    } while (aggr_len+DRR_LEN_TOLERANCE < len);
    return accum;
}
#endif

double
drr_trace_ray_trilin_approx (Volume* vol, double* p1in, double* p2in)
{
    return 0.0;
}

double
drr_trace_ray_trilin_exact (Volume* vol, double* p1in, double* p2in)
{
    return 0.0;
}
