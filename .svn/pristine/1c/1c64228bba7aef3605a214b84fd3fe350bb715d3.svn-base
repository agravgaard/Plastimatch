/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_mask_h_
#define _itk_mask_h_

#include "plmutil_config.h"
#include "itk_image_type.h"

enum Mask_operation {
    MASK_OPERATION_FILL,
    MASK_OPERATION_MASK
};

#if defined (commentout)
template <class T, class U>
T
vector_mask_image (T& vf_image, U& ref_image);

template <class T>
T
vector_mask_image (T& image, float x_spacing,
			float y_spacing, float z_spacing);
template <class T>
T
vector_mask_image (T& vf_image, float* origin, float* spacing, int* size);

template <class T>
T
vector_mask_image (T& vf_image, Plm_image_header* pih);
#endif

template <class T>
T
mask_image (
    T input,
    UCharImageType::Pointer mask,
    Mask_operation mask_operation,
    float mask_value
);

#endif
