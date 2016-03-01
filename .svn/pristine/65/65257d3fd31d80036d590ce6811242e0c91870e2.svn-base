/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _drr_opencl_p_h_
#define _drr_opencl_p_h_

#include "plmreconstruct_config.h"
#include "opencl_util.h"

typedef struct drr_opencl_state Drr_opencl_state;
struct drr_opencl_state
{
    Opencl_device ocl_dev;
    Opencl_buf *ocl_buf_img;
    Opencl_buf *ocl_buf_vol;
};

struct volume_limit_f {
    /* upper and lower limits of volume, including tolerances */
    float lower_limit[3];
    float upper_limit[3];

    /* dir == 0 if lower_limit corresponds to lower index */
    int dir[3];
};
typedef struct volume_limit_f Volume_limit_f;

#endif
