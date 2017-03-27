#ifndef _tex_kernels_h_
#define _tex_kernels_h_

__global__ void kernel_texture(float* dev_return, int test_size);
__global__ void kernel_no_texture(float* dev_test_data, float* dev_return, int test_size);


texture<float, 1, cudaReadModeElementType> tex_test;

#endif
