/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_threshold_h_
#define _itk_threshold_h_

#include "plmutil_config.h"
#include "float_pair_list.h"
#include "itk_image_type.h"

PLMUTIL_API 
UCharImageType::Pointer itk_threshold_above (
    const FloatImageType::Pointer& image, 
    float threshold);
PLMUTIL_API 
UCharImageType::Pointer
itk_threshold (
    const FloatImageType::Pointer& image_in, 
    const Float_pair_list& fpl);
PLMUTIL_API 
UCharImageType::Pointer
itk_threshold (
    const FloatImageType::Pointer& image_in, 
    const std::string& fpl_string);

#endif
