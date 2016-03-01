/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include "itk_image_stats.h"
#include "logfile.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_math.h"
#include "pcmd_filter.h"
#include "print_and_exit.h"
#include "rt_study.h"
#include "synthetic_mha.h"
#include "volume_conv.h"
#include "volume_gaussian.h"

static Plm_image::Pointer
create_gabor_kernel (const Filter_parms *parms, const Plm_image::Pointer& img)
{
    Rt_study rt_study;
    Synthetic_mha_parms smp;
    Plm_image_header pih (img);
    int ker_width[3], ker_half_width[3];

    for (int i = 0; i < 3; i++) {
        ker_half_width[i] = 2 * parms->gauss_width / pih.spacing(i);
        ker_width[i] = 2 * ker_half_width[i] + 1;
        smp.dim[i] = ker_width[i];
        smp.origin[i] = - pih.spacing(i) * ker_half_width[i];
        smp.spacing[i] = pih.spacing(i);
        smp.gauss_center[i] = 0.f;
        smp.gauss_std[i] = parms->gauss_width;
    }
    smp.gabor_use_k_fib = parms->gabor_use_k_fib;
    smp.gabor_k_fib[0] = parms->gabor_k_fib[0];
    smp.gabor_k_fib[1] = parms->gabor_k_fib[1];
    smp.pattern = PATTERN_GABOR;
    smp.background = 0;
    smp.foreground = 1;
    smp.image_normalization = Synthetic_mha_parms::NORMALIZATION_GABOR;
    smp.m_want_ss_img = false;
    smp.m_want_dose_img = false;

    synthetic_mha (&rt_study, &smp);
    return rt_study.get_image();
}

static Plm_image::Pointer
create_gauss_kernel (const Filter_parms *parms, const Plm_image::Pointer& img)
{
    Rt_study rt_study;
    Synthetic_mha_parms smp;
    Plm_image_header pih (img);
    int ker_width[3], ker_half_width[3];

    for (int i = 0; i < 3; i++) {
        ker_half_width[i] = 2 * parms->gauss_width / pih.spacing(i);
        ker_width[i] = 2 * ker_half_width[i] + 1;
        smp.dim[i] = ker_width[i];
        smp.origin[i] = - pih.spacing(i) * ker_half_width[i];
        smp.spacing[i] = pih.spacing(i);
        smp.gauss_center[i] = 0.f;
        smp.gauss_std[i] = parms->gauss_width;
    }
    smp.pattern = PATTERN_GAUSS;
    smp.background = 0;
    smp.foreground = 1;
    smp.image_normalization = Synthetic_mha_parms::NORMALIZATION_SUM_ONE;
    smp.m_want_ss_img = false;
    smp.m_want_dose_img = false;

    synthetic_mha (&rt_study, &smp);
    return rt_study.get_image();
}

static void
filter_main (Filter_parms* parms)
{
    Plm_image::Pointer img = Plm_image::New (parms->in_image_fn);
    if (!img) {
        print_and_exit ("Sorry, couldn't load input image\n");
    }

    Plm_image::Pointer ker;
    Volume::Pointer volume_out;

    if (parms->filter_type == Filter_parms::FILTER_TYPE_GABOR)
    {
        ker = create_gabor_kernel (parms, img);
    }
    else if (parms->filter_type == Filter_parms::FILTER_TYPE_GAUSSIAN_COMBINED)
    {
        ker = create_gauss_kernel (parms, img);
    }
    else if (parms->filter_type == Filter_parms::FILTER_TYPE_GAUSSIAN_SEPARABLE)
    {
        volume_out = volume_gaussian (
            img->get_volume_float(), 
            parms->gauss_width, 
            2.0);
    }
    else if (parms->filter_type == Filter_parms::FILTER_TYPE_KERNEL)
    {
        /* Not yet implemented */
    }

    /* For kernel-style filters, do the actual filtering here */
    if (ker) {
        lprintf ("kernel size: %d %d %d\n",
            ker->dim(0), ker->dim(1), ker->dim(2));
        if (parms->out_kernel_fn != "") {
            ker->save_image (parms->out_kernel_fn);
        }
        volume_out = volume_conv (
            img->get_volume_float(), ker->get_volume_float());
    }

    Plm_image::Pointer img_out = Plm_image::New (volume_out);
    
    double min_val, max_val, avg;
    int non_zero, num_vox;
    itk_image_stats (img_out->itk_float(), &min_val, &max_val, 
        &avg, &non_zero, &num_vox);

    lprintf ("Filter result: MIN %g AVG %g MAX %g NONZERO: (%d / %d)\n",
        min_val, avg, max_val, non_zero, num_vox);

    if (parms->out_image_fn == "") {
        lprintf ("Warning: No output file specified.\n");
    } else {
        img_out->save_image (parms->out_image_fn);
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] input_image\n", argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Filter_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", "output image filename", 1, "");
    parser->add_long_option ("", "output-kernel", 
        "output kernel filename", 1, "");

    /* Main pattern */
    parser->add_long_option ("", "pattern",
        "filter type: {"
        "gabor, gauss, kernel"
        "}, default is gauss", 
        1, "gauss");

    /* Filter options */
    parser->add_long_option ("", "kernel", "kernel image filename", 1, "");
    parser->add_long_option ("", "gauss-width",
        "the width (in mm) of a uniform Gaussian smoothing filter", 1, "");
    parser->add_long_option ("", "gabor-k-fib", 
        "choose gabor direction at index i within fibonacci spiral "
        "of length n; specified as \"i n\" where i and n are integers, "
        "and i is between 0 and n-1", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that two input files were given */
    if (parser->number_of_arguments() != 1) {
	throw (dlib::error ("Error.  You must specify one input file"));
    }

    /* Input files */
    parms->in_image_fn = (*parser)[0].c_str();

    /* Output files */
    if (parser->option ("output")) {
        parms->out_image_fn = parser->get_string("output");
    }
    if (parser->option ("output-kernel")) {
        parms->out_kernel_fn = parser->get_string("output-kernel");
    }

    /* Main pattern */
    std::string arg = parser->get_string ("pattern");
    if (arg == "gabor") {
        parms->filter_type = Filter_parms::FILTER_TYPE_GABOR;
    }
    else if (arg == "gauss-slow") {
        parms->filter_type = Filter_parms::FILTER_TYPE_GAUSSIAN_COMBINED;
    }
    else if (arg == "gauss") {
        parms->filter_type = Filter_parms::FILTER_TYPE_GAUSSIAN_SEPARABLE;
    }
    else if (arg == "kernel") {
        parms->filter_type = Filter_parms::FILTER_TYPE_KERNEL;
    }
    else {
        throw (dlib::error ("Error. Unknown --pattern argument: " + arg));
    }

    /* Filter options */
    if (parser->option ("kernel")) {
        parms->out_image_fn = parser->get_string("kernel");
    }
    if (parser->option ("gauss-width")) {
        parms->gauss_width = parser->get_float("gauss-width");
    }
    if (parser->option ("gabor-k-fib")) {
        parms->gabor_use_k_fib = true;
        parser->assign_int_2 (parms->gabor_k_fib, "gabor-k-fib");
    }
}

void
do_command_filter (int argc, char *argv[])
{
    Filter_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    filter_main (&parms);
}
