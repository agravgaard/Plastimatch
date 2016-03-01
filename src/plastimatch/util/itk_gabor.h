/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_gabor_h_
#define _itk_gabor_h_

#include "plmutil_config.h"
#include "itk_image_type.h"

class Plm_image_header;

PLMUTIL_API void itk_gabor (FloatImageType::Pointer image);
PLMUTIL_API FloatImageType::Pointer itk_gabor_create (const Plm_image_header *pih);

#endif
