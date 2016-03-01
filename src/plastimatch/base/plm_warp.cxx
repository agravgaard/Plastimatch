/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <time.h>

#include "bspline_warp.h"
#include "bspline_xform.h"
#include "itk_image_type.h"
#include "itk_warp.h"
#include "mha_io.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_warp.h"
#include "print_and_exit.h"
#include "volume.h"
#include "xform.h"

static void
plm_warp_itk (
    Plm_image::Pointer& im_warped,     /* Output (optional) */
    DeformationFieldType::Pointer *vf_out,   /* Output (optional) */
    const Xform::Pointer& xf_in,             /* Input */
    Plm_image_header *pih,                   /* Input */
    const Plm_image::Pointer& im_in,                        /* Input */
    float default_val,     /* Input:  Value for pixels without match */
    int interp_lin         /* Input:  Trilinear (1) or nn (0) */
)
{
    Xform xform_tmp;
    DeformationFieldType::Pointer vf;

    /* Create an itk vector field from xf_in */
    printf ("plm_warp_itk: xform_to_itk_vf\n");
    xform_to_itk_vf (&xform_tmp, xf_in.get(), pih);
    vf = xform_tmp.get_itk_vf ();

    /* If caller wants the vf, we assign it here */
    if (vf_out) {
	*vf_out = vf;
    }

    /* If caller only wants the vf, we are done */
    if (!im_warped) {
	return;
    }

    /* Convert GPUIT images to ITK */
    printf ("plm_warp_itk: convert_to_itk\n");
    im_in->convert_to_itk ();

    /* Warp the image */
    printf ("plm_warp_itk: warping...\n");
    switch (im_in->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	im_warped->m_itk_uchar = itk_warp_image (
	    im_in->m_itk_uchar, 
	    vf, 
	    interp_lin, 
	    static_cast<unsigned char>(default_val));
	im_warped->m_original_type = PLM_IMG_TYPE_ITK_UCHAR;
	im_warped->m_type = PLM_IMG_TYPE_ITK_UCHAR;
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	im_warped->m_itk_short = itk_warp_image (
	    im_in->m_itk_short, 
	    vf, 
	    interp_lin, 
	    static_cast<short>(default_val));
	im_warped->m_original_type = PLM_IMG_TYPE_ITK_SHORT;
	im_warped->m_type = PLM_IMG_TYPE_ITK_SHORT;
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	im_warped->m_itk_ushort = itk_warp_image (
	    im_in->m_itk_ushort, 
	    vf, 
	    interp_lin, 
	    static_cast<unsigned short>(default_val));
	im_warped->m_original_type = PLM_IMG_TYPE_ITK_USHORT;
	im_warped->m_type = PLM_IMG_TYPE_ITK_USHORT;
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	im_warped->m_itk_uint32 = itk_warp_image (
	    im_in->m_itk_uint32, 
	    vf, 
	    interp_lin, 
	    static_cast<uint32_t>(default_val));
	im_warped->m_original_type = PLM_IMG_TYPE_ITK_ULONG;
	im_warped->m_type = PLM_IMG_TYPE_ITK_ULONG;
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	im_warped->m_itk_float = itk_warp_image (
	    im_in->m_itk_float, 
	    vf, 
	    interp_lin, 
	    static_cast<float>(default_val));
	im_warped->m_original_type = PLM_IMG_TYPE_ITK_FLOAT;
	im_warped->m_type = PLM_IMG_TYPE_ITK_FLOAT;
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	im_warped->m_itk_double = itk_warp_image (
	    im_in->m_itk_double, 
	    vf, 
	    interp_lin, 
	    static_cast<double>(default_val));
	im_warped->m_original_type = PLM_IMG_TYPE_ITK_DOUBLE;
	im_warped->m_type = PLM_IMG_TYPE_ITK_DOUBLE;
	break;
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	im_warped->m_itk_uchar_vec = itk_warp_image (
	    im_in->m_itk_uchar_vec, 
	    vf, 
	    interp_lin, 
	    static_cast<unsigned char> (default_val));
	im_warped->m_original_type = PLM_IMG_TYPE_ITK_UCHAR_VEC;
	im_warped->m_type = PLM_IMG_TYPE_ITK_UCHAR_VEC;
	break;
    case PLM_IMG_TYPE_ITK_CHAR:
    case PLM_IMG_TYPE_ITK_LONG:
    default:
	print_and_exit ("Unhandled case in plm_warp_itk (%s)\n",
	    plm_image_type_string (im_in->m_type));
	break;
    }
}

