/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_correspond_h_
#define _bspline_correspond_h_

#include "plmutil_config.h"

class Volume;

PLMUTIL_API int inside_roi (float* xyz, const Volume* roi);

PLMUTIL_API int bspline_find_correspondence_dcos (
    float *mxyz,             /* Output: xyz coordinates in moving image (mm) */
    float *mijk,             /* Output: ijk indices in moving image (vox) */
    const float *fxyz,       /* Input:  xyz coordinates in fixed image (mm) */
    const float *dxyz,       /* Input:  displacement from fixed to moving (mm) */
    const Volume *moving     /* Input:  moving image */
);

PLMUTIL_API int bspline_find_correspondence_dcos_roi (
    float *mxyz,               /* Output: xyz coordinates in moving image (mm) */
    float *mijk,               /* Output: ijk indices in moving image (vox) */
    const float *fxyz,         /* Input:  xyz coordinates in fixed image (mm) */
    const float *dxyz,         /* Input:  displacement from fixed to moving (mm) */
    const Volume *moving,      /* Input:  moving image */
    const Volume *moving_roi  /* Input:  moving image roi */
);

#endif
