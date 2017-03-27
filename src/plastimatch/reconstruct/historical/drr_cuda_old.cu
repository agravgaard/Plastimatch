/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

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

/*****************
*  C   #includes *
*****************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cuda.h>

#include "drr_cuda.h"
#include "drr_cuda_p.h"
#include "drr_opts.h"
#include "file_util.h"
#include "plm_math.h"
#include "proj_image.h"
#include "volume.h"
#include "timer.h"


// P R O T O T Y P E S ////////////////////////////////////////////////////
void checkCUDAError(const char *msg);
__global__ void kernel_drr_i3 (float * dev_vol,  int2 img_dim, float2 ic, float3 nrm, float sad, float scale, float3 vol_offset, int3 vol_dim, float3 vol_pix_spacing);



// T E X T U R E S ////////////////////////////////////////////////////////
texture<float, 1, cudaReadModeElementType> tex_img;
texture<float, 1, cudaReadModeElementType> tex_matrix;
texture<float, 1, cudaReadModeElementType> tex_coef;
texture<float, 3, cudaReadModeElementType> tex_3Dvol;


// uses 3D textures and pre-calculated coefs to accelerate DRR generation.
void kernel_drr_i3 (float * dev_img, int2 img_dim, float2 ic, float3 nrm, float sad, float scale, float3 vol_offset, int3 vol_dim, float3 vol_pix_spacing)
{
    // CUDA 2.0 does not allow for a 3D grid, which severely
    // limits the manipulation of large 3D arrays of data.  The
    // following code is a hack to bypass this implementation
    // limitation.
    extern __shared__ float sdata[];
    float3 vp;
    int i,j,k;
    int x,y,xy7;
    float vol;

    unsigned int tid = threadIdx.x;

    x = blockIdx.x;
    y = blockIdx.y;
    xy7=7*(y*img_dim.x+x);
	
    if (abs(tex1Dfetch(tex_matrix, 5))>abs(tex1Dfetch(tex_matrix, 4))) {
	vp.x=vol_offset.x+threadIdx.x*vol_pix_spacing.x;
	vp.y=tex1Dfetch(tex_coef, xy7)*vp.x+tex1Dfetch(tex_coef, xy7+1);
	vp.z=tex1Dfetch(tex_coef, xy7+4)*vp.x
	    +tex1Dfetch(tex_coef, xy7+5)*vp.y+tex1Dfetch(tex_coef, xy7+6);

	i=  threadIdx.x;
	j=  __float2int_rd((vp.y-vol_offset.y)/vol_pix_spacing.y);
	k=  __float2int_rd((vp.z-vol_offset.z)/vol_pix_spacing.z);

	//if (j<0||j>=vol_dim.y||k<0||k>=vol_dim.z)
	if ((i-vol_dim.x/2)*(i-vol_dim.x/2)+(j-vol_dim.y/2)*(j-vol_dim.y/2)
	    > vol_dim.y*vol_dim.y/4||k<0||k>=vol_dim.z) 
	{
	    sdata[tid]=0.0f;
	} else {
	    vol=tex3D(tex_3Dvol,i,j,k);
	    sdata[tid]=(vol+1000.0f);
	}
    } else {
	vp.y=vol_offset.y+threadIdx.x*vol_pix_spacing.y;
	vp.x=tex1Dfetch(tex_coef, xy7+2)*vp.y+tex1Dfetch(tex_coef, xy7+3);
	vp.z=tex1Dfetch(tex_coef, xy7+4)*vp.x
	    +tex1Dfetch(tex_coef, xy7+5)*vp.y+tex1Dfetch(tex_coef, xy7+6);
	j=  threadIdx.x;
	i=  __float2int_rd((vp.x-vol_offset.x)/vol_pix_spacing.x);
	k=  __float2int_rd((vp.z-vol_offset.z)/vol_pix_spacing.z);

	if ((i-vol_dim.x/2)*(i-vol_dim.x/2)+(j-vol_dim.y/2)*(j-vol_dim.y/2)
	    > vol_dim.y*vol_dim.y/4||k<0||k>=vol_dim.z)
	{
	    sdata[tid]=0.0f;
	} else {
	    vol=tex3D(tex_3Dvol,i,j,k);
	    sdata[tid]=(vol+1000.0f);
	}
    }

    __syncthreads();

    // do reduction in shared mem
    for(unsigned int s=blockDim.x/2; s>32; s>>=1) 
    {
        if (tid < s)
            sdata[tid] += sdata[tid + s];
        __syncthreads();
    }

#ifndef __DEVICE_EMULATION__
    if (tid < 32)
#endif
    {
        sdata[tid] += sdata[tid + 32]; EMUSYNC;
        sdata[tid] += sdata[tid + 16]; EMUSYNC;
        sdata[tid] += sdata[tid +  8]; EMUSYNC;
        sdata[tid] += sdata[tid +  4]; EMUSYNC;
        sdata[tid] += sdata[tid +  2]; EMUSYNC;
        sdata[tid] += sdata[tid +  1]; EMUSYNC;
    }

    // write result for this block to global mem
    if (tid == 0) 
	dev_img[blockIdx.x*img_dim.y + blockIdx.y] = sdata[0];
}

void*
drr_cuda_state_create (
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
    cudaMalloc ((void**) &state->dev_matrix, 12 * sizeof(float));
    cudaMalloc ((void**) &state->dev_kargs, sizeof(Drr_kernel_args));

    printf ("printf state = %p\n", state);
    printf ("printf state->kargs = %p\n", state->kargs);

    kargs->vol_offset.x = vol->offset[0];
    kargs->vol_offset.y = vol->offset[1];
    kargs->vol_offset.z = vol->offset[2];
    kargs->vol_dim.x = vol->dim[0];
    kargs->vol_dim.y = vol->dim[1];
    kargs->vol_dim.z = vol->dim[2];
    kargs->vol_pix_spacing.x = vol->pix_spacing[0];
    kargs->vol_pix_spacing.y = vol->pix_spacing[1];
    kargs->vol_pix_spacing.z = vol->pix_spacing[2];

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

    cudaMalloc ((void**) &state->dev_img, 
	options->image_resolution[0] * options->image_resolution[1] 
	* sizeof(float));

    cudaMalloc ((void**) &state->dev_coef, 
	7 * options->image_resolution[0] * options->image_resolution[1] 
	* sizeof(float));
    checkCUDAError ("Unable to allocate coef devmem");
    state->host_coef = (float*) malloc (
	7 * options->image_resolution[0] * options->image_resolution[1] 
	* sizeof(float));
		
    return (void*) state;
}

void
drr_cuda_state_destroy (
    void *void_state
)
{
    Drr_cuda_state *state = (Drr_cuda_state*) void_state;
    
    cudaFree (state->dev_img);
    cudaFree (state->dev_kargs);
    cudaFree (state->dev_matrix);
    cudaFree (state->dev_coef);
    free (state->host_coef);
    free (state->kargs);
}

void
drr_cuda_render_volume_perspective (
    Proj_image *proj,
    void *void_state,
    Volume *vol, 
    double ps[2], 
    char *multispectral_fn, 
    Drr_options *options
)
{
    Timer timer, total_timer;
    double time_kernel = 0;
    double time_io = 0;
    int i;

    // CUDA device pointers
    Drr_cuda_state *state = (Drr_cuda_state*) void_state;
    Drr_kernel_args *kargs = state->kargs;
    float *host_coef = state->host_coef;

    // Start the timer
    plm_timer_start (&total_timer);
    plm_timer_start (&timer);

    // Load dynamic kernel arguments
    kargs->img_dim.x = proj->dim[0];
    kargs->img_dim.y = proj->dim[1];
    kargs->ic.x = proj->pmat->ic[0];
    kargs->ic.y = proj->pmat->ic[1];
    kargs->nrm.x = proj->pmat->nrm[0];
    kargs->nrm.y = proj->pmat->nrm[1];
    kargs->nrm.z = proj->pmat->nrm[2];
    kargs->sad = proj->pmat->sad;
    kargs->sid = proj->pmat->sid;
    for (i = 0; i < 12; i++) {
	kargs->matrix[i] = (float) proj->pmat->matrix[i];
    }

    // Precalculate coeff
    int xy7;
    double *matrix = proj->pmat->matrix;
    for (int x = 0; x < proj->dim[0] ; x++) {
	for (int y = 0; y < proj->dim[1] ; y++) {
	    xy7 = 7 * (y * proj->dim[0] + x);

#if defined (commentout)
	    host_coef[xy7]  =((y-ic[1])*proj->pmat->matrix[8]-proj->pmat->matrix[4])/(proj->pmat->matrix[5]-(y-ic[1])*proj->pmat->matrix[9]);
	    host_coef[xy7+2]=((y-ic[1])*proj->pmat->matrix[9]-proj->pmat->matrix[5])/(proj->pmat->matrix[4]-(y-ic[1])*proj->pmat->matrix[8]);
	    host_coef[xy7+1]=(y-ic[1])*proj->pmat->matrix[11]/(proj->pmat->matrix[5]-(y-ic[1])*proj->pmat->matrix[9]);
	    host_coef[xy7+3]=(y-ic[1])*proj->pmat->matrix[11]/(proj->pmat->matrix[4]-(y-ic[1])*proj->pmat->matrix[8]);
	    host_coef[xy7+4]=(x-ic[0])*proj->pmat->matrix[8]/proj->pmat->matrix[2];
	    host_coef[xy7+5]=(x-ic[0])*proj->pmat->matrix[9]/proj->pmat->matrix[2];
	    host_coef[xy7+6]=(x-ic[0])*proj->pmat->matrix[11]/proj->pmat->matrix[2];
#endif

	    host_coef[xy7]   = (y * matrix[8] - matrix[4])
		/ (matrix[5] - y * matrix[9]);
	    host_coef[xy7+2] = (y * matrix[9] - matrix[5])
		/ (matrix[4] - y * matrix[8]);
	    host_coef[xy7+1] = y * matrix[11]
		/ (matrix[5] - y * matrix[9]);
	    host_coef[xy7+3] = y * matrix[11]
		/ (matrix[4] - y * matrix[8]);
	    host_coef[xy7+4] = x * matrix[8] / matrix[2];
	    host_coef[xy7+5] = x * matrix[9] / matrix[2];
	    host_coef[xy7+6] = x * matrix[11] / matrix[2];
	}
    }

    time_io += plm_timer_report (&timer);
    plm_timer_start (&timer);

    cudaMemcpy (state->dev_matrix, kargs->matrix, sizeof(kargs->matrix), 
	cudaMemcpyHostToDevice);

    cudaBindTexture (0, tex_matrix, state->dev_matrix, sizeof(kargs->matrix));

    cudaMemcpy (state->dev_coef, host_coef, 
	7 * proj->dim[0] * proj->dim[1] * sizeof(float), 
	cudaMemcpyHostToDevice);

    cudaBindTexture (0, tex_coef, state->dev_coef, 
	7 * proj->dim[0] * proj->dim[1] * sizeof(float));

    // Thead Block Dimensions
    int tBlock_x = vol->dim[0];
    int tBlock_y = 1;
    int tBlock_z = 1;

    // Each element in the volume (each voxel) gets 1 thread
    int blocksInX = proj->dim[0];
    int blocksInY = proj->dim[1];
    dim3 dimGrid  = dim3(blocksInX, blocksInY);
    dim3 dimBlock = dim3(tBlock_x, tBlock_y, tBlock_z);

    // Invoke ze kernel  \(^_^)/
    // Note: proj->img AND proj->matrix are passed via texture memory

    int smemSize = vol->dim[0]  * sizeof(float);

    plm_timer_start (&timer);

    //-------------------------------------
    kernel_drr_i3<<< dimGrid, dimBlock, smemSize>>> (
	state->dev_img, 
	kargs->img_dim,
	kargs->ic,
	kargs->nrm,
	kargs->sad,
	kargs->scale,
	kargs->vol_offset,
	kargs->vol_dim,
	kargs->vol_pix_spacing);

    checkCUDAError("Kernel Panic!");

#if defined (TIME_KERNEL)
    // CUDA kernel calls are asynchronous...
    // In order to accurately time the kernel
    // execution time we need to set a thread
    // barrier here after its execution.
    cudaThreadSynchronize();
#endif

    time_kernel += plm_timer_report (&timer);

    // Unbind the image and projection matrix textures
    //cudaUnbindTexture( tex_img );
    cudaUnbindTexture (tex_matrix);
    cudaUnbindTexture (tex_coef);

    // Copy reconstructed volume from device to host
    //cudaMemcpy( vol->img, dev_vol, vol->npix * vol->pix_size, cudaMemcpyDeviceToHost );
    cudaMemcpy (proj->img, state->dev_img, 
	proj->dim[0] * proj->dim[1] * sizeof(float), 
	cudaMemcpyDeviceToHost);
    checkCUDAError("Error: Unable to retrieve data volume.");
}


///////////////////////////////////////////////////////////////////////////
// FUNCTION: checkCUDAError() /////////////////////////////////////////////
void checkCUDAError(const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if ( cudaSuccess != err) 
    {
        fprintf(stderr, "CUDA ERROR: %s (%s).\n", msg, cudaGetErrorString( err) );
        exit(EXIT_FAILURE);
    }                         
}
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
// Vim Editor Settings ////////////////////////////////////////////////////
// vim:ts=8:sw=8:cindent:nowrap
///////////////////////////////////////////////////////////////////////////
