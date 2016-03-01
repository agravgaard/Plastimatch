/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _drr_cuda_p_h_
#define _drr_cuda_p_h_

#include "drr_cuda.h"

typedef struct drr_kernel_args Drr_kernel_args;
struct drr_kernel_args
{
    int2 img_dim;
    float2 ic;
    float3 nrm;
    float sad;
    float sid;
    float scale;
    float3 p1;
    float3 ul_room;
    float3 incr_r;
    float3 incr_c;
    int4 image_window;
    float3 lower_limit;
    float3 upper_limit;
    float3 vol_origin;
    int3 vol_dim;
    float3 vol_spacing;
    float matrix[12];
    //char padding[4]; //for data alignment << ?
    //padding to 128Bytes
};

typedef struct drr_cuda_state Drr_cuda_state;
struct drr_cuda_state
{
    Drr_kernel_args *kargs;         // Holds kernel parameters on host
    Drr_kernel_args *dev_kargs;     // Holds kernel parameters on device
    float *dev_vol;	            // Holds volume on device
    float *dev_img;	            // Holds image pixels on device
    float *dev_matrix;              // Holds projection matrix on device
};

#endif
