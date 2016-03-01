/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_cuda_math_h_
#define _plm_cuda_math_h_

#include "plm_config.h"
#include <cuda.h>
#include "plm_int.h"

/* Host to device operators */
#if defined (commentout)
inline __host__
int3
make_int3 (int *a)
{
    return make_int3 (a[0], a[1], a[2]);
}
#endif

inline __host__
int3
make_int3 (plm_long *a)
{
    return make_int3 (a[0], a[1], a[2]);
}

inline __host__
int3
make_int3 (size_t *a)
{
    return make_int3 ((int) a[0], (int) a[1], (int) a[2]);
}

inline __host__
int4
make_int4 (int *a)
{
    return make_int4 (a[0], a[1], a[2], a[3]);
}

inline __host__
float2
make_float2 (float *a)
{
    return make_float2 (a[0], a[1]);
}

inline __host__
float2
make_float2 (double *a)
{
    return make_float2 (a[0], a[1]);
}

inline __host__
float3
make_float3 (float *a)
{
    return make_float3 (a[0], a[1], a[2]);
}

inline __host__
float3
make_float3 (double *a)
{
    return make_float3 (a[0], a[1], a[2]);
}

inline __host__
float4
make_float4 (float *a)
{
    return make_float4 (a[0], a[1], a[2], a[3]);
}

inline __host__
float4
make_float4 (double *a)
{
    return make_float4 (a[0], a[1], a[2], a[3]);
}

/* Device type conversion */
inline __device__
int3
make_int3 (float3 a)
{
    return make_int3 (a.x, a.y, a.z);
}

/* Overloaded operators */
inline __host__ __device__ 
float3 
operator+ (float3 a, float3 b)
{
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline __host__ __device__ 
float3 
operator- (float3 a, float3 b)
{
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline __host__ __device__ 
float3 
operator* (float a, float3 b)
{
    return make_float3(a * b.x, a * b.y, a * b.z);
}

inline __host__ __device__ 
float3
operator* (float3 a, float3 b)
{
    return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline __host__ __device__ 
float3
operator/ (float a, float3 b)
{
    return make_float3(a / b.x, a / b.y, a / b.z);
}

inline __host__ __device__ 
int3
operator< (int3 a, int3 b)
{
    return make_int3 (a.x < b.x, a.y < b.y, a.z < b.z);
}

inline __host__ __device__ 
int3
operator< (float3 a, float3 b)
{
    return make_int3 (a.x < b.x, a.y < b.y, a.z < b.z);
}

inline __host__ __device__ 
int3
operator< (float3 a, float b)
{
    return make_int3 (a.x < b, a.y < b, a.z < b);
}

/* Misc functions */
inline __host__ __device__ 
float 
dot (float3 a, float3 b)
{ 
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline __host__ __device__ 
float3 
normalize (float3 v)
{
    float inv_len = 1.0f / sqrtf(dot(v, v));
    return inv_len * v;
}

inline __host__ __device__ 
float3
fabsf3 (float3 a)
{
    return make_float3 (fabsf(a.x), fabsf(a.y), fabsf(a.z));
}

inline __host__ __device__ 
float3
floorf3 (float3 a)
{
    return make_float3 (floorf(a.x), floorf(a.y), floorf(a.z));
}

inline __host__ __device__
void
swapf (float *a, float *b)
{
    float c = *a;
    *a = *b;
    *b = c;
}

inline __host__ __device__
void 
sortf (float *a, float *b)
{
    if (*a > *b) {
	swapf (a, b);
    }
}

inline __host__ __device__
void
sortf3 (float3 *a, float3 *b)
{
    sortf (&a->x, &b->x);
    sortf (&a->y, &b->y);
    sortf (&a->z, &b->z);
}

#endif
