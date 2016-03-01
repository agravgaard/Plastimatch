/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_warp_h_
#define _itk_warp_h_

#include "plmbase_config.h"
#include "itk_image_type.h"

template<class T, class U> T PLMBASE_API itk_warp_image (
        T im_in,
        DeformationFieldType::Pointer vf, 
        int linear_interp,
        U default_val
);

PLMBASE_API UCharVecImageType::Pointer itk_warp_image (
        UCharVecImageType::Pointer im_in, 
        DeformationFieldType::Pointer vf, 
        int linear_interp,
        unsigned char default_val
);

#endif
