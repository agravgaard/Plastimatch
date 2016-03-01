/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_mse_h_
#define _bspline_mse_h_

#include "plmregister_config.h"

class Bspline_optimize;

PLMREGISTER_API void bspline_score_c_mse (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_g_mse (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_h_mse (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_i_mse (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_k_mse (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_l_mse (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_m_mse (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_n_mse (Bspline_optimize *bod);

PLMREGISTER_API void
bspline_score_normalize (
    Bspline_optimize *bod,
    double raw_score
);

PLMREGISTER_API void bspline_score_mse (Bspline_optimize *bod);

#endif
