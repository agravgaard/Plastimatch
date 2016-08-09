/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bspline.h"
#include "bspline_mi.h"
#include "bspline_optimize.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "mha_io.h"
#include "plm_clp.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "registration_metric_type.h"
#include "volume.h"
#include "volume_grad.h"

enum Check_grad_process {
    CHECK_GRAD_PROCESS_FWD,
    CHECK_GRAD_PROCESS_BKD,
    CHECK_GRAD_PROCESS_CTR,
    CHECK_GRAD_PROCESS_LINE
};

class Check_grad_opts {
public:
    std::string fixed_fn;
    std::string fixed_roi_fn;
    std::string moving_fn;
    std::string input_xf_fn;
    std::string input_search_dir_fn;
    std::string output_fn;
    const char* xpm_hist_prefix;

    float factr;
    float pgtol;
    float step_size;
    int line_range[2];
    plm_long vox_per_rgn[3];
    Check_grad_process process;
    int random;
    float random_range[2];
    int control_point_index;
    std::string debug_dir;

    char bsp_implementation;
    BsplineThreading bsp_threading;
    Registration_metric_type bsp_metric;

public:
    Check_grad_opts () {
        xpm_hist_prefix = 0;
	factr = 0;
	pgtol = 0;
	step_size = 1e-4;
	line_range[0] = 0;
	line_range[1] = 30;
	for (int d = 0; d < 3; d++) {
	    vox_per_rgn[d] = 15;
	}
	process = CHECK_GRAD_PROCESS_FWD;
	random = 0;
	random_range[0] = 0;
	random_range[1] = 0;
        control_point_index = 514;  // -1;
        debug_dir = "";
        bsp_implementation = '\0';
        bsp_threading = BTHR_CPU;
        bsp_metric = REGISTRATION_METRIC_MSE;
    }
};

