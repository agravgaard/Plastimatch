/* -----------------------------------------------------------------------
See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
----------------------------------------------------------------------- */
#ifndef _DEMONS_OPENCL_P_H_
#define _DEMONS_OPENCL_P_H_

/*
Constants
*/
#define BLOCK_SIZE 256
#define MAX_GPU_COUNT 8

/*
Data Structures
*/
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

#endif