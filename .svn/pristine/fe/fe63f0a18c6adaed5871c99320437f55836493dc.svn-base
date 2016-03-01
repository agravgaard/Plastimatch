/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmreconstruct_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cuda.h>

#include "cuda_util.h"
#include "drr.h"
#include "drr_cuda.h"
#include "drr_cuda_p.h"
#include "plm_cuda_math.h"
#include "plm_math.h"
#include "proj_image.h"
#include "proj_matrix.h"
#include "volume.h"
#include "volume_limit.h"

/****************************************************\
* Uncomment the line below to enable verbose output. *
* Enabling this should not nerf performance.         *
\****************************************************/
#define VERBOSE 1

/**********************************************************\
* Uncomment the line below to enable detailed performance  *
* reporting.  This measurement alters the system, however, *
* resulting in significantly slower kernel execution.      *
\**********************************************************/
#define TIME_KERNEL
#ifdef __DEVICE_EMULATION__
#define EMUSYNC __syncthreads()
#else
#define EMUSYNC
#endif

/* Textures */
//texture<float, 1, cudaReadModeElementType> tex_img;
//texture<float, 1, cudaReadModeElementType> tex_matrix;
//texture<float, 3, cudaReadModeElementType> tex_vol;
texture<float, 1, cudaReadModeElementType> tex_vol;

#define DRR_LEN_TOLERANCE 1e-6

/* From volume_limit.c */
__device__ int
volume_limit_clip_segment (
    float3 lower_limit,         /* INPUT:  The bounding box to clip to */
    float3 upper_limit,         /* INPUT:  The bounding box to clip to */
    float3 *ip1,                /* OUTPUT: Intersection point 1 */
    float3 *ip2,                /* OUTPUT: Intersection point 2 */
    float3 p1,                  /* INPUT:  Line segment point 1 */
    float3 p2                   /* INPUT:  Line segment point 2 */
)
{
    float3 ray, inv_ray;
    float alpha_in, alpha_out;
    float3 alpha_low, alpha_high;
    int3 ploc;
    int3 is_parallel;

    ray = p2 - p1;
    inv_ray = 1.0f / ray;

    /* Find intersection configuration of ray base */
    /* -1 is POINTLOC_LEFT, 0 is POINTLOC_INSIDE, 1 is POINTLOC_RIGHT */
    ploc = make_int3 (-1, -1, -1);
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
    is_parallel = fabsf3(ray) < DRR_LEN_TOLERANCE;

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
    sortf3 (&alpha_low, &alpha_high);

    /* Check if alpha values overlap in all three dimensions.
       alpha_in is the minimum alpha, where the ray enters the volume.
       alpha_out is where it exits the volume. */
    alpha_in = fmaxf(alpha_low.x, fmaxf (alpha_low.y, alpha_low.z));
    alpha_out = fminf(alpha_high.x, fminf (alpha_high.y, alpha_high.z));

    /* If exit is before entrance, the segment does not intersect the volume */
    if (alpha_out - alpha_in < DRR_LEN_TOLERANCE) {
	return 0;
    }

    /* Compute the volume intersection points */
    *ip1 = p1 + alpha_in * ray;
    *ip2 = p1 + alpha_out * ray;

    return 1;
}

/* From volume_limit.c */
__device__ 
float
ray_trace_uniform (
    float *dev_vol,           /* Output: the rendered drr */
    float3 vol_offset,        /* Input:  volume geometry */
    int3 vol_dim,             /* Input:  volume resolution */
    float3 vol_spacing,       /* Input:  volume voxel spacing */
    float3 ip1,               /* Input:  intersection point 1 */
    float3 ip2                /* Input:  intersection point 2 */
)
{
    float3 ray = normalize (ip2 - ip1);
    float step_length = 0.1f;
    float3 inv_spacing = 1.0f / vol_spacing;
    float acc = 0.0f;
    int step;

#define MAX_STEPS 10000

    for (step = 0; step < MAX_STEPS; step++) {
	float3 ipx;
	int3 ai;
	int idx;

	/* Find 3-D location for this sample */
	ipx = ip1 + step * step_length * ray;

	/* Find 3D index of sample within 3D volume */
	ai = make_int3 (floorf3 (((ipx - vol_offset) 
		    + 0.5 * vol_spacing) * inv_spacing));

	/* Find linear index within 3D volume */
        idx = ((ai.z * vol_dim.y + ai.y) * vol_dim.x) + ai.x;

	if (ai.x >= 0 && ai.y >= 0 && ai.z >= 0 &&
	    ai.x < vol_dim.x && ai.y < vol_dim.y && ai.z < vol_dim.z)
	{
	    acc += step_length * tex1Dfetch (tex_vol, idx);
	}
    }
    return acc;
}

