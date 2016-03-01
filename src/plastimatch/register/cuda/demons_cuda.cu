/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <cuda.h>

#include "cuda_util.h"
#include "demons.h"
#include "demons_cuda.h"
#include "demons_state.h"
#include "gaussian.h"
#include "plm_cuda_math.h"
#include "plm_timer.h"
#include "volume.h"

/*
Constants
*/
#define BLOCK_SIZE 256


/*
Texture Memory
*/
texture<float, 1, cudaReadModeElementType> tex_fixed;
texture<float, 1, cudaReadModeElementType> tex_moving;
texture<float, 1, cudaReadModeElementType> tex_grad;
texture<float, 1, cudaReadModeElementType> tex_grad_mag;
texture<float, 1, cudaReadModeElementType> tex_vf_est;
texture<float, 1, cudaReadModeElementType> tex_vf_smooth;


/*
Constant Memory
*/
__constant__ int c_dim[3];
__constant__ int c_moving_dim[3];
__constant__ float c_spacing_div2[3];
__constant__ float c_f2mo[3];
__constant__ float c_f2ms[3];
__constant__ float c_invmps[3];


/*
Constant Memory Functions
*/
void 
setConstantDimension (plm_long *h_dim)
{
    int i_dim[3] = { h_dim[0], h_dim[1], h_dim[2] };
    cudaMemcpyToSymbol (c_dim, i_dim, sizeof(int3));
    //cudaMemcpyToSymbol(c_dim, h_dim, sizeof(int3));
}

void 
setConstantMovingDimension (plm_long *h_dim)
{
    int i_dim[3] = { h_dim[0], h_dim[1], h_dim[2] };
    cudaMemcpyToSymbol (c_moving_dim, i_dim, sizeof(int3));
}

void setConstantPixelSpacing(float *h_spacing_div2)
{
	cudaMemcpyToSymbol(c_spacing_div2, h_spacing_div2, sizeof(float3));
}

void setConstantF2mo(float *h_f2mo)
{
	cudaMemcpyToSymbol(c_f2mo, h_f2mo, sizeof(float3));
}

void setConstantF2ms(float *h_f2ms)
{
	cudaMemcpyToSymbol(c_f2ms, h_f2ms, sizeof(float3));
}

void setConstantInvmps(float *h_invmps)
{
	cudaMemcpyToSymbol(c_invmps, h_invmps, sizeof(float3));
}


/*
Device Functions
*/
__device__ int volume_index_cuda (int *dims, int i, int j, int k)
{
	return i + (dims[0] * (j + dims[1] * k));
}


/*
Kernels
*/
__global__ void calculate_gradient_magnitude_image_kernel (float *grad_mag, int blockY, float invBlockY)
{
	/* Find position in volume */
	int blockIdx_z = __float2int_rd(blockIdx.y * invBlockY);
	int blockIdx_y = blockIdx.y - __mul24(blockIdx_z, blockY);
	int x = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
	int y = __mul24(blockIdx_y, blockDim.y) + threadIdx.y;
	int z = __mul24(blockIdx_z, blockDim.z) + threadIdx.z;

	if (x >= c_dim[0] || y >= c_dim[1] || z >= c_dim[2])
		return;

	long v = (z * c_dim[1] * c_dim[0]) + (y * c_dim[0]) + x;
	long v3 = v * 3;

	float vox_grad_x = tex1Dfetch(tex_grad, v3);
	float vox_grad_y = tex1Dfetch(tex_grad, v3 + 1);
	float vox_grad_z = tex1Dfetch(tex_grad, v3 + 2);

	grad_mag[v] = vox_grad_x * vox_grad_x + vox_grad_y * vox_grad_y + vox_grad_z * vox_grad_z;
}

