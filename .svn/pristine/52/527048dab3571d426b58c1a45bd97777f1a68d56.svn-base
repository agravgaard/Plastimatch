/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "file_util.h"
#include "itk_adjust.h"
#include "path_util.h"
#include "pcmd_thumbnail.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "thumbnail.h"

class Thumbnail_parms {
public:
    std::string input_fn;
    std::string output_fn;
    int axis;
    int dim;
    float spacing;
    float loc;
    bool auto_adjust;
public:
    Thumbnail_parms () {
	output_fn = "thumb.mhd";
	dim = 16;
	spacing = 30.0;
        axis = 2;
	loc = 0.0;
        auto_adjust = false;
    }
};

static void
thumbnail_main (Thumbnail_parms* parms)
{
    Plm_image::Pointer pli;

    /* Load image */
    pli = plm_image_load (parms->input_fn, PLM_IMG_TYPE_ITK_FLOAT);

    /* Make thumbnail */
    Thumbnail thumbnail;
    thumbnail.set_input_image (pli);
    thumbnail.set_thumbnail_dim (parms->dim);
    thumbnail.set_thumbnail_spacing (parms->spacing);
    thumbnail.set_axis (parms->axis);
    thumbnail.set_slice_loc (parms->loc);
    pli->m_itk_float = thumbnail.make_thumbnail ();

    /* Adjust the intensities */
    if (parms->auto_adjust) {
        printf ("Auto-adjusting intensities...\n");
        pli->set_itk (itk_auto_adjust (pli->m_itk_float));
    }

    /* Can't write float for these types... */
    if (extension_is (parms->output_fn, "png")
        || extension_is (parms->output_fn, "tif") 
        || extension_is (parms->output_fn, "tiff"))
    {
        pli->convert (PLM_IMG_TYPE_ITK_UCHAR);
    }

    /* Save the output file */
    pli->save_image (parms->output_fn);
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
    Thumbnail_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Input files */
    parser->add_long_option ("", "input", 
        "input directory or filename of image", 1, "");

    /* Output files */
    parser->add_long_option ("", "output", 
        "output image; can be mha, mhd, nii, nrrd, or other format "
        "supported by ITK", 1, "");

    /* Geometry options */
    parser->add_long_option ("", "dim", 
        "size of output image in voxels", 1, "16");
    parser->add_long_option ("", "spacing", 
        "voxel spacing in mm", 1, "30");
    parser->add_long_option ("", "axis", 
        "either \"x\", \"y\", or \"z\"", 1, "z");
    parser->add_long_option ("", "loc", 
        "location of thumbnail along axis", 1, "0");
    parser->add_long_option ("", "auto-adjust", 
        "adjust the intensities", 0);

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an input file was given */
    if (!parser->option ("input")) {
        throw (dlib::error ("Error.  Please specify an input file "));
    }

    /* Check that an output file was given */
    if (!parser->option ("output")) {
        throw (dlib::error ("Error.  Please specify an output file "));
    }

    /* Check that no extraneous options were given */
    if (parser->number_of_arguments() != 0) {
        std::string extra_arg = (*parser)[0];
        throw (dlib::error ("Error.  Unknown option " + extra_arg));
    }

    /* Input/output files */
    parms->input_fn = parser->get_string("input").c_str();
    parms->output_fn = parser->get_string("output").c_str();

    /* Other options */
    std::string arg = parser->get_string ("axis");
    if (arg == "z") {
        parms->axis = 2;
    }
    else if (arg == "y") {
        parms->axis = 1;
    }
    else if (arg == "x") {
        parms->axis = 0;
    }
    else {
        throw (dlib::error ("Error. Unknown --axis argument: " + arg));
    }

    parms->loc = parser->get_int ("loc");
    parms->spacing = parser->get_float("spacing");
    parms->dim = parser->get_int ("dim");
    if (parser->option ("auto-adjust")) {
        parms->auto_adjust = true;
    }
}

void
do_command_thumbnail (int argc, char *argv[])
{
    Thumbnail_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    /* Run the thumbnailer */
    thumbnail_main (&parms);
}
