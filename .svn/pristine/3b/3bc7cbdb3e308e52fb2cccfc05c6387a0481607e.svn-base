/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _fdk_cuda_p_h_
#define _fdk_cuda_p_h_

#include <cuda.h>
#include "fdk_cuda.h"

typedef struct kernel_args_fdk Fdk_cuda_kernel_args;
struct kernel_args_fdk
{
    int2 img_dim;
    float2 ic;
    float3 nrm;
    float sad;
    float sid;
    float scale;
    float3 vol_origin;
    int3 vol_dim;
    float3 vol_spacing;
    float matrix[12];
    char padding[4]; //for data alignment
    //padding to 128Bytes
};

typedef struct fdk_cuda_state Fdk_cuda_state;
struct fdk_cuda_state
{
    Fdk_cuda_kernel_args kargs;         // Host kernel args
    Fdk_cuda_kernel_args *dev_kargs;    // Device kernel args
    float *dev_vol;                     // Device volume voxels
    float *dev_img;                     // Device image pixels
    float *dev_matrix;                  // Device projection matrix
    dim3 dimGrid;                       // CUDA grid size
    dim3 dimBlock;                      // CUDA block size
    int blocksInY;                      // CUDA grid size
};

 
#endif
