/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_distance_map_h_
#define _itk_distance_map_h_

#include "plmutil_config.h"
#include "itk_image_type.h"

PLMUTIL_API
FloatImageType::Pointer
itk_distance_map_danielsson (
    const UCharImageType::Pointer& ref_image,
    bool use_squared_distance,
    bool inside_positive
);

PLMUTIL_API
FloatImageType::Pointer
itk_distance_map_maurer (
    const UCharImageType::Pointer& ref_image,
    bool use_squared_distance,
    bool inside_positive
);

#endif
