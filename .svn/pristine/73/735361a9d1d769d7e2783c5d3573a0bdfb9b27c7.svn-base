/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdlib.h>

#include "pcmd_drr.h"
#include "plm_clp.h"

class Drr_parms {
public:
    std::string input_fn;
    std::string output_fn;
    std::string output_dicom;
};

static void
do_drr (Drr_parms *parms)
{
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch drr [options] [infile]\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Drr_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

#if defined (commentout)
    /* Basic options */
    parser->add_long_option ("", "output-img", 
	"Output image filename", 1, "");
    parser->add_long_option ("", "input", 
	"Input image filename (required)", 1, "");
    parser->add_long_option ("", "bottom", 
	"Bottom of patient (top of couch)", 1, "");
    parser->add_long_option ("", "lower-threshold", 
	"Lower threshold (include voxels above this value)", 1, "");
    parser->add_long_option ("", "debug", "Create debug images", 0);
    parser->add_long_option ("", "fast", "Use reduced image size", 0);
#endif

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

#if defined (commentout)
    /* Check that an input file was given */
    parser->check_required ("input");
    parser->check_required ("output-img");

    Segment_body *sb = &parms->sb;

    /* Copy values into output struct */
    parms->output_fn = parser->get_string("output-img");
#if defined (commentout)
    parms->output_dicom = parser->get_string("output-dicom");
#endif
    parms->input_fn = parser->get_string("input");
    if (parser->option ("lower-threshold")) {
	sb->m_lower_threshold = parser->get_float("lower-threshold");
    }
#if defined (commentout)
    parms->upper_threshold = parser->get_float("upper-threshold");
#endif
    if (parser->option ("bottom")) {
	sb->m_bot_given = true;
	sb->m_bot = parser->get_float ("bottom");
    }
    if (parser->option ("fast")) {
	sb->m_fast = true;
    }
    if (parser->option ("debug")) {
	sb->m_debug = true;
    }
#endif
}

void
do_command_drr (int argc, char *argv[])
{
    Drr_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    do_drr (&parms);
}
