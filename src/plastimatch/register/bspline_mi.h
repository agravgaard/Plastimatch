/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_mi_h_
#define _bspline_mi_h_

#include "plmregister_config.h"

class Bspline_optimize;

PLMREGISTER_API void bspline_score_c_mi (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_d_mi (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_e_mi (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_f_mi (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_g_mi (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_h_mi (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_i_mi (Bspline_optimize *bod);
PLMREGISTER_API void bspline_score_k_mi (Bspline_optimize *bod);

PLMREGISTER_API void bspline_score_mi (Bspline_optimize *bod);

#endif
