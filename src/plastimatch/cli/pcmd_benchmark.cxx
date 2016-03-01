/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <list>

#include "pcmd_benchmark.h"
#include "plm_clp.h"

class Benchmark_parms {
public:
    std::string output_fn;
    std::string input_fn;
};

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] input_file\n", argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Benchmark_parms* parms, 
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

    /* Check that one, and only one, input file was given */
    if (parser->number_of_arguments() == 0) {
	throw (dlib::error ("Error.  You must specify an input file"));
	
    } else if (parser->number_of_arguments() > 1) {
	std::string extra_arg = (*parser)[1];
	throw (dlib::error ("Error.  Unknown option " + extra_arg));
    }

    /* Copy input filenames to parms struct */
    parms->input_fn = (*parser)[0];

    /* Output files */
    parms->output_fn = parser->get_string("output");
}

void
do_command_benchmark (int argc, char *argv[])
{
    Benchmark_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);
}
