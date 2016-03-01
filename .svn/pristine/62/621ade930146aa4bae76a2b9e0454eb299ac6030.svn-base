/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include <string>
#include "itk_image_load.h"
#include "itk_image_type.h"
#include "pcmd_jacobian.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "vf_jacobian.h"

class Jacobian_parms {
public:
    std::string input_fn;
    std::string outputimg_fn;
    std::string outputstats_fn;
public:
    Jacobian_parms () {
	outputimg_fn = " ";
	outputstats_fn = " ";
    }
};

static void
jacobian_main (Jacobian_parms* parms)
{
    //Xform vol;
    FloatImageType::Pointer jacimage;
    std::cout << "file name: " << parms->input_fn;
    //xform_load (&vol, (const char*) parms->input_fn);
    DeformationFieldType::Pointer vol = itk_image_load_float_field (
        parms->input_fn);
    std::cout << "...loaded xf!" << std::endl;

    /* Make jacobian */
    Jacobian jacobian;
    jacobian.set_input_vf (vol);
    jacobian.set_output_vfstats_name (parms->outputstats_fn);

    
    jacimage=jacobian.make_jacobian();
    Plm_image img;
    img.init();
    img.set_itk( jacimage);
    img.save_image(parms->outputimg_fn);
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
    Jacobian_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Input files */
    parser->add_long_option ("", "input", 
        "input directory or filename of image", 1, "");

    /* Output files */
    parser->add_long_option ("", "output-img", 
        "output image; can be mha, mhd, nii, nrrd, or other format "
        "supported by ITK", 1, "");

    /* Output files */
    parser->add_long_option ("", "output-stats", 
        "output stats file; .txt format", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an input file was given */
    if (!parser->option ("input")) {
        throw (dlib::error ("Error.  Please specify an input file "));
    }

    /* Check that an output file was given */
    if (!parser->option ("output-img")) {
        throw (dlib::error ("Error.  Please specify an output image file "));
    }

    /* Check that no extraneous options were given */
    if (parser->number_of_arguments() != 0) {
        std::string extra_arg = (*parser)[0];
        throw (dlib::error ("Error.  Unknown option " + extra_arg));
    }

    /* Input/output files */
    parms->input_fn = parser->get_string("input").c_str();
    parms->outputimg_fn = parser->get_string("output-img").c_str();
    parms->outputstats_fn = parser->get_string("output-stats").c_str();

    /* Other options */
    std::string arg = parser->get_string ("output-stats");
}

void
do_command_jacobian (int argc, char *argv[])
{
    Jacobian_parms parms;

    /* Parse command line parameters */

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    /* Run the jacobianer */
    jacobian_main (&parms);
}
