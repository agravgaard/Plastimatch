/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "itk_image_load.h"
#include "itk_image_save.h"
#include "plm_clp.h"
#include "plm_math.h"
#include "synthetic_vf.h"
#include "volume_header.h"

typedef struct synthetic_vf_main_parms Synthetic_vf_main_parms;
struct synthetic_vf_main_parms {
    std::string output_fn;
    std::string fixed_fn;
    Synthetic_vf_parms sv_parms;
};

static void
deduce_geometry (
    Plm_image_header *pih,           /* Output */
    Synthetic_vf_main_parms *parms   /* Input */
)
{
    /* Try to guess the proper dimensions and spacing for output image */
    if (parms->fixed_fn != "") {
	/* use the spacing of user-supplied fixed image */
	printf ("Setting PIH from FIXED\n");
	FloatImageType::Pointer fixed = itk_image_load_float (
	    parms->fixed_fn, 0);
	pih->set_from_itk_image (fixed);
    } else {
	/* use user-supplied or default values, which are already set */
    }
}

void
do_synthetic_vf (Synthetic_vf_main_parms *parms)
{
    Synthetic_vf_parms *sv_parms = &parms->sv_parms;

    /* Deduce output geometry */
    deduce_geometry (&sv_parms->pih, parms);

    /* Create vf */
    DeformationFieldType::Pointer vf = synthetic_vf (sv_parms);

    /* Save to file */
    itk_image_save (vf, parms->output_fn);
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch synth-vf [options]\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Synthetic_vf_main_parms *parms, 
    dlib::Plm_clp *parser, 
    int argc, 
    char* argv[]
)
{
    Volume_header vh;
    plm_long *dim = vh.get_dim();

    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", 
	"output filename", 1, "");

    /* Image size */
    parser->add_long_option ("", "origin", 
	"location of first image voxel in mm \"x y z\"", 1, "0 0 0");
    parser->add_long_option ("", "dim", 
	"size of output image in voxels \"x [y z]\"", 1, "100");
    parser->add_long_option ("", "spacing", 
	"voxel spacing in mm \"x [y z]\"", 1, "5");
    parser->add_long_option ("", "direction-cosines", 
	"oriention of x, y, and z axes; Specify either preset value,"
	" {identity, rotated-{1,2,3}, sheared},"
	" or 9 digit matrix string \"a b c d e f g h i\"", 1, "");
    parser->add_long_option ("", "volume-size", 
	"size of output image in mm \"x [y z]\"", 1, "500");
    parser->add_long_option ("", "fixed", 
	"An input image used to set the size of the output ", 1, "");

    /* Patterns */
    parser->add_long_option ("", "xf-gauss",
	"gaussian warp");
    parser->add_long_option ("", "xf-radial",
	"radial expansion (or contraction)");
    parser->add_long_option ("", "xf-trans",
	"uniform translation in mm \"x y z\"", 1);
    parser->add_long_option ("", "xf-zero", "Null transform");

    /* Pattern options */
    parser->add_long_option ("", "gauss-center", 
	"location of center of gaussian warp \"x [y z]\"", 1, "0 0 0");
    parser->add_long_option ("", "gauss-mag", 
	"displacment magnitude for gaussian warp in mm \"x [y z]\"", 1, "10");
    parser->add_long_option ("", "gauss-std", 
	"width of gaussian std in mm \"x [y z]\"", 1, "10");
    parser->add_long_option ("", "radial-center", 
	"location of center of radial warp \"x [y z]\"", 1, "0 0 0");
    parser->add_long_option ("", "radial-mag", 
	"displacement magnitude for radial warp in mm \"x [y z]\"", 1, "10");

    /* Parse the command line arguments */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an output file was given */
    if (!parser->option("output")) {
	throw dlib::error (
	    "Error, you must specify an --output option.\n"
	);
    }

    /* Check that a xf option was given */
    if (!parser->option("xf-gauss") && 
        !parser->option("xf-radial") && 
	!parser->option("xf-trans") && 
	!parser->option("xf-zero"))
    {
	std::cout << 
	    "Error, you must specify one of the --xf-*** options.\n";
	usage_fn (parser, argc, argv);
	exit (1);
    }

    /* Copy values into output struct */
    Synthetic_vf_parms *sv_parms = &parms->sv_parms;

    /* Basic options */
    parms->output_fn = parser->get_string("output");

    /* Patterns */
    if (parser->option("xf-zero")) {
	sv_parms->pattern = Synthetic_vf_parms::PATTERN_ZERO;
    } else if (parser->option("xf-trans")) {
	sv_parms->pattern = Synthetic_vf_parms::PATTERN_TRANSLATION;
	parser->assign_float_13 (sv_parms->translation, "xf-trans");
    } else if (parser->option("xf-radial")) {
	sv_parms->pattern = Synthetic_vf_parms::PATTERN_RADIAL;
    } else if (parser->option("xf-gauss")) {
	sv_parms->pattern = Synthetic_vf_parms::PATTERN_GAUSSIAN;
    } else {
	throw (dlib::error ("Error. Unknown --xf argument."));
    }

    /* Fixed */
    if (parser->option ("fixed")) {
        parms->fixed_fn = parser->get_string("fixed");
    }

    /* Image size */
    parser->assign_plm_long_13 (dim, "dim");

    /* Direction cosines */
    if (parser->option ("direction-cosines")) {
	std::string arg = parser->get_string("direction-cosines");
	if (!vh.get_direction_cosines().set_from_string (arg)) {
	    throw (dlib::error ("Error parsing --direction-cosines "
		    "(should have nine numbers)\n"));
	}
    }

    /* If origin not specified, volume is centered about size */
    float volume_size[3];
    parser->assign_float_13 (volume_size, "volume-size");
    if (parser->option ("origin")) {
	parser->assign_float_13 (vh.get_origin(), "origin");
    } else {
	for (int d = 0; d < 3; d++) {
	    /* GCS FIX: This should include direction cosines */
	    vh.get_origin()[d] = - 0.5 * volume_size[d] 
		+ 0.5 * volume_size[d] / dim[d];
	}
    }

    /* If spacing not specified, set spacing from size and resolution */
    if (parser->option ("spacing")) {
	parser->assign_float_13 (vh.get_spacing(), "spacing");
    } else {
	for (int d = 0; d < 3; d++) {
	    vh.get_spacing()[d] 
		= volume_size[d] / ((float) dim[d]);
	}
    }

    /* Set the pih */
    sv_parms->pih.set (vh);

    /* Radial options */
    parser->assign_float_13 (sv_parms->radial_center, "radial-center");
    parser->assign_float_13 (sv_parms->radial_mag, "radial-mag");

    /* Gaussian options */
    parser->assign_float_13 (sv_parms->gaussian_center, "gauss-center");
    parser->assign_float_13 (sv_parms->gaussian_mag, "gauss-mag");
    parser->assign_float_13 (sv_parms->gaussian_std, "gauss-std");
}

void
do_command_synth_vf (int argc, char* argv[])
{
    Synthetic_vf_main_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    do_synthetic_vf (&parms);
}
