/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_demons_h_
#define _itk_demons_h_

#include "plmregister_config.h"
#include "itk_image_type.h"
#include "xform.h"

class Registration_data;
class Stage_parms;

Xform::Pointer
do_itk_demons_stage (Registration_data* regd, 
    const Xform::Pointer& xf_in, const Stage_parms* stage);

#endif
