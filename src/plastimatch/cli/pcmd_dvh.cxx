/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "dvh.h"
#include "pcmd_dvh.h"
#include "plm_clp.h"

class Dvh_parms_pcmd {
public:
    std::string input_ss_img_fn;
    std::string input_ss_list_fn;
    std::string input_dose_fn;
    std::string output_csv_fn;
    Dvh::Dvh_units dose_units;
    Dvh::Dvh_normalization normalization;
    Dvh::Histogram_type histogram_type;
    int num_bins;
    float bin_width;
public:
    Dvh_parms_pcmd () {
        dose_units = Dvh::default_dose_units ();
        normalization = Dvh::default_normalization ();
        histogram_type = Dvh::default_histogram_type ();
        num_bins = Dvh::default_histogram_num_bins ();
        bin_width = Dvh::default_histogram_bin_width ();
    }
};

static void
usage_fn (dlib::Plm_clp *parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch dvh [options]\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Dvh_parms_pcmd *parms, 
    dlib::Plm_clp *parser, 
    int argc, 
    char *argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Input files */
    parser->add_long_option ("", "input-ss-img", 
	"structure set image file", 1, "");
    parser->add_long_option ("", "input-ss-list", 
        "structure set list file containing names and colors", 1, "");
    parser->add_long_option ("", "input-dose", 
        "dose image file", 1, "");

    /* Parameters */
    parser->add_long_option ("", "dose-units", 
        "specify units of dose in input file as either cGy as \"cgy\" "
        "or Gy as \"gy\" (default=\"gy\")", 1, "");
    parser->add_long_option ("", "cumulative", 
        "create a cumulative DVH (this is the default)", 0);
    parser->add_long_option ("", "differential", 
        "create a differential DVH instead of a cumulative DVH", 0);
    parser->add_long_option ("", "normalization", 
        "specify histogram values as either voxels \"vox\" or percent "
        "\"pct\" (default=\"pct\")", 1, "");
    parser->add_long_option ("", "num-bins", 
        "specify number of bins in the histogram (default=256)", 1, "");
    parser->add_long_option ("", "bin-width", 
        "specify bin width in the histogram in units of Gy "
        "(default=0.5)", 1, "");

    /* Output files */
    parser->add_long_option ("", "output-csv", 
        "file to save dose volume histogram data in csv format", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that input file were given */
    if (!parser->have_option ("input-dose"))
    {
	throw (dlib::error ("Error.  You must specify an input dose "
                " with --input-dose"));
    }
    if (!parser->have_option ("input-ss-img"))
    {
	throw (dlib::error ("Error.  You must specify an input structure "
                "set with --input-ss-img image"));
    }

    /* Check that an output file was given */
    if (!parser->have_option ("output-csv"))
    {
	throw (dlib::error (
                "Error.  You must specify an output file with --output-csv"));
    }

    /* Copy values into output struct */
    parms->input_ss_img_fn = parser->get_string("input-ss-img");
    if (parser->have_option ("input-ss-list")) {
        parms->input_ss_list_fn = parser->get_string("input-ss-list");
    }
    parms->input_dose_fn = parser->get_string("input-dose");
    parms->output_csv_fn = parser->get_string("output-csv");
    if (parser->have_option ("dose-units")) {
        if (parser->get_string("dose-units") == "cGy" 
            || parser->get_string("dose-units") == "cgy")
        {
            parms->dose_units = Dvh::DVH_UNITS_CGY;
        }
    }
    if (parser->have_option ("normalization")) {
        if (parser->get_string("normalization") == "vox") {
            parms->normalization = Dvh::DVH_NORMALIZATION_VOX;
        }
    }
    if (parser->have_option ("differential")) {
        parms->histogram_type = Dvh::DVH_DIFFERENTIAL_HISTOGRAM;
    }
    if (parser->have_option ("num-bins")) {
        parms->num_bins = parser->get_int ("num-bins");
    }
    if (parser->have_option ("bin-width")) {
        parms->bin_width = parser->get_float ("bin-width");
    }
}

void
do_command_dvh (int argc, char *argv[])
{
    Dvh_parms_pcmd parms;
    
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    Dvh dvh;
    dvh.set_structure_set_image (
        parms.input_ss_img_fn.c_str(), 
        parms.input_ss_list_fn.c_str());
    dvh.set_dose_image (
        parms.input_dose_fn.c_str());
    dvh.set_dose_units (parms.dose_units);
    dvh.set_dvh_parameters (
        parms.normalization,
        parms.histogram_type, 
        parms.num_bins,
        parms.bin_width);

    dvh.run ();

    dvh.save_csv (parms.output_csv_fn.c_str());
}
