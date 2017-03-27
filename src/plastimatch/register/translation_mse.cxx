/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "interpolate.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "registration_data.h"
#include "stage_parms.h"
#include "translation_mse.h"
#include "volume.h"
#include "volume_macros.h"
#include "volume_resample.h"
#include "xform.h"

float
translation_mse (
    const Stage_parms *stage,
    const Metric_state::Pointer& ssi,
    const float dxyz[3])
{
    plm_long fijk[3], fv;         /* Indices within fixed image (vox) */
    float fxyz[3];                /* Position within fixed image (mm) */
    float mijk[3];                /* Indices within moving image (vox) */
    float mxyz[3];                /* Position within moving image (mm) */

    float li_1[3];                /* Fraction of interpolant in lower index */
    float li_2[3];                /* Fraction of interpolant in upper index */
    plm_long mijk_f[3], mvf;      /* Floor */
    plm_long mijk_r[3];           /* Round */
    float m_val;

    Volume *fixed = ssi->fixed_ss.get();
    Volume *moving = ssi->moving_ss.get();
    float* f_img = (float*) fixed->img;
    float* m_img = (float*) moving->img;

    double score_acc = 0.0;
    plm_long num_vox = 0;

    LOOP_Z (fijk, fxyz, fixed) {
        LOOP_Y (fijk, fxyz, fixed) {
            LOOP_X (fijk, fxyz, fixed) {

                /* Compute moving image coordinate of fixed image voxel */
                mxyz[2] = fxyz[2] + dxyz[2] - moving->origin[2];
                mxyz[1] = fxyz[1] + dxyz[1] - moving->origin[1];
                mxyz[0] = fxyz[0] + dxyz[0] - moving->origin[0];
                mijk[2] = PROJECT_Z (mxyz, moving->proj);
                mijk[1] = PROJECT_Y (mxyz, moving->proj);
                mijk[0] = PROJECT_X (mxyz, moving->proj);

                if (!moving->is_inside (mijk)) continue;

                /* Compute interpolation fractions */
                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving);

                /* Find linear index of "corner voxel" in moving image */
                mvf = volume_index (moving->dim, mijk_f);

                /* Compute moving image intensity using linear interpolation */
                /* Macro is slightly faster than function */
                LI_VALUE (m_val, 
                    li_1[0], li_2[0],
                    li_1[1], li_2[1],
                    li_1[2], li_2[2],
                    mvf, m_img, moving);

                /* Compute linear index of fixed image voxel */
                fv = volume_index (fixed->dim, fijk);

                /* Compute intensity difference */
                float diff = m_val - f_img[fv];

                score_acc += diff * diff;
                num_vox++;
            }
        }
    }

    /* Normalize score */
    const int MIN_VOX = 1;
    float final_metric;
    if (num_vox < MIN_VOX) {
        final_metric = FLT_MAX;
    } else {
        final_metric = score_acc / num_vox;
    }

    return final_metric;
}
