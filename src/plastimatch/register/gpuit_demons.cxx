/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "demons.h"
#include "gpuit_demons.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "registration_data.h"
#include "stage_parms.h"
#include "volume.h"
#include "volume_grad.h"
#include "volume_resample.h"
#include "xform.h"

Xform::Pointer
do_gpuit_demons_stage_internal (
    Registration_data* regd, 
    const Xform::Pointer& xf_in, 
    const Stage_parms* stage)
{
    Xform::Pointer xf_out = Xform::New ();
    int d;
    Demons_parms parms;
    Plm_image_header pih;

    Plm_image::Pointer fixed_image = regd->get_fixed_image();
    Plm_image::Pointer moving_image = regd->get_moving_image();
    Volume::Pointer& fixed = fixed_image->get_volume_float ();
    Volume::Pointer& moving = moving_image->get_volume_float ();
    Volume::Pointer moving_ss;
    Volume::Pointer fixed_ss;
    Volume::Pointer moving_grad;
    Volume* vf_out = 0;
    Volume* vf_in = 0;

    fixed->convert (PT_FLOAT);              /* Maybe not necessary? */
    moving->convert (PT_FLOAT);             /* Maybe not necessary? */

    lprintf ("SUBSAMPLE: (%g %g %g), (%g %g %g)\n", 
	stage->resample_rate_fixed[0], stage->resample_rate_fixed[1], 
	stage->resample_rate_fixed[2], stage->resample_rate_moving[0], 
	stage->resample_rate_moving[1], stage->resample_rate_moving[2]
    );
    moving_ss = volume_subsample_vox_legacy (
        moving, stage->resample_rate_moving);
    fixed_ss = volume_subsample_vox_legacy (
        fixed, stage->resample_rate_fixed);

    moving_grad = Volume::New (volume_make_gradient (moving_ss.get()));

    demons_default_parms (&parms);
    parms.max_its = stage->max_its;
    parms.filter_std = stage->demons_std;
    parms.accel = stage->demons_acceleration;
    parms.homog = stage->demons_homogenization;
    parms.threading = stage->threading_type;
    for (d = 0; d < 3; d++) {
	parms.filter_width[d] = stage->demons_filter_width[d];
    }

    /* Transform input xform to gpuit vector field */
    if (xf_in->m_type == XFORM_NONE) {
	vf_in = 0;
    } else {
        pih.set_from_gpuit (fixed_ss->dim, 
            fixed_ss->origin, fixed_ss->spacing, 
            fixed_ss->direction_cosines);
	xf_out = xform_to_gpuit_vf (xf_in, &pih);
	vf_in = xf_out->get_gpuit_vf().get();
    }

    /* Run demons */
    vf_out = demons (fixed_ss.get(), moving_ss.get(), moving_grad.get(), 
        vf_in, &parms);

    /* Do something with output vector field */
    xf_out->set_gpuit_vf (Volume::Pointer(vf_out));
    return xf_out;
}

Xform::Pointer
do_gpuit_demons_stage (
    Registration_data* regd, 
    const Xform::Pointer& xf_in,
    const Stage_parms* stage)
{
    return do_gpuit_demons_stage_internal (regd, xf_in, stage);
}
