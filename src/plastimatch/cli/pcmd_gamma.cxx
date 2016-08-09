/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "gamma_dose_comparison.h"
#include "logfile.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "plm_math.h"
#include "pcmd_gamma.h"
#include "print_and_exit.h"

#include "plm_file_format.h"
#include "rt_study.h"


static void
gamma_main (Gamma_parms* parms)
{
    Gamma_dose_comparison gdc;

    //DICOM_RD compatible (added by YK, Feb 2015)
    //In the prev version, RD couldn't be read directly due to the scale factor inside of the DICOM file.
    //work-around was to use (plastimatch convert ...)
    //here, that feature has been integrated into plastimatch gamma
    Plm_file_format file_type_ref, file_type_comp;
    Rt_study rt_study_ref, rt_study_comp;

    file_type_ref = plm_file_format_deduce(parms->ref_image_fn.c_str());
    file_type_comp = plm_file_format_deduce(parms->cmp_image_fn.c_str());

    if (file_type_ref == PLM_FILE_FMT_DICOM_DOSE) {
        rt_study_ref.load(parms->ref_image_fn.c_str(), file_type_ref);
        if (rt_study_ref.has_dose()) {
            gdc.set_reference_image (rt_study_ref.get_dose());
        }
        else {
            gdc.set_reference_image (parms->ref_image_fn.c_str());
        }
    }
    else {
        gdc.set_reference_image (parms->ref_image_fn.c_str());
    }

    if (file_type_comp == PLM_FILE_FMT_DICOM_DOSE) {
        rt_study_comp.load(parms->cmp_image_fn.c_str(), file_type_comp);
        if (rt_study_comp.has_dose()) {
            gdc.set_compare_image (rt_study_comp.get_dose());
        }
        else {
            gdc.set_compare_image (parms->cmp_image_fn.c_str());
        }

    }
    else {
        gdc.set_compare_image (parms->cmp_image_fn.c_str());
    }
    //End DICOM-RD    

    if (parms->mask_image_fn != "") {
        gdc.set_mask_image (parms->mask_image_fn);
    }

    gdc.set_spatial_tolerance (parms->dta_tolerance);
    gdc.set_dose_difference_tolerance (parms->dose_tolerance);
    if (parms->have_reference_dose) {
        gdc.set_reference_dose (parms->reference_dose);
    }
    gdc.set_gamma_max (parms->gamma_max);

    /*Extended by YK*/
    gdc.set_interp_search(parms->b_interp_search);//default: false
    gdc.set_local_gamma(parms->b_local_gamma);//default: false
    gdc.set_compute_full_region(parms->b_compute_full_region);//default: false
    gdc.set_resample_nn(parms->b_resample_nn); //default: false
    //gdc.set_include_high_dose_in_comp(parms->b_include_high_dose_comp); //default: false
    gdc.set_ref_only_threshold(parms->b_ref_only_threshold); //default: false

    if (parms->f_inherent_resample_mm > 0.0) {
        gdc.set_inherent_resample_mm(parms->f_inherent_resample_mm);
    }

    if (parms->f_analysis_threshold > 0) {
        gdc.set_analysis_threshold(parms->f_analysis_threshold);//0.1 = 10%
    }

    gdc.run ();

    if (parms->out_image_fn != "") {
        Plm_image::Pointer gamma_image = gdc.get_gamma_image ();
        gamma_image->save_image (parms->out_image_fn);
    }

    if (parms->out_failmap_fn != "") {		
        gdc.get_fail_image()->save_image(parms->out_failmap_fn);
    }

    if (parms->out_report_fn != "") {
        //Export output text using gdc.get_report_string();
        std::ofstream fout;
        fout.open(parms->out_report_fn.c_str());
        if (!fout.fail()){
            fout << "Reference_file_name\t" << parms->ref_image_fn.c_str() << std::endl;
            fout << "Compare_file_name\t" << parms->cmp_image_fn.c_str() << std::endl;

            fout << gdc.get_report_string();
            fout.close();
        }		
    }

    lprintf ("Pass rate = %2.6f %%\n", gdc.get_pass_fraction() * 100.0);
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] image_1 image_2\n", argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Gamma_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", "Output image", 1, "");

    /* Input files */
    parser->add_long_option ("", "mask", 
        "File name for input mask image, which controls which voxels "
        "are used in the gamma calculation", 1, "");

    /* Gamma options */
    parser->add_long_option ("", "dose-tolerance", 
        "The scaling coefficient for dose difference. (e.g. put 0.02 if you want to apply 2% dose difference criterion) "
        "(default is 0.03)", 1, "0.03");
    parser->add_long_option ("", "dta-tolerance", 
        "The distance-to-agreement (DTA) scaling coefficient in mm "
        "(default is 3)", 1, "3");
    parser->add_long_option ("", "reference-dose", 
        "The prescription dose (Gy) used to compute dose tolerance; if not "
        "specified, then maximum dose in reference volume is used",
        1, "");
    parser->add_long_option ("", "gamma-max", 
        "The maximum value of gamma to compute; smaller values run faster "
        "(default is 2.0)", 1, "2.0");

    /* extended by YK*/
    parser->add_long_option("", "ref-only-threshold", "With this option, analysis threshold is applied only to ref dose, regardless of the corresponding dose of compare image. ", 0);
    parser->add_long_option("", "interp-search", "With this option, smart interpolation search will be used in points near the reference point. This will eliminate the needs of fine resampling. However, it will take longer time to compute. ", 0);
    parser->add_long_option("", "local-gamma", "With this option, dose difference is calculated based on local dose difference. Otherwise, a given reference dose will be used, which is called global-gamma. ",0);
    parser->add_long_option("", "compute-full-region", "With this option, full gamma map will be generated over the entire image region (even for low-dose region). It is recommended not to use this option to speed up the computation. It has no effect on gamma pass-rate. ", 0);
    parser->add_long_option("", "resample-nn", "With this option, Nearest Neighbor will be used instead of linear interpolation in resampling the compare-image to the reference image. Not recommended for better results. ", 0);
    parser->add_long_option("", "inherent-resample", "Spacing value in [mm]. The reference image itself will be resampled by this value (Note: resampling compare-image to ref-image is inherent already). If arg < 0, this option is disabled.  "
        "(default is -1.0)", 1, "-1.0");

    parser->add_long_option("", "analysis-threshold",
        "Analysis threshold for dose in float (for example, input 0.1 to apply 10% of the reference dose). The final threshold dose (Gy) is calculated by multiplying this value and a given reference dose (or maximum dose if not given). "
        "(default is 0.1)", 1, "0.1");	

    parser->add_long_option("", "output-text", "Text file path for gamma evaluation result. ", 1, "");
    parser->add_long_option("", "output-failmap", "File path for binary gamma evaluation result. ", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that two input files were given */
    if (parser->number_of_arguments() < 2) {
	throw (dlib::error ("Error.  You must specify two input files"));
	
    } else if (parser->number_of_arguments() > 2) {
	std::string extra_arg = (*parser)[1];
	throw (dlib::error ("Error.  Extra argument " + extra_arg));
    }

    /* Input files */
    parms->ref_image_fn = (*parser)[0].c_str();
    parms->cmp_image_fn = (*parser)[1].c_str();
    if (parser->option ("mask")) {
        parms->mask_image_fn = parser->get_string("mask");
    }

    /* Output files */
    if (parser->option ("output")) {
        parms->out_image_fn = parser->get_string("output");
    }
    if (parser->option("output-failmap")) {
        parms->out_failmap_fn = parser->get_string("output-failmap");
    }

    /* Gamma options */
    parms->dose_tolerance = parser->get_float("dose-tolerance");
    parms->dta_tolerance = parser->get_float("dta-tolerance");
    parms->gamma_max = parser->get_float("gamma-max");
    if (parser->option("reference-dose")) {
        parms->have_reference_dose = true;
        parms->reference_dose = parser->get_float("reference-dose");
    }

    if (parser->option("interp-search")) {
        parms->b_interp_search = true;
    }
    if (parser->option("ref-only-threshold")) {
        parms->b_ref_only_threshold = true;
    }
	
    if (parser->option("local-gamma")) {
        parms->b_local_gamma = true;
    }

    if (parser->option("compute-full-region")) {		
        parms->b_compute_full_region = true;
    }

    if (parser->option("resample-nn")) {
        parms->b_resample_nn = true;
    }		

    if (parser->option("inherent-resample")) {
        parms->f_inherent_resample_mm = parser->get_float("inherent-resample");
    }

    if (parser->option("analysis-threshold")) {
        parms->f_analysis_threshold = parser->get_float("analysis-threshold");
    }

    /* Output file for text report */
    if (parser->option("output-text")) {
        parms->out_report_fn = parser->get_string("output-text");
    }
}

void
do_command_gamma (int argc, char *argv[])
{
    Gamma_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    gamma_main (&parms);
}
