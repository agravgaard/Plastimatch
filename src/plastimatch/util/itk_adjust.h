/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_adjust_h_
#define _itk_adjust_h_

#include "plmutil_config.h"
#include <list>
#include <utility>
#include "itk_image_type.h"
#include "float_pair_list.h"

PLMUTIL_API FloatImageType::Pointer itk_adjust (FloatImageType::Pointer image, const Float_pair_list& al);
PLMUTIL_API FloatImageType::Pointer itk_adjust (FloatImageType::Pointer image, const std::string& adj_string);
PLMUTIL_API FloatImageType::Pointer itk_auto_adjust (FloatImageType::Pointer image);

#endif
