/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _ss_img_extract_h_
#define _ss_img_extract_h_

#include "plmbase_config.h"
#include "itk_image_type.h"
#include "plm_image.h"

PLMBASE_API UCharImageType::Pointer ss_img_extract_bit (
    const Plm_image::Pointer& image,
    unsigned int bit
);
PLMBASE_API UCharImageType::Pointer ss_img_extract_bit (
    UInt32ImageType::Pointer image,
    unsigned int bit
);
PLMBASE_API UCharImageType::Pointer ss_img_extract_bit (
    UCharVecImageType::Pointer image,
    unsigned int bit
);
template<class T> PLMBASE_API typename itk::Image<typename T::ObjectType::IOPixelType, T::ObjectType::ImageDimension>::Pointer 
ss_img_extract_uchar (T im_in, unsigned int uchar_no);

PLMBASE_API void ss_img_insert_uchar (
    UCharVecImageType::Pointer vec_img,
    UCharImageType::Pointer uchar_img,
    unsigned int uchar_no
);

#endif
