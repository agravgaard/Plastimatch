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
#include "translation_mi.h"
#include "volume.h"
#include "volume_macros.h"
#include "volume_resample.h"
#include "xform.h"

float
translation_mi (
    const Stage_parms *stage,
    const Volume::Pointer& fixed,
    const Volume::Pointer& moving,
    const float dxyz[3])
{
    Bspline_mi_hist_set *mi_hist = new Bspline_mi_hist_set (
        stage->mi_hist_type,
        stage->mi_hist_fixed_bins,
        stage->mi_hist_moving_bins);
    mi_hist->initialize (fixed.get(), moving.get());
    mi_hist->reset_histograms ();
        
    float mse_score = 0.0f;
    double* f_hist = mi_hist->f_hist;
    double* m_hist = mi_hist->m_hist;
    double* j_hist = mi_hist->j_hist;
    double mhis = 0.0f;      /* Moving histogram incomplete sum */
    double jhis = 0.0f;      /* Joint  histogram incomplete sum */

    plm_long fijk[3], fidx;       /* Indices within fixed image (vox) */
    float fxyz[3];                /* Position within fixed image (mm) */
    float mijk[3];                /* Indices within moving image (vox) */
    float mxyz[3];                /* Position within moving image (mm) */
    plm_long mijk_f[3], midx_f;   /* Floor */
    plm_long mijk_r[3];           /* Round */
    float li_1[3];                /* Fraction of interpolant in lower index */
    float li_2[3];                /* Fraction of interpolant in upper index */

    plm_long num_vox = 0;
    
    /* PASS 1 - Accumulate histogram */
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

                /* Get tri-linear interpolation fractions */
                li_clamp_3d (mijk, mijk_f, mijk_r, li_1, li_2, moving.get());
                    
                /* Find the fixed image linear index */
                fidx = volume_index (fixed->dim, fijk);

                /* Find linear index the corner voxel used to identifiy the
                 * neighborhood of the moving image voxels corresponding
                 * to the current fixed image voxel */
                midx_f = volume_index (moving->dim, mijk_f);

                /* Add to histogram */
                mi_hist->add_pvi_8 (
                    fixed.get(), moving.get(), fidx, midx_f, li_1, li_2);

                num_vox++;
            }
        }
    }

    /* Compute score */
    return mi_hist->compute_score (num_vox);
}
