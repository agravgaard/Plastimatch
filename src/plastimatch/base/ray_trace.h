/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _ray_trace_h_
#define _ray_trace_h_

#include "plmbase_config.h"
#include "ray_trace_callback.h"

class Volume;
class Volume_limit;

//#define DRR_VERBOSE
#define DRR_PLANE_RAY_TOLERANCE 1e-8
#define DRR_STRIDE_TOLERANCE 1e-10
#define DRR_HUGE_DOUBLE 1e10
#define DRR_LEN_TOLERANCE 1e-6
#define DRR_TOPLANE_TOLERANCE 1e-7
#define DRR_BOUNDARY_TOLERANCE 1e-6

#define DRR_MSD_NUM_BINS 60

#define DRR_PREPROCESS_ATTENUATION 1

PLMBASE_C_API void ray_trace_exact (
    Volume *vol,                  /* Input: volume */
    Volume_limit *vol_limit,      /* Input: min/max coordinates of volume */
    Ray_trace_callback callback,  /* Input: callback function */
    void *callback_data,          /* Input: callback function private data */
    double *p1in,                 /* Input: start point for ray */
    double *p2in                  /* Input: end point for ray */
);
PLMBASE_C_API void ray_trace_uniform (
    Volume *vol,                  /* Input: volume */
    Volume_limit *vol_limit,      /* Input: min/max coordinates of volume */
    Ray_trace_callback callback,  /* Input: callback function */
    void *callback_data,          /* Input: callback function private data */
    double *p1in,                 /* Input: start point for ray */
    double *p2in,                 /* Input: end point for ray */
    float ray_step                /* Input: uniform step size */
);

#endif