/* Native warping (only gpuit bspline + float) */
static void
plm_warp_native (
    Plm_image::Pointer& im_warped,        /* Output */
    DeformationFieldType::Pointer *vf,    /* Output */
    const Xform::Pointer& xf_in,          /* Input */
    Plm_image_header *pih,                /* Input */
    const Plm_image::Pointer& im_in,      /* Input */
    float default_val,     /* Input:  Value for pixels without match */
    int interp_lin         /* Input:  Trilinear (1) or nn (0) */
)
{
    Xform xf_tmp;
    Xform vf_tmp;
    Bspline_xform* bxf_in = xf_in->get_gpuit_bsp ();
    Volume *vf_out = 0;     /* Output vector field */
    Volume *v_out = 0;      /* Output warped image */
    plm_long dim[3];
    float origin[3];
    float spacing[3];
    float direction_cosines[9];

    /* Convert input image to gpuit format */
    printf ("Running: plm_warp_native\n");
    printf ("Converting input image...\n");
    Volume::Pointer v_in = im_in->get_volume_float ();

    /* Transform input xform to gpuit bspline with correct voxel spacing */
    printf ("Converting xform...\n");
    xform_to_gpuit_bsp (&xf_tmp, xf_in.get(), pih, bxf_in->grid_spac);

    /* Create output vf */
    pih->get_origin (origin);
    pih->get_spacing (spacing);
    pih->get_dim (dim);
    pih->get_direction_cosines (direction_cosines);
    if (vf) {
	printf ("Creating output vf...\n");
	vf_out = new Volume (dim, origin, spacing, direction_cosines,
	    PT_VF_FLOAT_INTERLEAVED, 3);
    }

    /* Create output image */
    printf ("Creating output volume...\n");
    v_out = new Volume (dim, origin, spacing, direction_cosines, 
	PT_FLOAT, 1);

    /* Warp using gpuit native warper */
    printf ("Running native warper...\n");
    bspline_warp (v_out, vf_out, xf_tmp.get_gpuit_bsp(), v_in, 
	interp_lin, default_val);

    /* Return output image to caller */
    if (im_warped) {
	im_warped->set_volume (v_out);

	/* Bspline_warp only operates on float.  We need to back-convert */
	printf ("Back convert to original type...\n");
	im_warped->convert (im_in->m_original_type);
	im_warped->m_original_type = im_in->m_original_type;
    } else {
	delete v_out;
    }

    /* Return vf to caller */
    if (vf) {
	printf ("> Convert vf to itk\n");
	*vf = xform_gpuit_vf_to_itk_vf (vf_out, 0);
	printf ("> Conversion complete.\n");
	delete vf_out;
    }
    printf ("plm_warp_native is complete.\n");
}

