/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _DRR_OPENCL_H_
#define _DRR_OPENCL_H_

#include "plmreconstruct_config.h"
#include "delayload.h"

class Drr_options;
class Proj_image;
class Volume;
class Volume_limit;

PLMRECONSTRUCT_C_API void* drr_opencl_state_create (
    Proj_image *proj,
    Volume *vol,
    Drr_options *options
);
PLMRECONSTRUCT_C_API void drr_opencl_state_destroy (void *dev_state);
PLMRECONSTRUCT_C_API void drr_opencl_ray_trace_image (
    Proj_image *proj, 
    Volume *vol, 
    Volume_limit *vol_limit, 
    double p1[3], 
    double ul_room[3], 
    double incr_r[3], 
    double incr_c[3], 
    void *dev_state, 
    Drr_options *options
);

#endif
