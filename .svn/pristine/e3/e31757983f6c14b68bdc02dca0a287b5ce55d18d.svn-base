/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "itk_threshold.h"
#include "itk_image_save.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "plm_math.h"
#include "pcmd_threshold.h"
#include "print_and_exit.h"
#include "string_util.h"

static void
threshold_main (Pcmd_threshold* parms)
{
    Plm_image::Pointer plm_image = plm_image_load (
	parms->img_in_fn, 
	PLM_IMG_TYPE_ITK_FLOAT);
    FloatImageType::Pointer img_in = plm_image->m_itk_float;
    UCharImageType::Pointer img_out;

    if (parms->range_string != "") {
        img_out = itk_threshold (img_in, parms->range_string);
    }

    if (parms->output_dicom) {
	itk_image_save_short_dicom (
	    img_out, parms->img_out_fn.c_str(), 0);
    } else {
        Plm_image pli (img_out);
	if (parms->output_type) {
	    pli.convert (parms->output_type);
	}
	pli.save_image (parms->img_out_fn);
    }
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
    Pcmd_threshold* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Input files */
    parser->add_long_option ("", "input", 
        "input directory or filename", 1, "");

    /* Output files */
    parser->add_long_option ("", "output", "output image", 1, "");

    /* Different ways to specify threshold range */
    parser->add_long_option ("", "above", 
        "value above which output has value high", 1, "");
    parser->add_long_option ("", "below", 
        "value below which output has value high", 1, "");
    parser->add_long_option ("", "range", 
        "a string that forms a list of threshold ranges of the form "
        "\"r1-lo,r1-hi,r2-lo,r2-hi,...\", "
        "such that voxels with intensities within any of the ranges "
        "([r1-lo,r1-hi], [r2-lo,r2-hi], ...) have output value high",
        1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an output file was given */
    if (!parser->option ("input")) {
	throw (dlib::error ("Error.  Please specify an input file "
		"using the --input option"));
    }

    /* Check that an output file was given */
    if (!parser->option ("output")) {
	throw (dlib::error ("Error.  Please specify an output file "
		"using the --output option"));
    }

    /* Parse range options.  Check that one and only one range 
       option was given */
    bool have_range = false;
    bool range_error = false;
    if (parser->option ("above")) {
        parms->range_string = string_format ("%f,inf", 
            parser->get_float ("above"));
        have_range = true;
    }
    if (parser->option ("below")) {
        if (have_range) {
            range_error = true;
        } else {
            parms->range_string = string_format ("-inf,%f", 
                parser->get_float ("below"));
            have_range = true;
        }
    }
    if (parser->option ("range")) {
        if (have_range) {
            range_error = true;
        } else {
            parms->range_string = parser->get_string ("range");
            have_range = true;
        }
    }
    if (have_range == false) {
	throw (dlib::error ("Error.  Please specify a range with the "
                "--above, --below, or --range option"));
    }
    if (range_error == true) {
	throw (dlib::error ("Error.  Only one range option (--above, "
                "--below, or --range) may be specified"));
    }

    /* Copy input filenames to parms struct */
    parms->img_in_fn = parser->get_string("input").c_str();

    /* Output files */
    parms->img_out_fn = parser->get_string("output").c_str();
}

void
do_command_threshold (int argc, char *argv[])
{
    Pcmd_threshold parms;
    
    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    threshold_main (&parms);
}