__global__ void 
estimate_kernel (
    float *vf_est_img, 
    float *ssd, 
    int *inliers, 
    float homog, 
    float denominator_eps, 
    float accel, 
    int blockY, 
    float invBlockY
)
{
    /* Find position in volume */
    int blockIdx_z = __float2int_rd(blockIdx.y * invBlockY);
    int blockIdx_y = blockIdx.y - __mul24(blockIdx_z, blockY);
    int i = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
    int j = __mul24(blockIdx_y, blockDim.y) + threadIdx.y;
    int k = __mul24(blockIdx_z, blockDim.z) + threadIdx.z;

    if (i >= c_dim[0] || j >= c_dim[1] || k >= c_dim[2])
	return;

    long fv = (k * c_dim[1] * c_dim[0]) + (j * c_dim[0]) + i;
    long f3v = 3 * fv;

    float mi = c_f2mo[0] + i * c_f2ms[0];
    float mj = c_f2mo[1] + j * c_f2ms[1];
    float mk = c_f2mo[2] + k * c_f2ms[2];

    /* Find correspondence with nearest neighbor interpolation 
       and boundary checking */
    int mz = __float2int_rn (mk + c_invmps[2] 
	* tex1Dfetch(tex_vf_smooth, f3v + 2));	/* pixels (moving) */
    if (mz < 0 || mz >= c_moving_dim[2])
	return;

    int my = __float2int_rn (mj + c_invmps[1] 
	* tex1Dfetch(tex_vf_smooth, f3v + 1));	/* pixels (moving) */
    if (my < 0 || my >= c_moving_dim[1])
	return;

    int mx = __float2int_rn (mi + c_invmps[0] 
	* tex1Dfetch(tex_vf_smooth, f3v));		/* pixels (moving) */
    if (mx < 0 || mx >= c_moving_dim[0])
	return;

    int mv = (mz * c_moving_dim[1] + my) * c_moving_dim[0] + mx;
    int m3v = 3 * mv;

    /* Find image difference at this correspondence */
    float diff = tex1Dfetch(tex_fixed, fv) - tex1Dfetch(tex_moving, mv);		/* intensity */

    /* Compute denominator */
    float denom = tex1Dfetch(tex_grad_mag, mv) + homog * diff * diff;		/* intensity^2 per mm^2 */

    /* Compute SSD for statistics */
    inliers[fv] = 1;
    ssd[fv] = diff * diff;

    /* Threshold the denominator to stabilize estimation */
    if (denom < denominator_eps) 
	return;

    /* Compute new estimate of displacement */
    float mult = accel * diff / denom;					/* per intensity^2 */
    vf_est_img[f3v] += mult * tex1Dfetch(tex_grad, m3v);			/* mm */
    vf_est_img[f3v + 1] += mult * tex1Dfetch(tex_grad, m3v + 1);
    vf_est_img[f3v + 2] += mult * tex1Dfetch(tex_grad, m3v + 2);
}

template <class T> __global__ void reduction(T *vectorData, int totalElements)
{
	__shared__ T vector[BLOCK_SIZE * 2];

	/* Find position in vector */
	int threadID = threadIdx.x;
	int blockID = blockIdx.x;
	int xInVector = BLOCK_SIZE * blockID * 2 + threadID;

	vector[threadID] = (xInVector < totalElements) ? vectorData[xInVector] : 0;
	vector[threadID + BLOCK_SIZE] = (xInVector + BLOCK_SIZE < totalElements) ? vectorData[xInVector + BLOCK_SIZE] : 0;
	__syncthreads();

	/* Calculate partial sum */
	for (int stride = BLOCK_SIZE; stride > 0; stride >>= 1) {
		if (threadID < stride)
			vector[threadID] += vector[threadID + stride];
		__syncthreads();
	}
	__syncthreads();

	if (threadID == 0)
		vectorData[blockID] = vector[0];
}

