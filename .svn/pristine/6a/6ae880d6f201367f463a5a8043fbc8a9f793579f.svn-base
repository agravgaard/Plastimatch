/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_optimize_nlopt_h_
#define _bspline_optimize_nlopt_h_

#include "plmregister_config.h"
#if (NLOPT_FOUND)
#include "nlopt.h"
#endif

class Bspline_optimize;

#if (NLOPT_FOUND)
PLMREGISTER_C_API void bspline_optimize_nlopt (
        Bspline_optimize *bod,
        nlopt_algorithm algorithm
);
#endif


#endif