/* Main DRR function */
__global__ void
kernel_drr (
    float *dev_img,           /* Output: the rendered drr */
    int2 img_dim,             /* Input:  size of output image */
    float *dev_vol,           /* Input:  the input volume */
    float2 ic,                /* Input:  image center */
    float3 nrm,               /* Input:  normal vector */
    float sad,                /* Input:  source-axis distance */
    float scale,              /* Input:  user defined scale */
    float3 p1,                /* Input:  3-D loc, source */
    float3 ul_room,           /* Input:  3-D loc, upper-left pixel of panel */
    float3 incr_r,            /* Input:  3-D distance between pixels in row */
    float3 incr_c,            /* Input:  3-D distance between pixels in col */
    int4 image_window,        /* Input:  sub-window of image to render */
    float3 lower_limit,       /* Input:  lower bounding box of volume */
    float3 upper_limit,       /* Input:  upper bounding box of volume */
    float3 vol_offset,        /* Input:  volume geometry */
    int3 vol_dim,             /* Input:  volume resolution */
    float3 vol_spacing        /* Input:  volume voxel spacing */
)
{
    extern __shared__ float sdata[];

    float3 p2;
    float3 ip1, ip2;
    int r, c;
    float outval;
    float3 r_tgt, tmp;

    /* Get coordinates of this image pixel */
    c = blockIdx.x * blockDim.x + threadIdx.x;
    r = blockIdx.y * blockDim.y + threadIdx.y;

    /* Compute ray */
    r_tgt = ul_room;
    tmp = r * incr_r;
    r_tgt = r_tgt + tmp;
    tmp = c * incr_c;
    p2 = r_tgt + tmp;

    /* Compute output location */
    //cols = image_window.w - image_window.z + 1;
    //idx = (c - image_window.z) + (r - image_window.x) * cols;

    /* Clip ray to volume */
    if (volume_limit_clip_segment (lower_limit, upper_limit, 
	    &ip1, &ip2, p1, p2) == 0)
    {
	outval = 0.0f;
    } else {
	outval = ray_trace_uniform (dev_vol, vol_offset, vol_dim, vol_spacing, 
	    ip1, ip2);
    }

    /* Write output pixel value */
    if (r < img_dim.y && c < img_dim.x) {
	/* Translate from mm voxels to cm*gm */
	outval = 0.1 * outval;
	/* Add to image */
	dev_img[r*img_dim.x + c] = scale * outval;
    }
}

void*
drr_cuda_state_create_cu (
    Proj_image *proj,
    Volume *vol,
    Drr_options *options
)
{
    Drr_cuda_state *state;
    Drr_kernel_args *kargs;

    state = (Drr_cuda_state *) malloc (sizeof(Drr_cuda_state));
    memset (state, 0, sizeof(Drr_cuda_state));

    state->kargs = kargs = (Drr_kernel_args*) malloc (sizeof(Drr_kernel_args));
    //cudaMalloc ((void**) &state->dev_matrix, 12 * sizeof(float));
    cudaMalloc ((void**) &state->dev_kargs, sizeof(Drr_kernel_args));

    kargs->vol_origin = make_float3 (vol->origin);
    kargs->vol_dim = make_int3 (vol->dim);
    kargs->vol_spacing = make_float3 (vol->spacing);

#if defined (commentout)
    /* The below code is Junan's.  Presumably this way can be better 
       for using hardware linear interpolation, but for now I'm going 
       to follow Tony's method. */
    // prepare texture
    cudaChannelFormatDesc ca_descriptor;
    cudaExtent ca_extent;
    cudaArray *dev_3Dvol=0;

    ca_descriptor = cudaCreateChannelDesc<float>();
    ca_extent.width  = vol->dim[0];
    ca_extent.height = vol->dim[1];
    ca_extent.depth  = vol->dim[2];
    cudaMalloc3DArray (&dev_3Dvol, &ca_descriptor, ca_extent);
    cudaBindTextureToArray (tex_3Dvol, dev_3Dvol, ca_descriptor);

    cudaMemcpy3DParms cpy_params = {0};
    cpy_params.extent   = ca_extent;
    cpy_params.kind     = cudaMemcpyHostToDevice;
    cpy_params.dstArray = dev_3Dvol;

    //http://sites.google.com/site/cudaiap2009/cookbook-1#TOC-CUDA-3D-Texture-Example-Gerald-Dall
    // The pitched pointer is really tricky to get right. We give the
    // pitch of a row, then the number of elements in a row, then the
    // height, and we omit the 3rd dimension.
    cpy_params.srcPtr = make_cudaPitchedPtr ((void*)vol->img, 
	ca_extent.width * sizeof(float), ca_extent.width , ca_extent.height);

    cudaMemcpy3D (&cpy_params);
#endif

    cudaMalloc ((void**) &state->dev_vol, vol->npix * sizeof (float));
    CUDA_check_error ("Failed to allocate dev_vol.");
    cudaMemcpy (state->dev_vol, vol->img, vol->npix * sizeof (float), 
	cudaMemcpyHostToDevice);
    CUDA_check_error ("Failed to memcpy dev_vol host to device.");
    cudaBindTexture (0, tex_vol, state->dev_vol, vol->npix * sizeof (float));
    CUDA_check_error ("Failed to bind state->dev_vol to texture.");

    cudaMalloc ((void**) &state->dev_img, 
	options->image_resolution[0] * options->image_resolution[1] 
	* sizeof(float));
    CUDA_check_error ("Failed to allocate dev_img.\n");

    return (void*) state;
}

