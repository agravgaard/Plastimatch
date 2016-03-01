/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rbf_wendland_h_
#define _rbf_wendland_h_

#include "plmregister_config.h"

class Bspline_parms;
class Landmark_warp;
class Volume;

PLMREGISTER_API void bspline_rbf_wendland_find_coeffs (
        Volume *vector_field,
        Bspline_parms *parms
);
PLMREGISTER_API void bspline_rbf_wendland_update_vector_field (
        Volume *vector_field,
        Bspline_parms *parms
);
PLMREGISTER_API void rbf_wendland_warp (Landmark_warp *lw);

#endif
