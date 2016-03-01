/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_resample_h_
#define _itk_resample_h_

#include "plmbase_config.h"
#include "itk_image_type.h"

class Plm_image_header;

template <class T> T
vector_resample_image (const T& vf_image, const Plm_image_header* pih);

template <class T> T
resample_image (T& image, const Plm_image_header* pih, 
    float default_val, int interp_lin);
template <class T> T
resample_image (T& image, const Plm_image_header& pih, 
    float default_val, int interp_lin);
template <class T> T
resample_image (T& image, float spacing[3]);
PLMBASE_API UCharVecImageType::Pointer
resample_image (UCharVecImageType::Pointer image, float spacing[3]);

template <class T>
T
subsample_image (T& image, int x_sampling_rate,
        int y_sampling_rate, int z_sampling_rate,
        float default_val);

#endif
