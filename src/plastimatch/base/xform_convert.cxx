/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "plm_image_header.h"
#include "plm_int.h"
#include "print_and_exit.h"
#include "xform.h"
#include "xform_convert.h"

class Xform_convert_private
{
public:
    Xform::Pointer m_xf_out;
    Xform::Pointer m_xf_in;
public:
    Xform_convert_private () {
        m_xf_out =  Xform::New ();
    }
};

Xform_convert::Xform_convert ()
{
    d_ptr = new Xform_convert_private;

    m_xf_out_type = XFORM_NONE;

    for (int d = 0; d < 3; d++) {
        m_grid_spac[d] = 100.f;
    }
    m_nobulk = false;
}

Xform_convert::~Xform_convert ()
{
    delete d_ptr;
}

void
Xform_convert::set_input_xform (const Xform::Pointer& xf_in)
{
    d_ptr->m_xf_in = xf_in;
}

void
Xform_convert::run ()
{
    Plm_image_header pih;
    pih.set_from_volume_header (this->m_volume_header);
    Xform_type xf_in_type = d_ptr->m_xf_in->get_type();

    switch (this->m_xf_out_type) {
    case XFORM_NONE:
	print_and_exit ("Sorry, couldn't convert to XFORM_NONE\n");
	break;
    case XFORM_ITK_TRANSLATION:
	print_and_exit ("Sorry, couldn't convert to XFORM_ITK_TRANSLATION\n");
	break;
    case XFORM_ITK_VERSOR:
	print_and_exit ("Sorry, couldn't convert to XFORM_ITK_VERSOR\n");
	break;
    case XFORM_ITK_AFFINE:
	print_and_exit ("Sorry, couldn't convert to XFORM_ITK_AFFINE\n");
	break;
    case XFORM_ITK_BSPLINE:
	if (this->m_grid_spac[0] <= 0.0f) {
	    if (xf_in_type == XFORM_GPUIT_BSPLINE 
		|| xf_in_type == XFORM_ITK_BSPLINE)
	    {
		/* Use grid spacing of input bspline */
		if (this->m_nobulk) {
                    d_ptr->m_xf_out = xform_to_itk_bsp_nobulk (
			d_ptr->m_xf_in, &pih, 0);
		} else {
		    printf ("Standard case.\n");
		    pih.print ();
		    d_ptr->m_xf_out = xform_to_itk_bsp (
                        d_ptr->m_xf_in, &pih, 0);
		}
	    } else {
		print_and_exit ("Sorry, grid spacing cannot be zero\n");
	    }
	} else {
	    if (this->m_nobulk) {
                d_ptr->m_xf_out = xform_to_itk_bsp_nobulk (
                    d_ptr->m_xf_in, &pih, this->m_grid_spac);
	    } else {
                d_ptr->m_xf_out = xform_to_itk_bsp (
                    d_ptr->m_xf_in, &pih, this->m_grid_spac);
	    }
	}
	break;
    case XFORM_ITK_TPS:
	print_and_exit ("Sorry, couldn't convert to XFORM_ITK_TPS\n");
	break;
    case XFORM_ITK_VECTOR_FIELD:
	printf ("Converting to (itk) vector field\n");
	d_ptr->m_xf_out = xform_to_itk_vf (d_ptr->m_xf_in, &pih);
	break;
    case XFORM_GPUIT_BSPLINE:
	if (this->m_grid_spac[0] <=0.0f) {
	    if (xf_in_type == XFORM_GPUIT_BSPLINE 
		|| xf_in_type == XFORM_ITK_BSPLINE)
	    {
		d_ptr->m_xf_out = xform_to_gpuit_bsp (
                    d_ptr->m_xf_in, &pih, 0);
	    } else {
		print_and_exit ("Sorry, grid spacing cannot be zero for conversion to gpuit_bsp\n");
	    }
	} else {
	    d_ptr->m_xf_out = xform_to_gpuit_bsp (
                d_ptr->m_xf_in, &pih, this->m_grid_spac);
	}
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
    default:
	print_and_exit ("Sorry, couldn't convert to xform (type = %d)\n",
	    this->m_xf_out_type);
	break;
    }
}

Xform::Pointer&
Xform_convert::get_output_xform ()
{
    return d_ptr->m_xf_out;
}
