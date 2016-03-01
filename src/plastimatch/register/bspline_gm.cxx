/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#if (OPENMP_FOUND)
#include <omp.h>
#endif
#if (SSE2_FOUND)
#include <xmmintrin.h>
#endif

#include "bspline.h"
#include "bspline_correspond.h"
#include "bspline_interpolate.h"
#include "bspline_loop.txx"
#include "bspline_macros.h"
#include "bspline_gm.h"
#include "bspline_gm.txx"
#include "bspline_optimize.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "file_util.h"
#include "interpolate.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "mha_io.h"
#include "plm_math.h"
#include "plm_timer.h"
#include "string_util.h"
#include "volume.h"
#include "volume_macros.h"

/* -----------------------------------------------------------------------
   FUNCTION: bspline_score_k_mse(), bspline_score_l_mse()

   This is the same as 'c', except using templates.

   This is the older "fast" single-threaded MSE implementation, modified 
   to respect direction cosines (and ROI support removed).
   ----------------------------------------------------------------------- */
void
bspline_score_k_gm (
    Bspline_optimize *bod
)
{
    /* Create/initialize bspline_loop_user */
    Bspline_gm_k blu (bod);

    /* Run the loop */
    bspline_loop_voxel_serial (blu, bod);

    /* Normalize score for MSE */
    bspline_score_normalize (bod, blu.score_acc);
}

void
bspline_score_gm (
    Bspline_optimize *bod
)
{
    return bspline_score_k_gm (bod);
}
