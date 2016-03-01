/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "itk_union.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "plm_file_format.h"
#include "xform.h"

class Union_parms {
public:
    std::string input_1_fn;
    std::string input_2_fn;
    std::string output_fn;
};

void
union_main (Union_parms *parms)
{
    /* Load the inputs */
    Plm_image img_1 (parms->input_1_fn, PLM_IMG_TYPE_ITK_UCHAR);
    Plm_image img_2 (parms->input_2_fn, PLM_IMG_TYPE_ITK_UCHAR);

    /* Make the union */
    UCharImageType::Pointer itk_out = itk_union (
        img_1.itk_uchar(), img_2.itk_uchar());

    /* Save it */
    Plm_image img_out (itk_out);
    img_out.save_image (parms->output_fn);
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] input_1 input_2\n", argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Union_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", 
        "filename for output image", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an output file was given */
    if (!parser->option ("output")) {
	throw (dlib::error ("Error.  Please specify an output file "
		"using the --output option"));
    }

    /* Copy input filenames to parms struct.  Two input files 
       must be specified. */
    if (parser->number_of_arguments() != 2) {
	throw (dlib::error ("Error.  You must specify two input files"));
    }
    parms->input_1_fn = (*parser)[0];
    parms->input_2_fn = (*parser)[1];

    /* Output files */
    parms->output_fn = parser->get_string("output").c_str();
}

void
do_command_union (int argc, char *argv[])
{
    Union_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    union_main (&parms);
}
