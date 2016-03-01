/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rasterize_slice_h_
#define _rasterize_slice_h_

#include "plmbase_config.h"
#include "plm_int.h"


PLMBASE_C_API void rasterize_slice (
    unsigned char* acc_img,
    plm_long* dims,
    float* spacing,
    float* offset,
    size_t num_vertices,
    const float* x_in,           /* polygon vertices in mm */
    const float* y_in            /* polygon vertices in mm */
);

bool point_in_polygon (
    const float* x_in,           /* polygon vertices in mm */
    const float* y_in,           /* polygon vertices in mm */
    size_t num_vertices,
    float x_test,
    float y_test
);

#endif
