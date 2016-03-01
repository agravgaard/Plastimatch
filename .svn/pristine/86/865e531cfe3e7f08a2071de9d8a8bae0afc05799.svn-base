/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "itk_image_load.h"
#include "itk_mask.h"
#include "pcmd_mask.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "plm_image_type.h"
#include "print_and_exit.h"

class Mask_parms {
public:
    std::string input_fn;
    std::string output_fn;
    std::string mask_fn;
    enum Mask_operation mask_operation;
    float mask_value;
    bool output_dicom;
    Plm_image_type output_type;
public:
    Mask_parms () {
	mask_operation = MASK_OPERATION_FILL;
	mask_value = 0.;
	output_dicom = false;
	output_type = PLM_IMG_TYPE_UNDEFINED;
    }
};

static void
mask_main (Mask_parms* parms)
{
    Plm_image::Pointer img
        = plm_image_load_native (parms->input_fn);
    if (!img) {
	print_and_exit ("Error: could not open '%s' for read\n",
	    parms->input_fn.c_str());
    }

    UCharImageType::Pointer mask = itk_image_load_uchar (parms->mask_fn, 0);

    switch (img->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	img->m_itk_uchar = mask_image (img->m_itk_uchar, mask, 
	    parms->mask_operation, parms->mask_value);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	img->m_itk_short = mask_image (img->m_itk_short, mask, 
	    parms->mask_operation, parms->mask_value);
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	img->m_itk_ushort = mask_image (img->m_itk_ushort, mask, 
	    parms->mask_operation, parms->mask_value);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	img->m_itk_uint32 = mask_image (img->m_itk_uint32, mask, 
	    parms->mask_operation, parms->mask_value);
	break;
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_ITK_FLOAT:
	img->m_itk_float = mask_image (img->itk_float(), mask, 
	    parms->mask_operation, parms->mask_value);
	break;
    default:
	print_and_exit ("Unhandled conversion in mask_main\n");
	break;
    }

    if (parms->output_dicom) {
	img->save_short_dicom (parms->output_fn, 0);
    } else {
	if (parms->output_type) {
	    img->convert (parms->output_type);
	}
	img->save_image (parms->output_fn);
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options]\n", argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Mask_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Input files */
    parser->add_long_option ("", "input", 
	"input directory or filename; "
	"can be an image or dicom directory", 1, "");
    parser->add_long_option ("", "mask", 
	"input filename for mask image", 1, "");
    parser->add_long_option ("", "output", 
	"output filename (for image file) or "
	"directory (for dicom)", 1, "");

    /* Output options */
    parser->add_long_option ("", "output-format", 
	"arg should be \"dicom\" for dicom output", 1, "");
    parser->add_long_option ("", "output-type", 
	"type of output image, one of {uchar, short, float, ...}", 1, "");

    /* Algorithm options */
    parser->add_long_option ("", "mask-value", 
	"value to set for pixels with mask (for \"fill\"), "
	"or outside of mask (for \"mask\"", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check for required options */
    parser->check_required ("input");
    parser->check_required ("mask");
    parser->check_required ("output");

    /* Input files */
    parms->input_fn = parser->get_string("input");
    parms->output_fn = parser->get_string("output");
    parms->mask_fn = parser->get_string("mask");

    /* Output options */
    if (parser->option("output-format")) {
	std::string arg = parser->get_string ("output-format");
	if (arg != "dicom") {
	    throw (dlib::error ("Error. Unknown --output-format argument: " 
		    + arg));
	}
        parms->output_dicom = true;
    }
    if (parser->option("output-type")) {
	std::string arg = parser->get_string ("output-type");
	parms->output_type = plm_image_type_parse (arg.c_str());
	if (parms->output_type == PLM_IMG_TYPE_UNDEFINED) {
	    throw (dlib::error ("Error. Unknown --output-type argument: " 
		    + arg));
	}
    }

    /* Algorithm options */
    if (parser->option("mask-value")) {
	parms->mask_value = parser->get_float("mask-value");
    }
}

void
do_command_mask (int argc, char *argv[])
{
    Mask_parms parms;

    /* Check if we're doing fill or mask */
    if (!strcmp (argv[1], "mask")) {
	parms.mask_operation = MASK_OPERATION_MASK;
    } else {
	parms.mask_operation = MASK_OPERATION_FILL;
    }

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    /* Do the masking */
    mask_main (&parms);

    printf ("Finished!\n");
}
