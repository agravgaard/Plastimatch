/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "mabs.h"
#include "mabs_parms.h"
#include "pcmd_mabs.h"
#include "plm_clp.h"

class Mabs_parms_pcmd {
public:
    bool atlas_selection;
    bool train_atlas_selection;
    bool convert;
    bool prealign;
    bool train_registration;
    bool train;
    bool segment;

    std::string cmd_file_fn;
    std::string input_fn;
    std::string input_roi_fn;
    std::string output_dir;
    std::string output_dicom_dir;

public:
    Mabs_parms_pcmd () {
        atlas_selection = false;
        train_atlas_selection = false;
        convert = false;
        prealign = false;
        train = false;
        train_registration = false;
        segment = false;
    }
};

static void
usage_fn (dlib::Plm_clp *parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch mabs [options] command_file\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Mabs_parms_pcmd *parms, 
    dlib::Plm_clp *parser, 
    int argc, 
    char *argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Parameters */
    parser->add_long_option ("", "atlas-selection", 
        "run just atlas selection", 1, "");
    parser->add_long_option ("", "train-atlas-selection", 
        "run just train atlas selection", 0);
    parser->add_long_option ("", "convert", 
        "pre-process atlas", 0);
    parser->add_long_option ("", "pre-align", 
        "pre-process atlas", 0);
    parser->add_long_option ("", "train", 
        "perform full training to find the best registration "
        " and segmentation parameters", 0);
    parser->add_long_option ("", "train-registration", 
        "perform limited training to find the best registration "
        "parameters only", 0);
    parser->add_long_option ("", "segment", 
        "use mabs to segment the specified image or directory", 1, "");
    parser->add_long_option ("", "input-roi", 
        "input mask used to prealign the center of gravity", 1, "");
    parser->add_long_option ("", "output", 
        "output (non-dicom) directory when doing a segmentation", 1, "");
    parser->add_long_option ("", "output-dicom", 
        "output dicom directory when doing a segmentation", 1, "");

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

    /* Parameters */
    if (parser->have_option ("atlas-selection")) {
        parms->atlas_selection = true;
        parms->input_fn = parser->get_string ("atlas-selection");
    }
    if (parser->have_option ("train-atlas-selection")) {
        parms->train_atlas_selection = true;
    }
    if (parser->have_option ("convert")) {
        parms->convert = true;
    }
    if (parser->have_option ("pre-align")) {
        parms->prealign = true;
    }
    if (parser->have_option ("train")) {
        parms->train = true;
    }
    if (parser->have_option ("train-registration")) {
        parms->train_registration = true;
    }
    if (parser->have_option ("segment")) {
        parms->segment = true;
        parms->input_fn = parser->get_string ("segment");
    }
    if (parser->have_option ("input-roi")) {
        parms->input_roi_fn = parser->get_string ("input-roi");
    }
    if (parser->have_option ("output")) {
        parms->output_dir = parser->get_string ("output");
    }
    if (parser->have_option ("output-dicom")) {
        parms->output_dicom_dir = parser->get_string ("output-dicom");
    }
}

void
do_command_mabs (int argc, char *argv[])
{
    Mabs_parms_pcmd parms;
    
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    Mabs_parms mabs_parms;
    mabs_parms.parse_config (parms.cmd_file_fn.c_str());

    Mabs mabs;
    mabs.set_parms (&mabs_parms);
    
    /* If defined set input ROI file name */
    if (parms.input_roi_fn != "") {
        mabs.set_prealign_roi_cmd_name (parms.input_roi_fn);
    }

    /* Run the right function */
    if (parms.train_atlas_selection) {
        mabs.set_executed_command("train_atlas_selection");
        mabs.train_atlas_selection ();
    }
    else if (parms.convert) {
        mabs.set_executed_command("convert");
        mabs.atlas_convert ();
    }
    else if (parms.prealign) {
        mabs.set_executed_command("prealign");
        mabs.atlas_prealign ();
    }
    else if (parms.train_registration) {
        mabs.set_executed_command("train_registration");
        mabs.train_registration ();
    }
    else if (parms.train) {
        mabs.set_executed_command("train");
        mabs.train ();
    }
    else { // can be mabs.atlas_selection() or mabs.segment()
        
        /* Set parameters */
        if (parms.input_fn != "") {
            mabs.set_segment_input (parms.input_fn);
        }
        if (parms.output_dir != "") {
            mabs.set_segment_output (parms.output_dir);
        }
        if (parms.output_dicom_dir != "") {
            mabs.set_segment_output_dicom (parms.output_dicom_dir);
        }
       
        /* Run function */
        if (parms.atlas_selection) {
            mabs.set_executed_command("atlas_selection");
            mabs.atlas_selection ();
        }
        else {
            mabs.set_executed_command("segment");
            mabs.segment ();
        }
    }
}
