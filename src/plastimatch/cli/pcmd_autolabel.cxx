/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <iostream>

#include "autolabel.h"
#include "autolabel_parms.h"
#include "plm_clp.h"

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch autolabel [options] command_file\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Autolabel_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Basic options */
    parser->add_long_option ("", "train", 
	"Run training on the problem specified in the command file");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that only one argument was given */
    if (parser->number_of_arguments() != 1) {
	throw (dlib::error (
                "Error.  Only one configuration file can be used."));
    }

    /* Get filename of command file */
    parms->cmd_file_fn = (*parser)[0].c_str();
}

void
do_command_autolabel (int argc, char *argv[])
{
    Autolabel_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    autolabel (&parms);
}