void
check_gradient (
    Check_grad_opts *options, 
    Volume *fixed, 
    Volume *moving, 
    Volume *moving_grad)
{
    int i, j;
    float *x, *grad, *grad_fd;
    float score;
    FILE *fp;
    plm_long roi_offset[3];
    Bspline_optimize bod;
    Bspline_xform *bxf;
    Bspline_parms *parms = new Bspline_parms;

    /* Fixate images into bspline parms */
    parms->fixed = fixed;
    parms->moving = moving;
    parms->moving_grad = moving_grad;
    parms->implementation = options->bsp_implementation;
    parms->metric_type[0] = options->bsp_metric;

    /* Maybe we got a roi too */
    Plm_image::Pointer pli_fixed_roi;
    if (options->fixed_roi_fn != "") {
        pli_fixed_roi = Plm_image::New (options->fixed_roi_fn);
        parms->fixed_roi = pli_fixed_roi->get_volume_uchar().get();
    }

    /* Set extra debug stuff, if desired */
    if (options->debug_dir != "") {
        parms->debug = 1;
        parms->debug_dir = options->debug_dir;
        parms->debug_stage = 0;
    }

    /* Allocate memory and build lookup tables */
    printf ("Allocating lookup tables\n");
    memset (roi_offset, 0, 3*sizeof(plm_long));
    if (!options->input_xf_fn.empty()) {
        bxf = bspline_xform_load (options->input_xf_fn.c_str());
    } else {
        bxf = new Bspline_xform;
        bxf->initialize (
            fixed->origin,
            fixed->spacing,
            fixed->dim,
            roi_offset,
            fixed->dim,
            options->vox_per_rgn,
            fixed->get_direction_matrix()
        );
        if (options->random) {
            srand (time (0));
            for (i = 0; i < bxf->num_coeff; i++) {
                bxf->coeff[i] = options->random_range[0]
                    + (options->random_range[1] - options->random_range[0])
                    * rand () / (double) RAND_MAX;
            }
        }
    }
    bod.initialize (bxf, parms);
    Bspline_state *bst = bod.get_bspline_state ();

    /* Create scratch variables */
    x = (float*) malloc (sizeof(float) * bxf->num_coeff);
    grad = (float*) malloc (sizeof(float) * bxf->num_coeff);
    grad_fd = (float*) malloc (sizeof(float) * bxf->num_coeff);

    /* Save a copy of x */
    for (i = 0; i < bxf->num_coeff; i++) {
        x[i] = bxf->coeff[i];
    }

    if (parms->metric_type[0] == REGISTRATION_METRIC_MI_MATTES) {
        bst->mi_hist->initialize (parms->fixed, parms->moving);
    }

    /* Get score and gradient */
    bspline_score (&bod);
    if (parms->debug) {
        bspline_save_debug_state (parms, bst, bxf);
    }

    /* Save a copy of score and gradient */
    for (i = 0; i < bxf->num_coeff; i++) {
        grad[i] = bst->ssd.total_grad[i];
    }
    score = bst->ssd.score;

    /* If a search dir was specified, use that instead of the gradient */
    if (options->input_search_dir_fn != "") {
        std::ifstream ifs (options->input_search_dir_fn.c_str());
        if (!ifs) {
            print_and_exit ("Failure opening %s for read\n",
                options->input_search_dir_fn.c_str());
        }
        for (i = 0; i < bxf->num_coeff; i++) {
            ifs >> grad[i];
            if (!ifs) {
                print_and_exit ("Incomplete search_dir in %s\n", 
                    options->input_search_dir_fn.c_str());
            }
        }
    }

    if (options->output_fn.empty()) {
        fp = stdout;
    } else {
        fp = fopen (options->output_fn.c_str(), "w");
    }
    if (options->process == CHECK_GRAD_PROCESS_LINE) {
        /* For each step along line */
        for (i = options->line_range[0]; i <= options->line_range[1]; i++) {
            bst->it = i;

            /* Already computed for i = 0 */
            if (i == 0) {
                fprintf (fp, "%4d,%12.12f\n", i, score);
                continue;
            }

            /* Create new location for x */
            for (j = 0; j < bxf->num_coeff; j++) {
                bxf->coeff[j] = x[j] - i * options->step_size * grad[j];
            }

            /* Get score */
            bspline_score (&bod);
            if (parms->debug) {
                bspline_save_debug_state (parms, bst, bxf);
            }
        
            /* Compute difference between grad and grad_fd */
            fprintf (fp, "%4d,%12.12f\n", i, bst->ssd.score);

            // JAS 04.19.2010
            // This loop could take a while to exit.  This will
            // flush the buffer so that we will at least get the data
            // that we worked for if we get sick of waiting and opt
            // for early program termination.
            fflush(fp);
        }
    } else {
        /* Loop through directions */
        for (i = 0; i < bxf->num_coeff; i++) {
            bst->it = i;

            if (options->control_point_index >= 0) {
                if (i != options->control_point_index) {
                    continue;
                }
            }

            /* Take a step in this direction */
            for (j = 0; j < bxf->num_coeff; j++) {
                bxf->coeff[j] = x[j];
            }
            bxf->coeff[i] = bxf->coeff[i] + options->step_size;

            /* Get score */
            bspline_score (&bod);
            if (parms->debug) {
                bspline_save_debug_state (parms, bst, bxf);
            }
        
            /* Stash score difference in grad_fd */
            grad_fd[i] = (bst->ssd.score - score) / options->step_size;

            /* Compute difference between grad and grad_fd */
            fprintf (fp, "%12.12f,%12.12f\n", grad[i], grad_fd[i]);
        }
    }

    if (!options->output_fn.empty()) {
        fclose (fp);
    }
    free (x);
    free (grad);
    free (grad_fd);
    delete parms;
    delete bxf;
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: check_grad [options] fixed-image moving-image\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

/*
-A hardware             Either "cpu" or "cuda" (default=cpu)
-M { mse | mi }         Registration metric (default is mse)
    -f implementation       Choose implementation (a single letter: a, b, etc.)
    -s "i j k"              Integer knot spacing (voxels)
 -h prefix               Generate histograms for each MI iteration
 --debug                 Create various debug files
 -p process              Choices: "fwd", "bkd", "ctr" (for forward,
     backward, or central difference, or "line" for
     line profile. (default=fwd)
     -e step                 Step size (default is 1e-4)
     -l "min max"            Min, max range for line profile (default "0 30")
     -R "min max"          Random starting point (coeff between min, max)
 -X infile               Input bspline coefficients
 -O file                 Output file
*/
static void
parse_fn (
    Check_grad_opts *parms,
    dlib::Plm_clp *parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Algorithm options */
    parser->add_long_option ("A", "hardware",
	"algorithm hardware, either \"cpu\" or \"cuda\", "
        "default is cpu", 1, "cpu");
    parser->add_long_option ("f", "flavor", 
	"algroithm flavor, a single letter such as 'c' or 'f'",
        1, "");
    parser->add_long_option ("M", "metric",
	"registration metric, either \"mse\" or \"mi\", "
        "default is mse", 1, "mse");
    parser->add_long_option ("p", "process", 
	"analysis process, either \"fwd\", \"bkd\", or \"ctr\" "
        "for forward, backward, or central differencing, "
        "or \"line\" for line profile; default is \"fwd\"", 1, "fwd");
    parser->add_long_option ("e", "step",
        "step size; default is 1e-4", 1, "1e-4");
    parser->add_long_option ("l", "line-range", 
        "range of line profile as number of steps between \"min max\"; "
        "default is \"0 30\"", 1, "0 30");
    parser->add_long_option ("R", "random-start", 
        "use random coefficient values in range between \"min max\"", 1, "");

    /* Input/output files */
    parser->add_long_option ("X", "input-xform",
	"input bspline transform", 1, "");
    parser->add_long_option ("O", "output",
	"output file", 1, "");
    parser->add_long_option ("", "debug-dir", 
        "create various debug files", 1, "");
    parser->add_long_option ("H", "histogram-prefix", 
        "create MI histograms files with the specified prefix", 1, "");
    parser->add_long_option ("", "input-search-dir",
        "input file containing search direction", 1, "");
    parser->add_long_option ("", "fixed-roi", 
        "input file for fixed image roi", 1, "");
        
    /* Parse the command line arguments */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that two, and only two, input files were given */
    if (parser->number_of_arguments() < 2) {
	throw (dlib::error ("Error.  You must specify two input files"));
    } else if (parser->number_of_arguments() > 2) {
	std::string extra_arg = (*parser)[2];
	throw (dlib::error ("Error.  Unknown option " + extra_arg));
    }

    /* Copy algorithm values into parms struct */
    std::string val;
    val = parser->get_string("hardware").c_str();
    if (val == "cpu") {
        parms->bsp_threading = BTHR_CPU;
    } else if (val == "cuda") {
        parms->bsp_threading = BTHR_CUDA;
    } else {
        throw (dlib::error ("Error parsing --hardware, unknown option."));
    }
    val = parser->get_string("metric").c_str();
    if (val == "mse") {
        parms->bsp_metric = REGISTRATION_METRIC_MSE;
    } else if (val == "mi") {
        parms->bsp_metric = REGISTRATION_METRIC_MI_MATTES;
    } else {
        throw (dlib::error ("Error parsing --metric, unknown option."));
    }
    val = parser->get_string("flavor").c_str();
    parms->bsp_implementation = val[0];
    val = parser->get_string("process").c_str();
    if (val == "fwd") {
        parms->process = CHECK_GRAD_PROCESS_FWD;
    } else if (val == "bkd") {
        parms->process = CHECK_GRAD_PROCESS_BKD;
    } else if (val == "ctr") {
        parms->process = CHECK_GRAD_PROCESS_CTR;
    } else if (val == "line") {
        parms->process = CHECK_GRAD_PROCESS_LINE;
    } else {
        throw (dlib::error ("Error parsing --metric, unknown option."));
    }
    parms->step_size = parser->get_float ("step");
    parser->assign_int_2 (parms->line_range, "line-range");
    if (parser->have_option ("random-start")) {
        parser->assign_float_2 (parms->random_range, "random-start");
    }

    /* Copy input filenames to parms struct */
    parms->fixed_fn = (*parser)[0];
    parms->moving_fn = (*parser)[1];
    if (parser->have_option ("input-xform")) {
        parms->input_xf_fn = parser->get_string("input-xform");
    }
    if (parser->have_option ("input-search-dir")) {
        parms->input_search_dir_fn = parser->get_string("input-search-dir");
    }
    if (parser->have_option ("output")) {
        parms->output_fn = parser->get_string("output");
    }
    if (parser->have_option ("debug-dir")) {
        parms->debug_dir = parser->get_string ("debug-dir");
    }
    if (parser->have_option ("fixed-roi")) {
        parms->fixed_roi_fn = parser->get_string ("fixed-roi");
    }
    if (parser->have_option ("histogram-prefix")) {
        parms->xpm_hist_prefix 
            = parser->get_string("histogram-prefix").c_str();
    }
}

int
main (int argc, char* argv[])
{
    Check_grad_opts parms;
    Volume::Pointer moving, fixed;
    Volume *moving_grad;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv);

    /* Load images */
    Plm_image::Pointer pli_fixed 
        = Plm_image::New (new Plm_image (parms.fixed_fn));
    Plm_image::Pointer pli_moving 
        = Plm_image::New (new Plm_image (parms.moving_fn));
    fixed = pli_fixed->get_volume_float ();
    moving = pli_moving->get_volume_float ();

    /* Compute spatial gradient */
    moving_grad = volume_make_gradient (moving.get());

    /* Check the gradient */
    check_gradient (&parms, fixed.get(), moving.get(), moving_grad);

    /* Free memory */
    delete moving_grad;

    return 0;
}
