/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "gdcm1_dose.h"
#include "itk_image_load.h"
#include "itk_image_stats.h"
#include "logfile.h"
#include "mha_io.h"
#include "pcmd_stats.h"
#include "plm_clp.h"
#include "plm_file_format.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "pointset.h"
#include "print_and_exit.h"
#include "proj_image.h"
#include "rt_study.h"
#include "ss_img_stats.h"
#include "vf_stats.h"
#include "volume.h"
#include "xform.h"

class Stats_parms {
public:
    bool structure;
    std::string mask_fn;
    std::list<std::string> input_fns;
};


static void
stats_vf_main (Stats_parms* parms, const std::string& current_fn)
{
    Volume *vol = 0;
    Xform xf1, xf2;

    xf1.load (current_fn);

    if (xf1.m_type == XFORM_GPUIT_VECTOR_FIELD) {
	vol = xf1.get_gpuit_vf().get();
    }
    else if (xf1.m_type == XFORM_ITK_VECTOR_FIELD) {
	/* GCS FIX: This logic should be moved inside of xform class */
	Plm_image_header pih;
	pih.set_from_itk_image (xf1.get_itk_vf ());
	xform_to_gpuit_vf (&xf2, &xf1, &pih);
	vol = xf2.get_gpuit_vf().get();
    }
    else 
    {
	print_and_exit ("Error: input file %s is not a vector field\n", 
            current_fn.c_str());
    }

    if (vol->pix_type != PT_VF_FLOAT_INTERLEAVED) {
	fprintf (stderr, 
	    "Sorry, file \"%s\" is not an interleaved float vector field.\n", 
            current_fn.c_str());
	fprintf (stderr, "Type = %d\n", vol->pix_type);
	delete vol;
	exit (-1);
    }

    if (parms->mask_fn.length() == 0) {
    	vf_analyze (vol, 0);
	vf_analyze_jacobian (vol, 0);
    	vf_analyze_strain (vol, 0);
	vf_analyze_second_deriv (vol);
    }
    else {
        Plm_image::Pointer pli = Plm_image::New (new Plm_image(
                parms->mask_fn));
        pli->convert (PLM_IMG_TYPE_GPUIT_UCHAR);
        Volume* mask = pli->get_vol();
	vf_analyze (vol, mask);
	vf_analyze_jacobian (vol, mask);
	vf_analyze_strain (vol, mask);
	vf_analyze_second_deriv (vol);
    }
}

static void
stats_pointset_main (Stats_parms* parms, const std::string& current_fn)
{
    Labeled_pointset ps;
    ps.load (current_fn.c_str());
    lprintf ("Pointset has %d points\n", ps.get_count());
}

static void
stats_proj_image_main (Stats_parms* parms, const std::string& current_fn)
{
    Proj_image *proj;

    proj = new Proj_image (current_fn, "");
    proj->debug_header ();
    proj->stats ();
    delete proj;
}

static void
stats_ss_image_main (Stats_parms* parms, const std::string& current_fn)
{
    Plm_image plm (current_fn);

    if (plm.m_type != PLM_IMG_TYPE_ITK_UCHAR_VEC) {
	print_and_exit ("Failure loading file %s as ss_image.\n",
	    current_fn.c_str());
    }

    UCharVecImageType::Pointer img = plm.m_itk_uchar_vec;

    ss_img_stats (img);
}

static void
stats_structure_main (Stats_parms* parms, const std::string& current_fn)
{
    Rt_study rt_study;
    rt_study.load (current_fn);
}

static void
stats_img_main (Stats_parms* parms, const std::string& current_fn)
{
    Plm_image pli (current_fn);
    FloatImageType::Pointer img = pli.itk_float();

    double min_val, max_val, avg;
    int non_zero, num_vox;
    itk_image_stats (img, &min_val, &max_val, &avg, &non_zero, &num_vox);

    printf ("MIN %f AVE %f MAX %f NONZERO %d NUMVOX %d\n", 
	(float) min_val, (float) avg, (float) max_val, non_zero, num_vox);
}

static void
stats_dicom_dose (Stats_parms* parms, const std::string& current_fn)
{
#if PLM_DCM_USE_DCMTK
    /* Sorry, not yet supported */
#elif PLM_DCM_USE_GDCM1
    Plm_image *dose = gdcm1_dose_load (0, current_fn.c_str());
    FloatImageType::Pointer img = dose->itk_float ();
    double min_val, max_val, avg;
    int non_zero, num_vox;
    itk_image_stats (img, &min_val, &max_val, &avg, &non_zero, &num_vox);

    printf ("MIN %f AVE %f MAX %f NONZERO %d NUMVOX %d\n", 
	(float) min_val, (float) avg, (float) max_val, non_zero, num_vox);

    delete dose;
#endif
}

static void
stats_main (Stats_parms* parms)
{
    std::list<std::string>::iterator it = parms->input_fns.begin();
    while (it != parms->input_fns.end()) {
        std::string current_fn = *it;
        if (parms->structure) {
            stats_structure_main (parms, current_fn);
            ++it;
            continue;
        }
        Plm_file_format file_format = plm_file_format_deduce (current_fn);
        switch (file_format) {
        case PLM_FILE_FMT_IMG:
            stats_img_main (parms, current_fn);
            break;
        case PLM_FILE_FMT_VF:
            stats_vf_main (parms, current_fn);
            break;
        case PLM_FILE_FMT_POINTSET:
            stats_pointset_main (parms, current_fn);
            break;
        case PLM_FILE_FMT_PROJ_IMG:
            stats_proj_image_main (parms, current_fn);
            break;
        case PLM_FILE_FMT_DICOM_DOSE:
            stats_dicom_dose (parms, current_fn);
            break;
        case PLM_FILE_FMT_SS_IMG_VEC:
            stats_ss_image_main (parms, current_fn);
            break;
        default:
            printf ("Warning, stats requested for file type: %s\n",
                plm_file_format_string (file_format));
            stats_img_main (parms, current_fn);
            break;
        }
        ++it;
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf (
        "Usage: plastimatch stats [options] input_file [input_file ...]\n");
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Stats_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Weight vector */
    parser->add_long_option ("", "mask", 
        "A binary image (usually unsigned char) where only non-zero voxels "
        "are considered for statistics", 1, "");
    parser->add_long_option ("", "structure", 
        "Compute structure statistics rather than image statistics", 0);

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    if (parser->option ("mask")) {
        parms->mask_fn = parser->get_string ("mask");
    }
    if (parser->option ("structure")) {
        parms->structure = true;
    } else {
        parms->structure = false;
    }

    /* Check that no extraneous options were given */
    if (parser->number_of_arguments() == 0) {
	throw (dlib::error ("Error.  You must specify at least one "
                "file for printing stats."));
    }

    /* Copy input filenames to parms struct */
    for (unsigned long i = 0; i < parser->number_of_arguments(); i++) {
        parms->input_fns.push_back ((*parser)[i]);
    }
}

void
do_command_stats (int argc, char *argv[])
{
    Stats_parms parms;
    
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);
    stats_main (&parms);
}
