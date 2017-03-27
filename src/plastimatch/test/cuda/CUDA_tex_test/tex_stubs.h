#ifndef _tex_stubs_h_
#define _tex_stubs_h_


#if defined __cplusplus
extern "C" {
#endif

void CUDA_texture_test(float* test_data, int elements);
void checkCUDAError(const char *msg);

#if defined __cplusplus
}
#endif


#endif