__global__ void vf_convolve_x_kernel (float *vf_out, float *ker, int half_width, int blockY, float invBlockY)
{
	int i, i1;		/* i is the offset in the vf */
	int j, j1, j2;	/* j is the index of the kernel */
	int d;			/* d is the vector field direction */

	/* Find position in volume */
	int blockIdx_z = __float2int_rd(blockIdx.y * invBlockY);
	int blockIdx_y = blockIdx.y - __mul24(blockIdx_z, blockY);
	int x = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
	int y = __mul24(blockIdx_y, blockDim.y) + threadIdx.y;
	int z = __mul24(blockIdx_z, blockDim.z) + threadIdx.z;

	if (x >= c_dim[0] || y >= c_dim[1] || z >= c_dim[2])
		return;

	long v3 = 3 * ((z * c_dim[1] * c_dim[0]) + (y * c_dim[0]) + x);

	j1 = x - half_width;
	j2 = x + half_width;
	if (j1 < 0) j1 = 0;
	if (j2 >= c_dim[0]) {
		j2 = c_dim[0] - 1;
	}
	i1 = j1 - x;
	j1 = j1 - x + half_width;
	j2 = j2 - x + half_width;

	long index;
	for (d = 0; d < 3; d++) {
		float sum = 0.0;
		for (i = i1, j = j1; j <= j2; i++, j++) {
			index = v3 + (3 * i) + d;
			sum += ker[j] * tex1Dfetch(tex_vf_est, index);
		}
		vf_out[v3 + d] = sum;
	}
}

__global__ void vf_convolve_y_kernel (float *vf_out, float *ker, int half_width, int blockY, float invBlockY)
{
	int i, i1;		/* i is the offset in the vf */
	int j, j1, j2;	/* j is the index of the kernel */
	int d;			/* d is the vector field direction */

	/* Find position in volume */
	int blockIdx_z = __float2int_rd(blockIdx.y * invBlockY);
	int blockIdx_y = blockIdx.y - __mul24(blockIdx_z, blockY);
	int x = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
	int y = __mul24(blockIdx_y, blockDim.y) + threadIdx.y;
	int z = __mul24(blockIdx_z, blockDim.z) + threadIdx.z;

	if (x >= c_dim[0] || y >= c_dim[1] || z >= c_dim[2])
		return;

	long v3 = 3 * ((z * c_dim[1] * c_dim[0]) + (y * c_dim[0]) + x);

	j1 = y - half_width;
	j2 = y + half_width;
	if (j1 < 0) j1 = 0;
	if (j2 >= c_dim[1]) {
		j2 = c_dim[1] - 1;
	}
	i1 = j1 - y;
	j1 = j1 - y + half_width;
	j2 = j2 - y + half_width;

	long index;
	for (d = 0; d < 3; d++) {
		float sum = 0.0;
		for (i = i1, j = j1; j <= j2; i++, j++) {
			index = v3 + (3 * i * c_dim[0]) + d;
			sum += ker[j] * tex1Dfetch(tex_vf_smooth, index);
		}
		vf_out[v3 + d] = sum;
	}
}

__global__ void vf_convolve_z_kernel (float *vf_out, float *ker, int half_width, int blockY, float invBlockY)
{
	int i, i1;		/* i is the offset in the vf */
	int j, j1, j2;	/* j is the index of the kernel */
	int d;			/* d is the vector field direction */

	/* Find position in volume */
	int blockIdx_z = __float2int_rd(blockIdx.y * invBlockY);
	int blockIdx_y = blockIdx.y - __mul24(blockIdx_z, blockY);
	int x = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
	int y = __mul24(blockIdx_y, blockDim.y) + threadIdx.y;
	int z = __mul24(blockIdx_z, blockDim.z) + threadIdx.z;

	if (x >= c_dim[0] || y >= c_dim[1] || z >= c_dim[2])
		return;

	long v3 = 3 * ((z * c_dim[1] * c_dim[0]) + (y * c_dim[0]) + x);

	j1 = z - half_width;
	j2 = z + half_width;
	if (j1 < 0) j1 = 0;
	if (j2 >= c_dim[2]) {
		j2 = c_dim[2] - 1;
	}
	i1 = j1 - z;
	j1 = j1 - z + half_width;
	j2 = j2 - z + half_width;

	long index;
	for (d = 0; d < 3; d++) {
		float sum = 0.0;
		for (i = i1, j = j1; j <= j2; i++, j++) {
			index = v3 + (3 * i * c_dim[0] * c_dim[1]) + d;
			sum += ker[j] * tex1Dfetch(tex_vf_est, index);
		}
		vf_out[v3 + d] = sum;
	}
}

