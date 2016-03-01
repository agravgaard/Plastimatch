/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "interpolate.h"
#include "plm_math.h"
#include "volume.h"

/* Clipping is done using clamping.  
   Note: the value of *maf can be at most dim[x]-2, because the linear 
   interpolation code assumes that the "lower" pixel is always valid. */
void
li_clamp (
    float ma,     /* Input:  (Unrounded) pixel coordinate (in vox) */
    plm_long dmax,  /* Input:  Maximum coordinate in this dimension */
    plm_long* maf,  /* Output: x, y, or z coord of "floor" pixel in moving img */
    plm_long* mar,  /* Output: x, y, or z coord of "round" pixel in moving img */
    float* fa1,	  /* Output: Fraction of interpolant for lower index voxel */
    float* fa2	  /* Output: Fraction of interpolant for upper index voxel */
)
{
    if (ma < 0.f) {
	*maf = 0;
	*mar = 0;
	*fa2 = 0.0f;
    } else if (ma >= dmax) {
	*maf = dmax - 1;
	*mar = dmax;
	*fa2 = 1.0f;
    } else {
	*maf = FLOOR_PLM_LONG (ma);
	*mar = ROUND_PLM_LONG (ma);
	*fa2 = ma - *maf;
    }
    *fa1 = 1.0f - *fa2;
}

void
li_clamp_3d (
    const float mijk[3],  /* Input:  Unrounded pixel coordinates in vox */
    plm_long mijk_f[3],   /* Output: "floor" pixel in moving img in vox*/
    plm_long mijk_r[3],   /* Ouptut: "round" pixel in moving img in vox*/
    float li_frac_1[3],   /* Output: Fraction for upper index voxel */
    float li_frac_2[3],   /* Output: Fraction for lower index voxel */
    Volume *moving        /* Input:  Volume (for dims) */
)
{
    li_clamp (mijk[0], moving->dim[0]-1, &mijk_f[0], 
	&mijk_r[0], &li_frac_1[0], &li_frac_2[0]);
    li_clamp (mijk[1], moving->dim[1]-1, &mijk_f[1], 
	&mijk_r[1], &li_frac_1[1], &li_frac_2[1]);
    li_clamp (mijk[2], moving->dim[2]-1, &mijk_f[2], 
	&mijk_r[2], &li_frac_1[2], &li_frac_2[2]);
}

float
li_value (
    float fx1, float fx2,  /* Input:  Fraction of upper, lower x voxel */
    float fy1, float fy2,  /* Input:  Fraction of upper, lower y voxel */
    float fz1, float fz2,  /* Input:  Fraction of upper, lower z voxel */
    plm_long mvf,            /* Input:  Index of lower-left voxel in 8-group */
    float *m_img,          /* Input:  Pointer to raw data */
    Volume *moving         /* Input:  Volume (for dimensions) */
)
{
    float m_x1y1z1, m_x2y1z1, m_x1y2z1, m_x2y2z1;
    float m_x1y1z2, m_x2y1z2, m_x1y2z2, m_x2y2z2;
    float m_val;

    m_x1y1z1 = fx1 * fy1 * fz1 * m_img[mvf];
    m_x2y1z1 = fx2 * fy1 * fz1 * m_img[mvf+1];
    m_x1y2z1 = fx1 * fy2 * fz1 * m_img[mvf+moving->dim[0]];
    m_x2y2z1 = fx2 * fy2 * fz1 * m_img[mvf+moving->dim[0]+1];
    m_x1y1z2 = fx1 * fy1 * fz2 * m_img[mvf+moving->dim[1]*moving->dim[0]];
    m_x2y1z2 = fx2 * fy1 * fz2 * m_img[mvf+moving->dim[1]*moving->dim[0]+1];
    m_x1y2z2 = fx1 * fy2 * fz2 * m_img[mvf+moving->dim[1]*moving->dim[0]+moving->dim[0]];
    m_x2y2z2 = fx2 * fy2 * fz2 * m_img[mvf+moving->dim[1]*moving->dim[0]+moving->dim[0]+1];
    m_val = m_x1y1z1 + m_x2y1z1 + m_x1y2z1 + m_x2y2z1 
	    + m_x1y1z2 + m_x2y1z2 + m_x1y2z2 + m_x2y2z2;

    return m_val;
}
