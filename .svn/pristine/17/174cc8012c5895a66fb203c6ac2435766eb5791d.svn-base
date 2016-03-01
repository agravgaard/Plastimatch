/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _ray_trace_probe_h_
#define _ray_trace_probe_h_

#include "plmbase_config.h"
#include "ray_trace_callback.h"

class Volume;
class Volume_limit;

PLMBASE_C_API void ray_trace_probe (
    Volume *vol,                  /* Input: volume */
    Volume_limit *vol_limit,      /* Input: min/max coordinates of volume */
    Ray_trace_callback callback,  /* Input: callback function */
    void *callback_data,          /* Input: callback function private data */
    double *p1in,                 /* Input: start point for ray */
    double *p2in,                 /* Input: end point for ray */
    float ray_depth,              /* Input: depth along ray to probe */
    float ray_idx                 /* Input: z-idnex along ray cast */
);

#endif