__global__ void volume_calc_grad_kernel (float *out_img, unsigned int blockY, float invBlockY)
{
	/* Find position in volume */
	int blockIdx_z = __float2int_rd(blockIdx.y * invBlockY);
	int blockIdx_y = blockIdx.y - __mul24(blockIdx_z, blockY);
	int i = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;
	int j = __mul24(blockIdx_y, blockDim.y) + threadIdx.y;
	int k = __mul24(blockIdx_z, blockDim.z) + threadIdx.z;

	if (i >= c_dim[0] || j >= c_dim[1] || k >= c_dim[2])
		return;

	/* p is prev, n is next */
	int i_p = (i == 0) ? 0 : i - 1;
	int i_n = (i == c_dim[0] - 1) ? c_dim[0] - 1 : i + 1;
	int j_p = (j == 0) ? 0 : j - 1;
	int j_n = (j == c_dim[1] - 1) ? c_dim[1] - 1 : j + 1;
	int k_p = (k == 0) ? 0 : k - 1;
	int k_n = (k == c_dim[2] - 1) ? c_dim[2] - 1 : k + 1;

	long v3 = 3 * ((k * c_dim[1] * c_dim[0]) + (j * c_dim[0]) + i);

	long gi = v3;
	long gj = v3 + 1;
	long gk = v3 + 2;

	int idx_p, idx_n;
	idx_p = volume_index_cuda (c_dim, i_p, j, k);
	idx_n = volume_index_cuda (c_dim, i_n, j, k);
	out_img[gi] = (float) (tex1Dfetch(tex_moving, idx_n) - tex1Dfetch(tex_moving, idx_p)) * c_spacing_div2[0];

	idx_p = volume_index_cuda (c_dim, i, j_p, k);
	idx_n = volume_index_cuda (c_dim, i, j_n, k);
	out_img[gj] = (float) (tex1Dfetch(tex_moving, idx_n) - tex1Dfetch(tex_moving, idx_p)) * c_spacing_div2[1];

	idx_p = volume_index_cuda (c_dim, i, j, k_p);
	idx_n = volume_index_cuda (c_dim, i, j, k_n);
	out_img[gk] = (float) (tex1Dfetch(tex_moving, idx_n) - tex1Dfetch(tex_moving, idx_p)) * c_spacing_div2[2];
}

