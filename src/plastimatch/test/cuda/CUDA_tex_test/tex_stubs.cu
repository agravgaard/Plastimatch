#include <stdio.h>
#include "tex_stubs.h"
#include "tex_kernels.h"

extern "C" void CUDA_texture_test(float* test_data, int elements)
{
	float* dev_test_data;
	float* dev_return;
	size_t test_size = elements * sizeof(float);

	// Allocate some global memory on the GPU
	cudaMalloc((void**)&dev_test_data, test_size);
	checkCUDAError("cudaMalloc(): dev_test_data"); 

	cudaMalloc((void**)&dev_return, test_size);
	checkCUDAError("cudaMalloc(): dev_return"); 

	// Copy test data to GPU global memory
	cudaMemcpy(dev_test_data, test_data, test_size, cudaMemcpyHostToDevice);
	checkCUDAError("cudaMemcpy(): test_data -> dev_test_data"); 

	cudaMemset(dev_return, 0, test_size);
	checkCUDAError("cudaMemset(): dev_return"); 

	memset(test_data, 0, test_size);

	// Bind allocated global memory to texture reference
	cudaBindTexture(0, tex_test, dev_test_data, test_size);
	checkCUDAError("cudaBindTexture(): dev_test_data -> tex_test"); 

	// Define the execution configuration
	int threads_per_block = 128;
	int num_threads = elements;
	int num_blocks = (int)ceil(num_threads / (float)threads_per_block);

	dim3 dimGrid(num_blocks, 1, 1);
	dim3 dimBlock(threads_per_block, 1, 1);

	// Invoke the kernel
	kernel_texture<<<dimGrid, dimBlock>>>(dev_return, test_size);
	checkCUDAError("Kernel Panic!"); 

	// Copy results back
	cudaMemcpy(test_data, dev_return, test_size, cudaMemcpyDeviceToHost);
	checkCUDAError("cudaMemcpy(): dev_return -> test_data"); 

	// Cleanup
	cudaUnbindTexture(tex_test);
	cudaFree(dev_test_data);
	cudaFree(dev_return);
}

extern "C" void checkCUDAError(const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) 
    {
        fprintf(stderr, "\n\nCUDA ERROR: %s (%s).\n", msg, cudaGetErrorString( err) );
        exit(EXIT_FAILURE);
    }                         
}
