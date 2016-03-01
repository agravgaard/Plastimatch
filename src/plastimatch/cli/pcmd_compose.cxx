/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdio.h>
#include <time.h>
#include "itkImageRegionIterator.h"
#include "itkVectorLinearInterpolateImageFunction.h"

#include "itk_image_save.h"
#include "itk_image_type.h"
#include "pcmd_compose.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "xform.h"

void
vf_compose (
    DeformationFieldType::Pointer vf1,
    DeformationFieldType::Pointer vf2,
    DeformationFieldType::Pointer vf_out
)
{
    vf_out->SetRegions (vf1->GetBufferedRegion());
    vf_out->SetOrigin (vf1->GetOrigin());
    vf_out->SetSpacing (vf1->GetSpacing());
    vf_out->Allocate();

    /* No one should ever have to write code like this */
    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator vf1_it (vf1, vf1->GetRequestedRegion());
    FieldIterator vf_out_it (vf_out, vf_out->GetRequestedRegion());
    DeformationFieldType::IndexType index;
    FloatPoint3DType point_1, point_2, point_3;
    FloatVector3DType displacement_1;
    typedef itk::VectorLinearInterpolateImageFunction < 
	DeformationFieldType, float > VectorInterpolatorType;
    VectorInterpolatorType::Pointer interpolator = VectorInterpolatorType::New();
    interpolator->SetInputImage (vf2);
    VectorInterpolatorType::OutputType displacement_2;
    FloatVector3DType displacement_3;

    vf1_it.GoToBegin();
    vf_out_it.GoToBegin();
    while (!vf1_it.IsAtEnd()) {
	index = vf1_it.GetIndex();
	vf1->TransformIndexToPhysicalPoint (index, point_1);
	displacement_1 = vf1_it.Get ();
	for (int r = 0; r < 3; r++) {
	    point_2[r] = point_1[r] + displacement_1[r];
	}
        if (interpolator->IsInsideBuffer (point_2)) {
	    displacement_2 = interpolator->Evaluate (point_2);
	    for (int r = 0; r < 3; r++) {
		point_3[r] = point_2[r] + displacement_2[r];
		displacement_3[r] = point_3[r] - point_1[r];
	    }
	    vf_out_it.Set (displacement_3);
	} else {
	    for (int r = 0; r < 3; r++) {
		displacement_3[r] = 0.0;
	    }
	    vf_out_it.Set (displacement_3);
	}
	++vf_out_it;
	++vf1_it;
    }
}

static void
convert_to_itk_vf (
    Xform *this_xf,     /* I/O: The xform to convert */
    Xform *another_xf   /* I:   The other xform, which helps guessing size */
)
{
    Plm_image_header pih;

    /* Guess size for rendering vector field */
    switch (this_xf->m_type) {
    case XFORM_ITK_TRANSLATION:
    case XFORM_ITK_VERSOR:
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
	switch (another_xf->m_type) {
	case XFORM_ITK_VECTOR_FIELD:
	    pih.set_from_itk_image (another_xf->get_itk_vf());
	    break;
	case XFORM_GPUIT_BSPLINE:
	    pih.set_from_gpuit_bspline (another_xf->get_gpuit_bsp());
	    break;
	default:
	    print_and_exit (
		"Sorry, couldn't guess size to render vf.\n");
	    break;
	}
	break;
    case XFORM_ITK_VECTOR_FIELD:
	/* Do nothing */
	return;
    case XFORM_GPUIT_BSPLINE:
	pih.set_from_gpuit_bspline (this_xf->get_gpuit_bsp());
	break;
    case XFORM_ITK_TPS:
    case XFORM_GPUIT_VECTOR_FIELD:
    default:
	/* Not yet handled */
	print_and_exit (
	    "Sorry, couldn't convert xf to vf.\n");
	break;
    }
    xform_to_itk_vf (this_xf, this_xf, &pih);
}

static void
compose_main (Compose_parms* parms)
{
    Xform xf1, xf2;

    xform_load (&xf1, parms->xf_in_1_fn);
    xform_load (&xf2, parms->xf_in_2_fn);
    convert_to_itk_vf (&xf1, &xf2);
    convert_to_itk_vf (&xf2, &xf1);

    DeformationFieldType::Pointer vf_out = DeformationFieldType::New();

    vf_compose (xf1.get_itk_vf(), xf2.get_itk_vf(), vf_out);

    itk_image_save (vf_out, parms->xf_out_fn);
}

static void
print_usage (void)
{
    printf (
	"Usage: plastimatch compose file_1 file_2 outfile\n"
	"\n"
	"Note:  file_1 is applied first, and then file_2.\n"
	"          outfile = file_2 o file_1\n"
	"          x -> x + file_2(x + file_1(x))\n"
    );
    exit (-1);
}

static void
compose_parse_args (Compose_parms* parms, int argc, char* argv[])
{
    if (argc != 5) {
	print_usage ();
    }
    
    parms->xf_in_1_fn = argv[2];
    parms->xf_in_2_fn = argv[3];
    parms->xf_out_fn = argv[4];
}

void
do_command_compose (int argc, char *argv[])
{
    Compose_parms parms;
    
    compose_parse_args (&parms, argc, argv);

    compose_main (&parms);

    printf ("Finished!\n");
}