//Volume* 
void
demons_cuda (
    Demons_state *demons_state,
    Volume* fixed, 
    Volume* moving, 
    Volume* moving_grad, 
    Volume* vf_init, 
    Demons_parms* parms
)
{
    int i;
    int	it;			/* Iterations */
    float f2mo[3];		/* Origin difference (in cm) from fixed to moving */
    float f2ms[3];		/* Slope to convert fixed to moving */
    float invmps[3];		/* 1/pixel spacing of moving image */
    float *kerx, *kery, *kerz;
    int fw[3];
    double diff_run, gpu_time, kernel_time;
    //Volume *vf_est, *vf_smooth;
    int inliers;
    float ssd;

    Plm_timer* timer = new Plm_timer;
    Plm_timer* gpu_timer = new Plm_timer;
    Plm_timer* kernel_timer = new Plm_timer;

    int threadX, threadY, threadZ, blockX, blockY, blockZ, num_elements, half_num_elements, reductionBlocks;
	size_t vol_size, interleaved_vol_size, inlier_size;
    int *d_inliers;
    float total_runtime, spacing_div2[3];
    float *d_vf_est, *d_vf_smooth, *d_moving, *d_fixed, *d_m_grad, *d_m_grad_mag, *d_kerx, *d_kery, *d_kerz, *d_swap, *d_ssd;
    dim3 block, grid, reductionGrid;

#if defined (commentout)
    /* Allocate memory for vector fields */
    if (vf_init) {
	/* If caller has an initial estimate, we copy it */
	vf_smooth = volume_clone(vf_init);
	vf_convert_to_interleaved(vf_smooth);
    } else {
	/* Otherwise initialize to zero */
	vf_smooth = new Volume (fixed->dim, fixed->origin, fixed->spacing, 
	    fixed->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3, 0);
    }
    vf_est = new Volume (fixed->dim, fixed->origin, fixed->spacing, 
	fixed->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3, 0);
#endif

    printf ("Hello from demons_cuda()\n");

    /* Initialize GPU timers */
    gpu_time = 0;
    kernel_time = 0;
	
    /* Determine GPU execution environment */
    threadX = BLOCK_SIZE;
    threadY = 1;
    threadZ = 1;
    blockX = (fixed->dim[0] + threadX - 1) / threadX;
    blockY = (fixed->dim[1] + threadY - 1) / threadY;
    blockZ = (fixed->dim[2] + threadZ - 1) / threadZ;
    block = dim3(threadX, threadY, threadZ);
    grid = dim3(blockX, blockY * blockZ);

    /*
      Calculate Moving Gradient
    */
    for (i = 0; i < 3; i++)
	spacing_div2[i] = 0.5 / moving->spacing[i];

    /* Determine size of device memory */
    vol_size = moving->dim[0] * moving->dim[1] * moving->dim[2] * sizeof(float);
    interleaved_vol_size = 3 * fixed->dim[0] * fixed->dim[1] * fixed->dim[2] * sizeof(float);
    inlier_size = moving->dim[0] * moving->dim[1] * moving->dim[2] * sizeof(int);

    /* Allocate device memory */
    gpu_timer->start ();
    cudaMalloc((void**)&d_vf_est, interleaved_vol_size);
    cudaMalloc((void**)&d_vf_smooth, interleaved_vol_size);
    cudaMalloc((void**)&d_fixed, vol_size);
    cudaMalloc((void**)&d_moving, vol_size);
    cudaMalloc((void**)&d_m_grad, interleaved_vol_size);
    cudaMalloc((void**)&d_m_grad_mag, vol_size);
    cudaMalloc((void**)&d_ssd, vol_size);
    cudaMalloc((void**)&d_inliers, inlier_size);

    /* Copy/Initialize device memory */
    cudaMemcpy(d_vf_est, demons_state->vf_est->img, 
	interleaved_vol_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_vf_smooth, demons_state->vf_est->img, 
	interleaved_vol_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_fixed, fixed->img, vol_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_moving, moving->img, vol_size, cudaMemcpyHostToDevice);
    cudaMemset(d_m_grad, 0, interleaved_vol_size);
    cudaMemset(d_m_grad_mag, 0, vol_size);
    gpu_time += gpu_timer->report ();

    /* Set device constant memory */
    setConstantDimension(fixed->dim);
    setConstantMovingDimension(moving->dim);
    setConstantPixelSpacing(spacing_div2);

    /* Bind device texture memory */
    cudaBindTexture(0, tex_fixed, d_fixed, vol_size);
    cudaBindTexture(0, tex_moving, d_moving, vol_size);
    gpu_time += gpu_timer->report ();

    /* Check for any errors prekernel execution */
    CUDA_check_error("Error before kernel execution");

    /* Call kernel */
    kernel_timer->start ();
    volume_calc_grad_kernel<<< grid, block>>>(d_m_grad, blockY, 1.0f / (float)blockY);
    cudaThreadSynchronize();
    kernel_time += kernel_timer->report ();

    /* Check for any errors postkernel execution */
    CUDA_check_error("Kernel execution failed");

    /* Bind device texture memory */
    gpu_timer->start ();
    cudaBindTexture(0, tex_grad, d_m_grad, interleaved_vol_size);
    gpu_time += gpu_timer->report ();

    /* Check for any errors prekernel execution */
    CUDA_check_error("Error before kernel execution");

    /* Call kernel */
    kernel_timer->start ();
    calculate_gradient_magnitude_image_kernel<<< grid, block>>> (
	d_m_grad_mag, blockY, 1.0f / (float)blockY);
    cudaThreadSynchronize();
    kernel_time += kernel_timer->report ();

    /* Check for any errors postkernel execution */
    CUDA_check_error("Kernel execution failed");

    /* Validate filter widths */
    validate_filter_widths (fw, parms->filter_width);

    /* Create the seperable smoothing kernels for the x, y, and z directions */
    kerx = create_ker (parms->filter_std / fixed->spacing[0], fw[0]/2);
    kery = create_ker (parms->filter_std / fixed->spacing[1], fw[1]/2);
    kerz = create_ker (parms->filter_std / fixed->spacing[2], fw[2]/2);
    kernel_stats (kerx, kery, kerz, fw);

    /* Compute some variables for converting pixel sizes / origins */
    for (i = 0; i < 3; i++) {
	invmps[i] = 1 / moving->spacing[i];
	f2mo[i] = (fixed->origin[i] - moving->origin[i]) / moving->spacing[i];
	f2ms[i] = fixed->spacing[i] / moving->spacing[i];
    }

    /* Allocate device memory */
    gpu_timer->start ();
    printf ("Doing cudaMalloc\n");
    cudaMalloc ((void**)&d_kerx, fw[0] * sizeof(float));
    cudaMalloc ((void**)&d_kery, fw[1] * sizeof(float));
    cudaMalloc ((void**)&d_kerz, fw[2] * sizeof(float));

    /* Copy/Initialize device memory */
    printf ("Doing cudaMemcpy\n");
    cudaMemcpy (d_kerx, kerx, fw[0] * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy (d_kery, kery, fw[1] * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy (d_kerz, kerz, fw[2] * sizeof(float), cudaMemcpyHostToDevice);

    /* Set device constant memory */
    setConstantF2ms (f2mo);
    setConstantF2ms (f2ms);
    setConstantInvmps (invmps);

    /* Bind device texture memory */
    printf ("Doing cudaBindTexture\n");
    cudaBindTexture (0, tex_grad_mag, d_m_grad_mag, vol_size);
    gpu_time += gpu_timer->report ();

    timer->start ();

    /* Main loop through iterations */
    for (it = 0; it < parms->max_its; it++) {
	printf ("Looping...\n");
	/* Estimate displacement, store into vf_est */
	inliers = 0; ssd = 0.0;

	/* Check for any errors prekernel execution */
	CUDA_check_error ("Error before kernel execution");

	gpu_timer->start ();
	cudaBindTexture(0, tex_vf_smooth, d_vf_smooth, interleaved_vol_size);
	cudaMemset(d_ssd, 0, vol_size);
	cudaMemset(d_inliers, 0, inlier_size);
	gpu_time += gpu_timer->report ();

	/* Call kernel */
	kernel_timer->start ();
	estimate_kernel<<< grid, block >>> (
	    d_vf_est, 
	    d_ssd, 
	    d_inliers, 
	    parms->homog, 
	    parms->denominator_eps, 
	    parms->accel, 
	    blockY, 
	    1.0f / (float)blockY);
	cudaThreadSynchronize ();
	kernel_time += kernel_timer->report ();

	/* Check for any errors postkernel execution */
	CUDA_check_error ("Kernel execution failed");

	num_elements = moving->dim[0] * moving->dim[1] * moving->dim[2];
	while (num_elements > 1) {
	    half_num_elements = num_elements / 2;
	    reductionBlocks = (half_num_elements + BLOCK_SIZE - 1) / BLOCK_SIZE;

	    /* Invoke kernels */
	    dim3 reductionGrid(reductionBlocks, 1);
	    kernel_timer->start ();
	    reduction<float><<< reductionGrid, block >>>(d_ssd, num_elements);
	    cudaThreadSynchronize();
	    reduction<int><<< reductionGrid, block >>>(d_inliers, num_elements);
	    cudaThreadSynchronize();
	    kernel_time += kernel_timer->report ();

	    /* Check for any errors postkernel execution */
	    CUDA_check_error("Kernel execution failed");

	    num_elements = reductionBlocks;
	}

	/* Smooth the estimate into vf_smooth.  The volumes are ping-ponged. */
	gpu_timer->start ();
	cudaUnbindTexture(tex_vf_smooth);
	cudaBindTexture(0, tex_vf_est, d_vf_est, interleaved_vol_size);
	cudaMemcpy(&ssd, d_ssd, sizeof(float), cudaMemcpyDeviceToHost);
	cudaMemcpy(&inliers, d_inliers, sizeof(int), cudaMemcpyDeviceToHost);
	gpu_time += gpu_timer->report ();

	/* Print statistics */
	printf ("----- SSD = %.01f (%d/%d)\n", ssd/inliers, inliers, fixed->npix);

	/* Check for any errors prekernel execution */
	CUDA_check_error("Error before kernel execution");

	/* Call kernel */
	kernel_timer->start ();
	vf_convolve_x_kernel<<< grid, block >>>(d_vf_smooth, d_kerx, fw[0] / 2, blockY, 1.0f / (float)blockY);
	cudaThreadSynchronize();
	kernel_time += kernel_timer->report ();

	/* Check for any errors postkernel execution */
	CUDA_check_error("Kernel execution failed");

	gpu_timer->start ();
	cudaUnbindTexture(tex_vf_est);
	cudaBindTexture(0, tex_vf_smooth, d_vf_smooth, interleaved_vol_size);
	gpu_time += gpu_timer->report ();

	/* Call kernel */
	kernel_timer->start ();
	vf_convolve_y_kernel<<< grid, block >>>(d_vf_est, d_kery, fw[1] / 2, blockY, 1.0f / (float)blockY);
	cudaThreadSynchronize();
	kernel_time += kernel_timer->report ();

	/* Check for any errors postkernel execution */
	CUDA_check_error("Kernel execution failed");

	gpu_timer->start ();
	cudaUnbindTexture(tex_vf_smooth);
	cudaBindTexture(0, tex_vf_est, d_vf_est, interleaved_vol_size);
	gpu_time += gpu_timer->report ();

	/* Call kernel */
	kernel_timer->start ();
	vf_convolve_z_kernel<<< grid, block >>>(d_vf_smooth, d_kerz, fw[2] / 2, blockY, 1.0f / (float)blockY);
	cudaThreadSynchronize();
	kernel_time += kernel_timer->report ();

	/* Check for any errors postkernel execution */
	CUDA_check_error("Kernel execution failed");

	/* Ping pong between estimate and smooth in each iteration*/
	d_swap = d_vf_est;
	d_vf_est = d_vf_smooth;
	d_vf_smooth = d_swap;
    }

    /* Copy final output from device to host */
    gpu_timer->start ();
    cudaMemcpy (demons_state->vf_smooth->img, d_vf_est, 
	interleaved_vol_size, cudaMemcpyDeviceToHost);
    gpu_time += gpu_timer->report ();

    free(kerx);
    free(kery);
    free(kerz);
    //delete vf_est;

    diff_run = timer->report ();
    printf("Time for %d iterations = %f (%f sec / it)\n", parms->max_its, diff_run, diff_run / parms->max_its);

    /* Print statistics */
    total_runtime = gpu_time + kernel_time;
    printf("\nTransfer run time: %f ms\n", gpu_time * 1000);
    printf("Kernel run time: %f ms\n", kernel_time * 1000);
    printf("Total CUDA run time: %f s\n\n", total_runtime);

    delete timer;
    delete kernel_timer;
    delete gpu_timer;

    /* Unbind device texture memory */
    cudaUnbindTexture(tex_vf_est);
    cudaUnbindTexture(tex_grad_mag);
    cudaUnbindTexture(tex_grad);
    cudaUnbindTexture(tex_moving);
    cudaUnbindTexture(tex_fixed);

    /* Free device global memory */
    cudaFree(d_vf_est);
    cudaFree(d_vf_smooth);
    cudaFree(d_moving);
    cudaFree(d_fixed);
    cudaFree(d_m_grad);
    cudaFree(d_m_grad_mag);
    cudaFree(d_ssd);
    cudaFree(d_inliers);

    //return vf_smooth;
}
