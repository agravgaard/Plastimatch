#include "tex_kernels.h"

__global__ void kernel_texture(float* dev_return, int test_size)
{
	// -- Setup Thread Attributes -----------------------------
	int blockIdxInGrid  = (gridDim.x * blockIdx.y) + blockIdx.x;
	int threadsPerBlock  = (blockDim.x * blockDim.y * blockDim.z);
	int threadIdxInBlock = (blockDim.x * blockDim.y * threadIdx.z) + (blockDim.x * threadIdx.y) + threadIdx.x;
	int threadIdxInGrid = (blockIdxInGrid * threadsPerBlock) + threadIdxInBlock;
	// --------------------------------------------------------

	// Return excess threads
	if ( threadIdxInGrid > (test_size/sizeof(float)) )
		return;


	// Read element from texture, increment it, and then
	// place it into the return array.
	dev_return[threadIdxInGrid] = tex1Dfetch(tex_test, threadIdxInGrid) + 1.0;
	
}


__global__ void kernel_no_texture(float* dev_test_data, float* dev_return, int test_size)
{
	// -- Setup Thread Attributes -----------------------------
	int blockIdxInGrid  = (gridDim.x * blockIdx.y) + blockIdx.x;
	int threadsPerBlock  = (blockDim.x * blockDim.y * blockDim.z);
	int threadIdxInBlock = (blockDim.x * blockDim.y * threadIdx.z) + (blockDim.x * threadIdx.y) + threadIdx.x;
	int threadIdxInGrid = (blockIdxInGrid * threadsPerBlock) + threadIdxInBlock;
	// --------------------------------------------------------

	// Return excess threads
	if ( threadIdxInGrid > (test_size/sizeof(float)) )
		return;


	// Read element from texture, increment it, and then
	// place it into the return array.
	dev_return[threadIdxInGrid] = dev_test_data[threadIdxInGrid] + 1.0;
	
}
