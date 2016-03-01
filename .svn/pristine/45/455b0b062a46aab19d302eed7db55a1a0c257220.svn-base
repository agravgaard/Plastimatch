/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <string>

#include "itk_crop.h"
#include "pcmd_crop.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "print_and_exit.h"

class Crop_parms {
public:
    std::string img_in_fn;
    std::string img_out_fn;
    int crop_vox[6];
public:
    Crop_parms () {
        img_in_fn = "";
        img_out_fn = "";
        for (int i = 0; i < 6; i++) {
            crop_vox[i] = 0;
        }
    }
};

static void
crop_main (Crop_parms* parms)
{
    Plm_image plm_image;

    plm_image.load_native (parms->img_in_fn);

    switch (plm_image.m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	plm_image.m_itk_uchar 
	    = itk_crop (plm_image.m_itk_uchar, parms->crop_vox);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	plm_image.m_itk_short 
		= itk_crop (plm_image.m_itk_short, parms->crop_vox);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	plm_image.m_itk_uint32 
		= itk_crop (plm_image.m_itk_uint32, parms->crop_vox);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	plm_image.m_itk_float 
		= itk_crop (plm_image.m_itk_float, parms->crop_vox);
	break;
    default:
	print_and_exit ("Unhandled image type in resample_main()\n");
	break;
    }

    plm_image.convert_and_save (parms->img_out_fn, plm_image.m_type);
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
    Crop_parms* parms, 
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
    parser->add_long_option ("", "output", 
        "output image", 1, "");

    /* Adjustment string */
    parser->add_long_option ("", "voxels", 
        "a string that specifies the voxels in the six corners "
        "of the region to be cropped, in the form "
        "\"x1 x2 y1 y2 z1 z2\"", 
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

    /* Check that an output file was given */
    if (!parser->option ("voxels")) {
	throw (dlib::error ("Error.  Please specify the voxels to be "
		"cropped using the --voxels option"));
    }

    /* Input files */
    parms->img_in_fn = parser->get_string("input").c_str();

    /* Output files */
    parms->img_out_fn = parser->get_string("output").c_str();

    /* Voxels option */
    parser->assign_int_6 (parms->crop_vox, "voxels");
}

void
do_command_crop (int argc, char *argv[])
{
    Crop_parms parms;
    
    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    crop_main (&parms);
}