void
drr_cuda_state_destroy_cu (
    void *void_state
)
{
    Drr_cuda_state *state = (Drr_cuda_state*) void_state;
    
    cudaUnbindTexture (tex_vol);
    cudaFree (state->dev_vol);
    cudaFree (state->dev_img);
    cudaFree (state->dev_kargs);
    //cudaFree (state->dev_matrix);
    free (state->kargs);
}

void
drr_cuda_ray_trace_image (
    Proj_image *proj, 
    Volume *vol, 
    Volume_limit *vol_limit, 
    double p1[3], 
    double ul_room[3], 
    double incr_r[3], 
    double incr_c[3], 
    void *dev_state, 
    Drr_options *options
)
{
    // CUDA device pointers
    Drr_cuda_state *state = (Drr_cuda_state*) dev_state;
    Drr_kernel_args *kargs = state->kargs;

    // Start the timer
    //plm_timer_start (&timer);

    // Load dynamic kernel arguments (different for each projection)
    kargs->img_dim.x = proj->dim[0];
    kargs->img_dim.y = proj->dim[1];
    kargs->ic.x = proj->pmat->ic[0];
    kargs->ic.y = proj->pmat->ic[1];
    kargs->nrm.x = proj->pmat->nrm[0];
    kargs->nrm.y = proj->pmat->nrm[1];
    kargs->nrm.z = proj->pmat->nrm[2];
    kargs->sad = proj->pmat->sad;
    kargs->sid = proj->pmat->sid;
    //for (i = 0; i < 12; i++) {
    //kargs->matrix[i] = (float) proj->pmat->matrix[i];
    //}
    kargs->p1.x = p1[0];
    kargs->p1.y = p1[1];
    kargs->p1.z = p1[2];
    kargs->ul_room.x = ul_room[0];
    kargs->ul_room.y = ul_room[1];
    kargs->ul_room.z = ul_room[2];
    kargs->incr_r = make_float3 (incr_r);
    kargs->incr_c = make_float3 (incr_c);
    kargs->image_window = make_int4 (options->image_window);
    kargs->lower_limit = make_float3 (vol_limit->lower_limit);
    kargs->upper_limit = make_float3 (vol_limit->upper_limit);
    kargs->scale = options->scale;

    //cudaMemcpy (state->dev_matrix, kargs->matrix, sizeof(kargs->matrix), 
    //cudaMemcpyHostToDevice);
    //cudaBindTexture (0, tex_matrix, state->dev_matrix, sizeof(kargs->matrix));

    // Thread Block Dimensions
    int tBlock_x = 16;
    int tBlock_y = 16;

    // Each element in the image gets 1 thread
    int blocksInX = (proj->dim[0]+tBlock_x-1)/tBlock_x;
    int blocksInY = (proj->dim[1]+tBlock_y-1)/tBlock_y;
    dim3 dimGrid  = dim3(blocksInX, blocksInY);
    dim3 dimBlock = dim3(tBlock_x, tBlock_y);

    //int smemSize = vol->dim[0]  * sizeof(float);

    //printf ("Preprocessing time: %f secs\n", plm_timer_report (&timer));
    //plm_timer_start (&timer);

    // Invoke ze kernel  \(^_^)/
    kernel_drr<<< dimGrid, dimBlock >>> (
	state->dev_img, 
	kargs->img_dim, 
	state->dev_vol, 
	kargs->ic,
	kargs->nrm,
	kargs->sad,
	kargs->scale, 
	kargs->p1, 
	kargs->ul_room, 
	kargs->incr_r, 
	kargs->incr_c, 
	kargs->image_window, 
	kargs->lower_limit,
	kargs->upper_limit,
	kargs->vol_origin,
	kargs->vol_dim,
	kargs->vol_spacing);
    CUDA_check_error ("Kernel Panic!");

#if defined (TIME_KERNEL)
    // CUDA kernel calls are asynchronous...
    // In order to accurately time the kernel
    // execution time we need to set a thread
    // barrier here after its execution.
    cudaThreadSynchronize();
#endif

    //cudaThreadSynchronize();
    //printf ("Kernel time: %f secs\n", plm_timer_report (&timer));

    // Copy reconstructed volume from device to host
    cudaMemcpy (proj->img, state->dev_img, 
	proj->dim[0] * proj->dim[1] * sizeof(float), 
	cudaMemcpyDeviceToHost);
    CUDA_check_error("Error: Unable to retrieve data volume.");
}
