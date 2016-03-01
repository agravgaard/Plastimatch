/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_warp_h_
#define _plm_warp_h_

#include "plmbase_config.h"
#include "itkBSplineDeformableTransform.h"
#include "xform.h"

class Plm_image;

/* -----------------------------------------------------------------------
   Public functions
   ----------------------------------------------------------------------- */
PLMBASE_API void
plm_warp (
    Plm_image::Pointer& im_warped,   /* Output: Output image (optional) */
    DeformationFieldType::Pointer *vf,    /* Output: Output vf (optional) */
    const Xform::Pointer& xf_in, /* Input:  Input image warped by this xform */
    Plm_image_header *pih, /* Input:  Size of output image */
    const Plm_image::Pointer& im_in,      /* Input:  Input image */
    float default_val,     /* Input:  Value for pixels without match */
    int use_itk,           /* Input:  Force use of itk (1) or not (0) */
    int interp_lin         /* Input:  Trilinear (1) or nn (0) */
);

#endif
