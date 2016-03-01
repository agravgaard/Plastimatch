#include <stdio.h>
#include <assert.h>

// Simple utility function to check for CUDA runtime errors
void checkCUDAError (const char *msg);

// Part 3 of 5: implement the kernel
__global__ void 
myFirstKernel(int *d_a)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    d_a[idx] = idx;  
}

__global__ void 
reduce(float *idata, float *odata) 
{
    extern __shared__ float sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
    sdata[tid] = idata[i];

    __syncthreads();

    for(unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
	if(tid < s) {
	    sdata[tid] += sdata[tid + s];
	}
	__syncthreads();
    }

    if(tid == 0) {
	odata[blockIdx.x] = sdata[0];
    }
}

int 
cuda_test_1 (int argc, char** argv)
{
    // pointer for host memory
    int *h_a;

    // pointer for device memory
    int *d_a;

    // define grid and block size
    int numBlocks = 8;
    int numThreadsPerBlock = 8;

    // Part 1 of 5: allocate host and device memory
    size_t memSize = numBlocks * numThreadsPerBlock * sizeof(int);
    h_a = (int *) malloc(memSize);
    cudaMalloc((void **) &d_a, memSize);

    checkCUDAError("cudaMalloc");

    // Part 2 of 5: launch kernel
    dim3 dimGrid(numBlocks, 1, 1);
    dim3 dimBlock(numThreadsPerBlock, 1, 1);
    myFirstKernel<<<dimGrid, dimBlock>>>(d_a);

    // block until the device has completed
    cudaThreadSynchronize();

    // check if kernel execution generated an error
    checkCUDAError("kernel execution");

    // Part 4 of 5: device to host copy
    cudaMemcpy( h_a, d_a, memSize, cudaMemcpyDeviceToHost );

    // Check for any CUDA errors
    checkCUDAError("cudaMemcpy");

    // Part 5 of 5: verify the data returned to the host is correct
    for (int i = 0; i < numBlocks; i++)	{
	for (int j = 0; j < numThreadsPerBlock; j++) {
	    assert (h_a[i * numThreadsPerBlock + j] == i * numThreadsPerBlock + j);
	}
    }

    // free device memory
    cudaFree(d_a);

    // free host memory
    free(h_a);

    // If the program makes it this far, then the results are correct and
    // there are no run-time errors.  Good work!
    printf("Correct!\n");

    return 0;
}

void 
checkCUDAError (const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if (cudaSuccess != err)  {
        fprintf (stderr, "Cuda error: %s: %s.\n", 
		 msg, cudaGetErrorString (err));
        exit(-1);
    }                         
}

int
cuda_mem_test (int argc, char** argv)
{
    void *test[4];
    int alloc_mb;
    const int MB = (1024*1024);

    //for (alloc_mb = 1024; alloc_mb <= 1024*1024*1024; alloc_mb *= 2) {
    for (alloc_mb = 100; alloc_mb <= 1100; alloc_mb += 100) {
	printf ("Alloc = 4 x %d MB = %d MB\n", alloc_mb, 4*alloc_mb);
	cudaMalloc ((void**) &test[0], alloc_mb * MB);
	checkCUDAError ("cudaMalloc[0]");
	cudaMalloc ((void**) &test[1], alloc_mb * MB);
	checkCUDAError ("cudaMalloc[1]");
	cudaMalloc ((void**) &test[2], alloc_mb * MB);
	checkCUDAError ("cudaMalloc[2]");
	cudaMalloc ((void**) &test[3], alloc_mb * MB);
	checkCUDAError ("cudaMalloc[3]");
	cudaFree (test[0]);
	checkCUDAError ("cudaFree");
	cudaFree (test[1]);
	checkCUDAError ("cudaFree");
	cudaFree (test[2]);
	checkCUDAError ("cudaFree");
	cudaFree (test[3]);
	checkCUDAError ("cudaFree");
    }
    return 0;
}

int 
main (int argc, char** argv)
{
    //cuda_test_1 (argc, argv);
    return cuda_mem_test (argc, argv);
}
