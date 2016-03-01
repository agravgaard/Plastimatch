/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <list>

#include "itk_scale.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "plm_file_format.h"
#include "xform.h"

class Scale_parms {
public:
    std::string output_fn;
    std::string input_fn;
    float weight;
public:
    Scale_parms () {
        weight = 1.0;
    }
};

static void
scale_image (Plm_image::Pointer& img, float weight)
{
    img->set_itk (itk_scale (img->itk_float(), weight));
}

static void
scale_vf (Xform *xf, float weight)
{
    xf->set_itk_vf (itk_scale (xf->get_itk_vf(), weight));
}

void
scale_vf_main (Scale_parms *parms)
{
    /* Load the input */
    Xform xf;
    xf.load (parms->input_fn);

    /* Weigh it */
    scale_vf (&xf, parms->weight);

    /* Save it */
    xf.save (parms->output_fn);
}

void
scale_vol_main (Scale_parms *parms)
{
    /* Load the input */
    Plm_image::Pointer img = plm_image_load (parms->input_fn, 
        PLM_IMG_TYPE_ITK_FLOAT);

    /* Weigh it */
    scale_image (img, parms->weight);

    /* Save it */
    img->convert_to_original_type ();
    img->save_image (parms->output_fn);
}

void
scale_main (Scale_parms *parms)
{
    /* What is the input file type? */
    Plm_file_format file_format = plm_file_format_deduce (parms->input_fn);

    switch (file_format) {
    case PLM_FILE_FMT_VF:
        scale_vf_main (parms);
        break;
    default:
        scale_vol_main (parms);
        break;
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] input_file\n", argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Scale_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", 
        "filename for output image or vector field", 1, "");

    /* Weight vector */
    parser->add_long_option ("", "weight", 
        "scale the input image or vector field by this value (float)", 
        1, "1.0");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an output file was given */
    if (!parser->option ("output")) {
	throw (dlib::error ("Error.  Please specify an output file "
		"using the --output option"));
    }

    /* Check that one, and only one, input file was given */
    if (parser->number_of_arguments() == 0) {
	throw (dlib::error ("Error.  You must specify an input file"));
	
    } else if (parser->number_of_arguments() > 1) {
	std::string extra_arg = (*parser)[1];
	throw (dlib::error ("Error.  Unknown option " + extra_arg));
    }

    /* Copy input filenames to parms struct */
    parms->input_fn = (*parser)[0];

    /* Scaling factor */
    if (parser->option ("weight")) {
        parms->weight = parser->get_float("weight");
    }

    /* Output files */
    parms->output_fn = parser->get_string("output");
}

void
do_command_scale (int argc, char *argv[])
{
    Scale_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    scale_main (&parms);
}
