/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_interpolate_h_
#define _bspline_interpolate_h_

#include "plmbase_config.h"
#include "plm_int.h"

class Bspline_xform;
class Volume;

PLMBASE_API void bspline_interpolate_vf (
    Volume* interp, const Bspline_xform* bxf);
PLMBASE_API void bspline_transform_point (
    float point_out[3], /* Output coordinate of point */
    Bspline_xform* bxf, /* Bspline transform coefficients */
    float point_in[3],  /* Input coordinate of point */
    int linear_interp   /* 1 = trilinear, 0 = nearest neighbors */
);
PLMBASE_API void bspline_interp_pix (
    float out[3],
    const Bspline_xform* bxf, 
    plm_long p[3],
    plm_long qidx
);
PLMBASE_API void bspline_interp_pix_b (
    float out[3], 
    Bspline_xform* bxf, 
    plm_long pidx, 
    plm_long qidx
);
PLMBASE_API void bspline_interp_pix_c (
    float out[3], 
    Bspline_xform* bxf, 
    plm_long pidx, 
    plm_long *q
);

#endif
