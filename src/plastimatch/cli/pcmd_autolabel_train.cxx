/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <iostream>

#include "autolabel_trainer.h"
#include "pcmd_autolabel_train.h"
#include "plm_clp.h"

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch autolabel-train [options] command_file\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Autolabel_train_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    parser->add_default_options ();

    /* Basic options */
    parser->add_long_option ("", "input", 
	"Input directory (required)", 1, "");
    parser->add_long_option ("", "output-dir", 
	"Directory to store training data", 1, "");
    parser->add_long_option ("", "task", 
	"Training task (required), choices are "
	"{la,tsv1,tsv2}", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Check if the -h or --version were given */
    parser->check_default_options ();

    /* Check that an input file was given */
    parser->check_required ("input");

    /* Check that a csv output was given */
    parser->check_required ("output-dir");

    /* Check that a task was given */
    parser->check_required ("task");

    /* Copy values into output struct */
    parms->input_dir = parser->get_string("input").c_str();
    parms->output_dir = parser->get_string("output-dir").c_str();
    parms->task = parser->get_string("task").c_str();
}

void
do_command_autolabel_train (int argc, char *argv[])
{
    Autolabel_train_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    autolabel_train (&parms);
}
