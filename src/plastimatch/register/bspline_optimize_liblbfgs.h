/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_optimize_liblbfgs_h_
#define _bspline_optimize_liblbfgs_h_

#include "plmregister_config.h"
#include "lbfgs.h"

class Bspline_optimize;

PLMREGISTER_C_API void bspline_optimize_liblbfgs (Bspline_optimize *bod);

#endif
