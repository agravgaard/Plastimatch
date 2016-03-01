/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <string.h>

#include "bspline_interpolate.h"
#include "itk_image_type.h"
#include "itk_point.h"
#include "print_and_exit.h"
#include "xform.h"
#include "xform_point.h"

/* -----------------------------------------------------------------------
   Transform points
   ----------------------------------------------------------------------- */
void
xform_point_transform_gpuit_bspline (
    FloatPoint3DType* itk_point_out, 
    Xform* xf_in, 
    FloatPoint3DType itk_point_in
)
{
    int d;
    float point_in[3], point_out[3];

    for (d = 0; d < 3; d++) {
	point_in[d] = itk_point_in[d];
    }

    bspline_transform_point (point_out, xf_in->get_gpuit_bsp(), point_in, 1);

    for (d = 0; d < 3; d++) {
	(*itk_point_out)[d] = point_out[d];
    }
}

void
xform_point_transform_itk_vf (
    FloatPoint3DType* point_out, 
    Xform* xf_in, 
    FloatPoint3DType point_in
)
{
    DeformationFieldType::Pointer vf = xf_in->get_itk_vf ();
    DeformationFieldType::IndexType idx;

    bool isInside = vf->TransformPhysicalPointToIndex (point_in, idx);
    if (isInside) {
	DeformationFieldType::PixelType pixelValue = vf->GetPixel (idx);
#if defined (commentout)
	printf ("pi [%g %g %g]\n", point_in[0], point_in[1], point_in[2]);
	printf ("idx [%ld %ld %ld]\n", idx[0], idx[1], idx[2]);
	printf ("vf [%g %g %g]\n", pixelValue[0], pixelValue[1], pixelValue[2]);
#endif
	for (int d = 0; d < 3; d++) {
	    (*point_out)[d] = point_in[d] + pixelValue[d];
	}
#if defined (commentout)
	printf ("po [%g %g %g]\n", 
	    (*point_out)[0], (*point_out)[1], (*point_out)[2]);
#endif
    } else {
	(*point_out) = point_in;
    }
}

void
xform_point_transform (
    FloatPoint3DType* point_out, 
    Xform* xf_in, 
    FloatPoint3DType point_in
)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
    case XFORM_ITK_TRANSLATION:
    case XFORM_ITK_VERSOR:
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
	print_and_exit (
	    "Sorry, xform_transform_point not defined for type %d\n",
	    xf_in->m_type);
	break;
    case XFORM_ITK_VECTOR_FIELD:
	xform_point_transform_itk_vf (point_out, xf_in, point_in);
	break;
    case XFORM_GPUIT_BSPLINE:
	xform_point_transform_gpuit_bspline (point_out, xf_in, point_in);
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
    default:
	print_and_exit (
	    "Sorry, xform_transform_point not defined for type %d\n",
	    xf_in->m_type);
	break;
    }
}
