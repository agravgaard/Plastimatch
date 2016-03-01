/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "ml_convert.h"
#include "pcmd_ml_convert.h"
#include "plm_clp.h"

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch ml_convert [options] feature-path [feature-path ...]\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Ml_convert* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Basic options */
    parser->add_long_option ("", "append",
	"Location of an existing input text file, to which additional "
        "features will be appended",
        1, "");
    parser->add_long_option ("", "labelmap",
	"Location of labelmap file", 1, "");
    parser->add_long_option ("", "mask",
	"Location of mask file", 1, "");
    parser->add_long_option ("", "input-ml-results", 
	"Location of the file containing the results in text format", 1, "");
    parser->add_long_option ("", "output",
	"Location of output file to be written; if --input-ml-results "
        "is specified, an image will be written, otherwise a text file "
        "will be written", 1, "");
    parser->add_long_option ("", "output-type",
	"Data type of output image file (either \"uchar\" or \"float\")",
        1, "uchar");
    parser->add_long_option ("", "output-format",
	"Output format, either \"libsvm\" or \"vw\", default is \"vw\"", 
        1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that all necessary inputs are given */
    if (!parser->have_option("input-ml-results")
        && !parser->have_option("labelmap") 
        && !parser->have_option("append"))
    {
        throw (dlib::error ("Error.  Must specify either "
                "--labelmap or --append option"));
    }
    if (parser->have_option("input-ml-results") 
        && !parser->have_option("output"))
    {
	throw (dlib::error ("Error.  When --input-ml-results is used, "
                "you must also include --output"));
    }
    if (parser->have_option("input-ml-results") 
        && parser->have_option("append"))
    {
	throw (dlib::error ("Error.  When --input-ml-results is used, "
                "you must not include --append"));
    }
    if (parser->have_option("labelmap") && parser->have_option("append")) {
	throw (dlib::error ("Error.  Do not specify both --labelmap and "
                "--append option"));
    }
    if (!parser->have_option("output") && !parser->have_option("append")) {
	throw (dlib::error ("Error.  You must specify either --output or "
                "--append option"));
    }
    if (parser->have_option("input-ml-results") 
        && !parser->have_option("mask")
        && !parser->have_option("labelmap"))
    {
	throw (dlib::error ("Error.  When --input-ml-results is used, "
                "you must also include --labelmap or --mask"));
    }

    /* Copy values into output struct */
    parms->set_append_filename (parser->get_string ("append"));
    parms->set_input_ml_results_filename (
        parser->get_string ("input-ml-results"));
    parms->set_label_filename (parser->get_string ("labelmap"));
    parms->set_mask_filename (parser->get_string ("mask"));
    if (parser->have_option ("output")) {
        parms->set_output_filename (parser->get_string ("output"));
    } else {
        parms->set_output_filename (parser->get_string ("append"));
    }
    parms->set_output_type (parser->get_string ("output-type"));
    parms->set_output_format (parser->get_string ("output-format"));

    /* Copy input filenames to parms struct */
    for (unsigned long i = 0; i < parser->number_of_arguments(); i++) {
        parms->add_feature_path ((*parser)[i]);
    }
}

void
do_command_ml_convert (int argc, char *argv[])
{
    Ml_convert ml_convert;

    plm_clp_parse (&ml_convert, &parse_fn, &usage_fn, argc, argv, 1);

    ml_convert.run();
}
