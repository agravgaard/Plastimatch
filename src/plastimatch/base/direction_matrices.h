/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _direction_matrices_h_
#define _direction_matrices_h_

#include "plmbase_config.h"

class Direction_cosines;

PLMBASE_API void
compute_direction_matrices (float *step, float *proj, 
    const Direction_cosines& dc, const float *spacing);

#endif
