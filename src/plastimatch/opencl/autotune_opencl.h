/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _AUTOTUNE_OPENCL_H_
#define _AUTOTUNE_OPENCL_H_

#include "plm_config.h"
#include "opencl_util.h"

#define MAX_GPU_COUNT 8

#if defined __cplusplus
extern "C" {
#endif

plmopencl_EXPORT
(void divideWork, 
    cl_device_id *devices, 
    cl_uint device_count, 
    int dimensions, 
    size_t work_per_device[MAX_GPU_COUNT][3], 
    size_t *work_total
);

#if defined __cplusplus
}
#endif

#endif
