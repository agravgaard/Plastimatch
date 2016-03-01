/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "demons_state.h"
#include "volume.h"

Demons_state::Demons_state (void)
{
}

Demons_state::~Demons_state (void)
{
}

void
Demons_state::init (
	Volume* fixed, 
	Volume* moving, 
	Volume* moving_grad, 
	Volume* vf_init, 
	Demons_parms* parms)
{
    /* Allocate memory for vector fields */
    if (vf_init) {
	/* If caller has an initial estimate, we copy it */
	this->vf_smooth = volume_clone (vf_init);
	vf_convert_to_interleaved (this->vf_smooth);
    } else {
	/* Otherwise initialize to zero */
	vf_smooth = new Volume (fixed->dim, fixed->origin, fixed->spacing, 
	    fixed->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3);
    }
    vf_est = new Volume (fixed->dim, fixed->origin, fixed->spacing, 
	fixed->direction_cosines, PT_VF_FLOAT_INTERLEAVED, 3);
}
