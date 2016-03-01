/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdlib.h>

#include "pcmd_segment.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "segment_body.h"

class Segment_parms {
public:
    std::string input_fn;
    std::string output_fn;
    std::string output_dicom;

    Segment_body sb;
};

static void
do_segment (Segment_parms *parms)
{
    Plm_image in;
    Plm_image out;

    Segment_body *sb = &parms->sb;

    /* Load the input image */
    in.load_native (parms->input_fn); 
    sb->img_in = &in;
    sb->img_out = &out;

    /* Do segmentation */
    sb->do_segmentation ();

    /* Save output file */
    sb->img_out->save_image (parms->output_fn);
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch segment [options]\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Segment_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Basic options */
    parser->add_long_option ("", "output-img", 
	"Output image filename", 1, "");
#if defined (commentout)
    parser->add_long_option ("", "output-dicom", 
	"Output dicom directory (for RTSTRUCT)", 1, "");
#endif
    parser->add_long_option ("", "input", 
	"Input image filename (required)", 1, "");
    parser->add_long_option ("", "bottom", 
	"Bottom of patient (top of couch)", 1, "");
    parser->add_long_option ("", "lower-threshold", 
	"Lower threshold (include voxels above this value)", 1, "");
#if defined (commentout)
    parser->add_long_option ("", "upper-threshold", 
	"Upper threshold (include voxels below this value)", 1, "");
#endif
    parser->add_long_option ("", "debug", "Create debug images", 0);
    parser->add_long_option ("", "fast", "Use reduced image size", 0);
    parser->add_long_option ("", "fill-holes", "Fill the holes inside the mask (can be slow)", 0);
    parser->add_long_option ("", "fill-options", "Set fill holes options as radius1, radius2, radius3, itr1, itr2, itr3", 1);

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

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
    if (parser->option ("fill-holes")) {
	sb->m_fill_holes = true;
    }
    if (parser->option("fill-options")){
    parser->assign_int_6(sb->m_fill_parms, "fill-options");
    }
    if (parser->option ("debug")) {
	sb->m_debug = true;
    }
}

void
do_command_segment (int argc, char *argv[])
{
    Segment_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    do_segment (&parms);
}
