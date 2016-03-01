/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _demons_h_
#define _demons_h_

#include "plmregister_config.h"
#include "threading.h"

class Volume;

class Demons_parms {
public:
    Threading threading;
    float denominator_eps;
    float homog;
    float accel;
    int filter_width[3];
    int max_its;
    float filter_std;
};

PLMREGISTER_API void demons_default_parms (Demons_parms* parms);
PLMREGISTER_API Volume* demons (
        Volume* fixed,
        Volume* moving,
        Volume* moving_grad,
        Volume* vf_init,
        Demons_parms* parms
);
Volume* demons_c (
        Volume* fixed,
        Volume* moving,
        Volume* moving_grad,
        Volume* vf_init,
        Demons_parms* parms
);

//plmopencl_EXPORT (
Volume* demons_opencl (
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad,
    Volume* vf_init,
    Demons_parms* parms
);

#endif