/* Native vector warping (only gpuit bspline + uchar_vec) */
static void
plm_warp_native_vec (
    Plm_image::Pointer& im_warped,        /* Output */
    DeformationFieldType::Pointer *vf,    /* Output */
    const Xform::Pointer& xf_in,          /* Input */
    Plm_image_header *pih,                /* Input */
    const Plm_image::Pointer& im_in,      /* Input */
    float default_val,     /* Input:  Value for pixels without match */
    int interp_lin         /* Input:  Trilinear (1) or nn (0) */
)
{
    Xform xf_tmp;
    Xform vf_tmp;
    Bspline_xform* bxf_in = xf_in->get_gpuit_bsp ();
    Volume *vf_out = 0;     /* Output vector field */
    Volume *v_out = 0;      /* Output warped image */
    plm_long dim[3];
    float origin[3];
    float spacing[3];
    float direction_cosines[9];

    /* Convert input image to gpuit format */
    printf ("Running: plm_warp_native_vec\n");
    printf ("Converting input image...\n");
    Volume::Pointer v_in = im_in->get_volume_uchar_vec ();

    /* Transform input xform to gpuit bspline with correct voxel spacing */
    printf ("Converting xform...\n");
    xform_to_gpuit_bsp (&xf_tmp, xf_in.get(), pih, bxf_in->grid_spac);

    /* Create output vf */
    pih->get_origin (origin);
    pih->get_spacing (spacing);
    pih->get_dim (dim);
    pih->get_direction_cosines (direction_cosines);
    if (vf) {
	printf ("Creating output vf...\n");
	vf_out = new Volume (dim, origin, spacing, direction_cosines,
	    PT_VF_FLOAT_INTERLEAVED, 3);
    }

    /* Create output image */
    printf ("Creating output volume (%d planes)...\n", v_in->vox_planes);
    v_out = new Volume (dim, origin, spacing, direction_cosines, 
	PT_UCHAR_VEC_INTERLEAVED, v_in->vox_planes);

    /* Warp using gpuit native warper */
    printf ("Running native warper...\n");
    bspline_warp (v_out, vf_out, xf_tmp.get_gpuit_bsp(), v_in, 
	interp_lin, default_val);

    /* Return output image to caller */
    if (im_warped) {
	im_warped->set_volume (v_out);

	/* Bspline_warp only operates on float.  We need to back-convert */
	printf ("Back convert to original type...\n");
	im_warped->convert (im_in->m_original_type);
	im_warped->m_original_type = im_in->m_original_type;
    } else {
	delete v_out;
    }

    /* Return vf to caller */
    if (vf) {
	printf ("> Convert vf to itk\n");
	*vf = xform_gpuit_vf_to_itk_vf (vf_out, 0);
	printf ("> Conversion complete.\n");
	delete vf_out;
    }
    printf ("plm_warp_native is complete.\n");
}

void
plm_warp (
    Plm_image::Pointer& im_warped,  /* Output: Output image (optional) */
    DeformationFieldType::Pointer* vf,    /* Output: Output vf (optional) */
    const Xform::Pointer& xf_in, /* Input:  Input image warped by this xform */
    Plm_image_header *pih, /* Input:  Size of output image */
    const Plm_image::Pointer& im_in,      /* Input:  Input image */
    float default_val,     /* Input:  Value for pixels without match */
    int use_itk,           /* Input:  Force use of itk (1) or not (0) */
    int interp_lin         /* Input:  Trilinear (1) or nn (0) */
)
{
    /* If user requested ITK-based warping, respect their wish */
    if (use_itk) {
	plm_warp_itk (im_warped, vf, xf_in, pih, im_in, default_val,
	    interp_lin);
	return;
    }

    /* Otherwise, try to do native warping where possible */
    if (xf_in->m_type == XFORM_GPUIT_BSPLINE) {
	switch (im_in->m_type) {
	case PLM_IMG_TYPE_ITK_UCHAR:
	case PLM_IMG_TYPE_ITK_SHORT:
	case PLM_IMG_TYPE_ITK_ULONG:
	case PLM_IMG_TYPE_ITK_FLOAT:
	case PLM_IMG_TYPE_GPUIT_UCHAR:
	case PLM_IMG_TYPE_GPUIT_SHORT:
	case PLM_IMG_TYPE_GPUIT_UINT32:
	case PLM_IMG_TYPE_GPUIT_FLOAT:
	    plm_warp_native (im_warped, vf, xf_in, pih, im_in, default_val,
		interp_lin);
	    break;
	case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	case PLM_IMG_TYPE_GPUIT_UCHAR_VEC:
	    plm_warp_native_vec (im_warped, vf, xf_in, pih, im_in, 
		default_val, interp_lin);
	    break;
	default:
	    plm_warp_itk (im_warped, vf, xf_in, pih, im_in, default_val,
		interp_lin);
	    break;
	}
    } else {
	plm_warp_itk (im_warped, vf, xf_in, pih, im_in, default_val,
	    interp_lin);
    }
}
