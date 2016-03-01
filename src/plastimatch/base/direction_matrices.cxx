/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "direction_cosines.h"
#include "direction_matrices.h"

void
compute_direction_matrices (float *step, float *proj, 
    const Direction_cosines& dc, const float *spacing)
{
    const float* inv_dc = dc.get_inverse ();
    for (int i = 0; i < 3; i++) {
	for (int j = 0; j < 3; j++) {
	    step[3*i+j] = dc[3*i+j] * spacing[j];
	    proj[3*i+j] = inv_dc[3*i+j] / spacing[i];
	}
    }
}
