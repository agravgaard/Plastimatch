/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _drr_cuda_h_
#define _drr_cuda_h_

#include "plmreconstruct_config.h"
#include "delayload.h"

class Drr_options;
class Proj_image;
class Volume;
class Volume_limit;

#if defined __cplusplus
extern "C" {
#endif

PLMRECONSTRUCTCUDA_API
DELAYLOAD_WRAP (
void* drr_cuda_state_create,
    Proj_image *proj,
    Volume *vol,
    Drr_options *options
);

PLMRECONSTRUCTCUDA_API
DELAYLOAD_WRAP (
void drr_cuda_state_destroy,
    void *void_state
);

void*
drr_cuda_state_create_cu (
    Proj_image *proj,
    Volume *vol,
    Drr_options *options
);
void
drr_cuda_state_destroy_cu (
    void *void_state
);

PLMRECONSTRUCTCUDA_API
DELAYLOAD_WRAP (
void drr_cuda_ray_trace_image,
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

#if defined __cplusplus
}
#endif

#endif
