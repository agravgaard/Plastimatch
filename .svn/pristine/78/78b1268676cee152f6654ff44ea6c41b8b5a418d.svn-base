/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#define DRR_HUGE_DOUBLE 1e10
#define DRR_LEN_TOLERANCE 1e-6
#define DRR_STRIDE_TOLERANCE 1e-10

/* From volume_limit.c */
int
volume_limit_clip_segment (
    float4 lower_limit,         /* INPUT:  The bounding box to clip to */
    float4 upper_limit,         /* INPUT:  The bounding box to clip to */
    float4 *ip1,                /* OUTPUT: Intersection point 1 */
    float4 *ip2,                /* OUTPUT: Intersection point 2 */
    float4 p1,                  /* INPUT:  Line segment point 1 */
    float4 p2                   /* INPUT:  Line segment point 2 */
)
{
    float4 ray, inv_ray;
    float alpha_in, alpha_out;
    float4 alpha_low, alpha_high;
    int4 ploc = {-1, -1, -1, 0};
    int4 is_parallel;

    ray = p2 - p1;
    inv_ray = 1.0f / ray;

    /* Find intersection configuration of ray base */
    /* -1 is POINTLOC_LEFT, 0 is POINTLOC_INSIDE, 1 is POINTLOC_RIGHT */
    if (p1.x > upper_limit.x) {
	ploc.x = 1;
    } else if (p1.x > lower_limit.x) {
	ploc.x = 0;
    }
    if (p1.y > upper_limit.y) {
	ploc.y = 1;
    } else if (p1.y > lower_limit.y) {
	ploc.y = 0;
    }
    if (p1.z > upper_limit.z) {
	ploc.z = 1;
    } else if (p1.z > lower_limit.z) {
	ploc.z = 0;
    }

    /* Check if ray parallel to grid */
    is_parallel = fabs (ray) < DRR_LEN_TOLERANCE;

    /* Compute alphas for general configuration */
    alpha_low = (lower_limit - p1) * inv_ray;
    alpha_high = (upper_limit - p1) * inv_ray;

    /* Check case where ray is parallel to grid.  If any dimension is 
       parallel to grid, then p1 must be inside slap, otherwise there 
       is no intersection of segment and cube. */
    if (is_parallel.x) {
	if (!ploc.x) return 0;
	alpha_low.x = - FLT_MAX;
	alpha_high.x = + FLT_MAX;
    }
    if (is_parallel.y) {
	if (!ploc.y) return 0;
	alpha_low.y = - FLT_MAX;
	alpha_high.y = + FLT_MAX;
    }
    if (is_parallel.z) {
	if (!ploc.z) return 0;
	alpha_low.z = - FLT_MAX;
	alpha_high.z = + FLT_MAX;
    }

    /* Sort alpha */
    int4 mask = alpha_high > alpha_low;
    float4 tmp = alpha_high;
    alpha_high = select (alpha_low, alpha_high, mask);
    alpha_low = select (tmp, alpha_low, mask);

    /* Check if alpha values overlap in all three dimensions.
       alpha_in is the minimum alpha, where the ray enters the volume.
       alpha_out is where it exits the volume. */
    alpha_in = fmax (alpha_low.x, fmax (alpha_low.y, alpha_low.z));
    alpha_out = fmin (alpha_high.x, fmin (alpha_high.y, alpha_high.z));

    /* If exit is before entrance, the segment does not intersect the volume */
    if (alpha_out - alpha_in < DRR_LEN_TOLERANCE) {
	return 0;
    }

    /* Compute the volume intersection points */
    *ip1 = p1 + alpha_in * ray;
    *ip2 = p1 + alpha_out * ray;

    return 1;
}

float
ray_trace_uniform (
    __global const float *dev_vol, /* Input:  the input volume */
    float4 vol_offset,             /* Input:  volume geometry */
    int4 vol_dim,                  /* Input:  volume resolution */
    float4 vol_spacing,            /* Input:  volume voxel spacing */
    float4 ip1,                    /* Input:  intersection point 1 */
    float4 ip2                     /* Input:  intersection point 2 */
)
{
    float4 ray;
    float step_length = 0.1f;
    float4 inv_spacing = 1.0f / vol_spacing;
    float acc = 0.0f;
    int step;

    ray = normalize (ip2 - ip1);

#define MAX_STEPS 10000

    for (step = 0; step < MAX_STEPS; step++) {
	float4 ipx;
	int4 ai;
	int idx;

	/* Find 3-D location for this sample */
	ipx = ip1 + step * step_length * ray;

	/* Find 3D index of sample within 3D volume */
	ai = convert_int4_sat_rtn (((ipx - vol_offset) 
		+ 0.5 * vol_spacing) * inv_spacing);

	/* Find linear index within 3D volume */
        idx = ((ai.z * vol_dim.y + ai.y) * vol_dim.x) + ai.x;

	if (ai.x >= 0 && ai.y >= 0 && ai.z >= 0 &&
	    ai.x < vol_dim.x && ai.y < vol_dim.y && ai.z < vol_dim.z)
	{
	    acc += step_length * dev_vol[idx];
	}
    }

    return acc;
}

/* GCS Dec 12, 2010.  
   A couple of points to consider when creating OpenCL kernels.
   OpenCL 1.0 does not support int3, float3.
   Maximum guaranteed support for 8 arguments in constant memory.
*/
__kernel void kernel_drr (
    __global float *dev_img,       /* Output: the rendered drr */
    __global const float *dev_vol, /* Input:  the input volume */
    int4 vol_dim,                  /* Input:  volume resolution */
    float4 vol_offset,             /* Input:  volume geometry */
    float4 vol_spacing,            /* Input:  volume voxel spacing */
    int2 img_dim,                  /* Input:  size of output image */
    float2 ic,                     /* Input:  image center */
    int4 img_window,               /* Input:  sub-window of image to render */
    float4 p1,                     /* Input:  3-D loc, source */
    float4 ul_room,                /* Input:  3-D loc, upper-left pix panel */
    float4 incr_r,                 /* Input:  3-D dist between pixels in row */
    float4 incr_c,                 /* Input:  3-D dist between pixels in col */
    float4 nrm,                    /* Input:  normal vector */
    float4 lower_limit,            /* Input:  lower bounding box of volume */
    float4 upper_limit,            /* Input:  upper bounding box of volume */
    const float sad,               /* Input:  source-axis distance */
    const float scale              /* Input:  user defined scale */
) {
    uint id = get_global_id(0);
    int r = id / img_dim.x;
    int c = id - (r * img_dim.x);

    float4 p2;
    float4 ip1, ip2;
    float outval;
    float4 r_tgt, tmp;
    int rc;

    if (r >= img_dim.x) {
	return;
    }

    /* Compute ray */
    r_tgt = ul_room;
    tmp = r * incr_r;
    r_tgt = r_tgt + tmp;
    tmp = c * incr_c;
    p2 = r_tgt + tmp;

    /* Clip ray to volume */
    rc = volume_limit_clip_segment (lower_limit, upper_limit, 
	&ip1, &ip2, p1, p2);
    if (rc == 0) {
	outval = 0.0f;
    } else {
	ip1.w = 0;
	ip2.w = 0;
	outval = ray_trace_uniform (dev_vol, vol_offset, vol_dim, vol_spacing, 
	    ip1, ip2);
    }

    /* Assign output value */
    if (r < img_dim.x && c < img_dim.y) {
	/* Translate from mm voxels to cm*gm */
	outval = 0.1 * outval;
	/* Add to image */
	//dev_img[r*img_dim.x+c] = scale * outval;
	dev_img[id] = scale * outval;
    }
}
