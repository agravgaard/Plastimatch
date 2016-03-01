/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"

#include "bspline_correspond.h"
#include "plm_int.h"
#include "volume.h"

static void
float_to_plm_long_clamp (plm_long* p, float* f, const plm_long* max)
{
    float p_f[3];
    p_f[0] = f[0];
    p_f[1] = f[1];
    p_f[2] = f[2];

    /* x */
    if (p_f[0] < 0.f) {
        p[0] = 0;
    }
    else if (p_f[0] >= max[0]) {
        p[0] = max[0] - 1;
    }
    else {
        p[0] = FLOOR_PLM_LONG (p_f[0]);
    }

    /* y */
    if (p_f[1] < 0.f) {
        p[1] = 0;
    }
    else if (p_f[1] >= max[1]) {
        p[1] = max[1] - 1;
    }
    else {
        p[1] = FLOOR_PLM_LONG (p_f[1]);
    }

    /* z */
    if (p_f[2] < 0.f) {
        p[2] = 0;
    }
    else if (p_f[2] >= max[2]) {
        p[2] = max[2] - 1;
    }
    else {
        p[2] = FLOOR_PLM_LONG (p_f[2]);
    }
}

int
inside_roi (float* xyz, const Volume* roi)
{
    float p_f[3];
    float tmp[3];
    plm_long p[3];

    tmp[0] = xyz[0] - roi->origin[0];
    tmp[1] = xyz[1] - roi->origin[1];
    tmp[2] = xyz[2] - roi->origin[2];

    p_f[0] = PROJECT_X (tmp, roi->proj);
    p_f[1] = PROJECT_Y (tmp, roi->proj);
    p_f[2] = PROJECT_Z (tmp, roi->proj);

    float_to_plm_long_clamp (p, p_f, roi->dim);

    unsigned char *m = (unsigned char*)roi->img;
    plm_long i = volume_index (roi->dim, p);

    /* 0 outside roi, 1 inside */
    return (int)m[i];
}

/* Find location and index of corresponding voxel in moving image.
 * This version takes direction cosines into consideration
   Return 1 if corresponding voxel lies within the moving image, 
   return 0 if outside the moving image.  */
int
bspline_find_correspondence_dcos
(
 float *mxyz,             /* Output: xyz coordinates in moving image (mm) */
 float *mijk,             /* Output: ijk indices in moving image (vox) */
 const float *fxyz,       /* Input:  xyz coordinates in fixed image (mm) */
 const float *dxyz,       /* Input:  displacement from fixed to moving (mm) */
 const Volume *moving     /* Input:  moving image */
 )
{
    float tmp[3];

    mxyz[0] = fxyz[0] + dxyz[0];
    mxyz[1] = fxyz[1] + dxyz[1];
    mxyz[2] = fxyz[2] + dxyz[2];

    tmp[0] = mxyz[0] - moving->origin[0];
    tmp[1] = mxyz[1] - moving->origin[1];
    tmp[2] = mxyz[2] - moving->origin[2];

    mijk[0] = PROJECT_X (tmp, moving->proj);
    mijk[1] = PROJECT_Y (tmp, moving->proj);
    mijk[2] = PROJECT_Z (tmp, moving->proj);

    if (!moving->is_inside (mijk)) return 0;

    return 1;
}

/* Find location and index of corresponding voxel in moving image.
 * This version takes direction cosines and true roiing into consideration
   Return 1 if corresponding voxel lies within the moving image, 
   return 0 if outside the moving image.  */
int
bspline_find_correspondence_dcos_roi
(
 float *mxyz,               /* Output: xyz coordinates in moving image (mm) */
 float *mijk,               /* Output: ijk indices in moving image (vox) */
 const float *fxyz,         /* Input:  xyz coordinates in fixed image (mm) */
 const float *dxyz,         /* Input:  displacement from fixed to moving (mm) */
 const Volume *moving,      /* Input:  moving image */
 const Volume *moving_roi   /* Input:  moving image roi */
 )
{
    float tmp[3];

    mxyz[0] = fxyz[0] + dxyz[0];
    mxyz[1] = fxyz[1] + dxyz[1];
    mxyz[2] = fxyz[2] + dxyz[2];

    tmp[0] = mxyz[0] - moving->origin[0];
    tmp[1] = mxyz[1] - moving->origin[1];
    tmp[2] = mxyz[2] - moving->origin[2];

    mijk[0] = PROJECT_X (tmp, moving->proj);
    mijk[1] = PROJECT_Y (tmp, moving->proj);
    mijk[2] = PROJECT_Z (tmp, moving->proj);

    if (!moving->is_inside (mijk)) return 0;

    if (moving_roi) {
        return inside_roi (mxyz, moving_roi);
    }

    return 1;
}
