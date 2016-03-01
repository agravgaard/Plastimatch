/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_conv_h_
#define _itk_image_conv_h_

#include "plmbase_config.h"
#include "itk_image.h"

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
template<class T> PLMBASE_API T itk_image_conv (
    T img,
    T ker
);

#endif
