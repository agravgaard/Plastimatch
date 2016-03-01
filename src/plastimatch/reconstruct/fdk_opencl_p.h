#ifndef _FDK_OPENCL_P_H_
#define _FDK_OPENCL_P_H_

#include "fdk_opencl.h"

#define MAX_GPU_COUNT 8

struct int2 {
	int x;
	int y;
};

struct int4 {
	int x;
	int y;
	int z;
	int w;
};

struct float2 {
	float x;
	float y;
};

struct float4 {
	float x;
	float y;
	float z;
	float w;
};

struct kernel_args_fdk {
    int2 img_dim;
    float2 ic;
    float4 nrm;
    float sad;
    float sid;
    float scale;
    float4 vol_offset;
    int4 vol_dim;
    float4 vol_spacing;
    float matrix[12];
};

#endif

