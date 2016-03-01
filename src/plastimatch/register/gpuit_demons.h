/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _gpuit_demons_h_
#define _gpuit_demons_h_

#include "plmregister_config.h"
#include "xform.h"

class Registration_data;
class Stage_parms;

Xform::Pointer
do_gpuit_demons_stage (
    Registration_data* regd,
    const Xform::Pointer& xf_in,
    const Stage_parms* stage);

#endif
