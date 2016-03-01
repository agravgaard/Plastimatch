/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bspline_landmarks.h"
#include "bspline_opts.h"
#include "bspline_regularize.h"
#include "delayload.h"
#if (CUDA_FOUND)
#include "cuda_util.h"
#endif

void
print_usage (void)
{
    printf (
    "Usage: bspline [options] fixed moving\n"
    "Options:\n"
    " -A hardware                Either \"cpu\" or \"cuda\" (default=cpu)\n"
    " -G gpuid                   Select GPU to use (default=0)\n"
    " -a { steepest | lbfgsb }   Choose optimization algorithm\n"
    " -M { mse | mi }            Registration metric (default is mse)\n"
    " -f implementation          Choose implementation (a single letter: a, b, etc.)\n"
    " -m iterations              Maximum iterations (default is 10)\n"
    " -R implementation          Choose regularization implementation (a, b, etc.)\n"
    " -S smoothness              Weight of regularization (floating point number)\n"
    " -B \"F M\"                   # of [F]ixed & [M]oving histogram bins (default=20)\n"
    " --factr value              L-BFGS-B cost converg tol (default is 1e+7)\n"
    " --pgtol value              L-BFGS-B projected grad tol (default is 1e-5)\n"
    " -s \"i j k\"                 Integer knot spacing (voxels)\n"
    " -h prefix                  Generate histograms for each MI iteration\n"
    " -V outfile                 Output vector field\n"
    " -X infile                  Input bspline coefficients\n"
    " -x outfile                 Output bspline coefficients\n"
    " -O outfile                 Output warped image\n"
    " --fixed-landmarks file     Input fixed landmarks file\n"
    " --moving-landmarks file    Input moving landmarks file\n"
    " --warped-landmarks file    Output warped landmarks file\n"
    " --landmark-stiffness float Relative weight of landmarks\n"      
    " -F landm_implementation    Landmark implementation: a or b (default is a)\n"
    " --young-modulus float      Young modulus (cost of vector field gradient)\n"
    " --rbf-radius float         Apply radial basis functions with a given radius\n"
	" --rbf-young-modulus float  RBF Young modulus (cost of RBF vf second derivative)\n"
    " --vopt                     Use v-optimal histograms (experimental/cpu only)\n"
	" --list-gpu                 Enumerates available GPU IDs and displays details\n"
    " --debug                    Create various debug files\n"
    );
    exit (1);
}

void
bspline_opts_parse_args (Bspline_options* options, int argc, char* argv[])
{
    int i, rc;
    Bspline_parms* parms = &options->parms;
    Regularization_parms* reg_parms = parms->reg_parms;
    Bspline_landmarks* blm = parms->blm;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') break;
	if (!strcmp (argv[i], "-A")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    if (!strcmp(argv[i], "cuda") || !strcmp(argv[i], "CUDA")
		|| !strcmp(argv[i], "gpu") || !strcmp(argv[i], "GPU")) {
		parms->threading = BTHR_CUDA;
	    }
	    else {
		parms->threading = BTHR_CPU;
	    }
	}
	else if (!strcmp (argv[i], "-a")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    if (!strcmp(argv[i], "steepest")) {
		parms->optimization = BOPT_STEEPEST;
	    } else if (!strcmp(argv[i], "liblbfgs")) {
		parms->optimization = BOPT_LIBLBFGS;
	    } else if (!strcmp(argv[i], "nlopt-lbfgs")) {
		parms->optimization = BOPT_NLOPT_LBFGS;
	    } else if (!strcmp(argv[i], "nlopt-ld-mma")) {
		parms->optimization = BOPT_NLOPT_LD_MMA;
	    } else if (!strcmp(argv[i], "nlopt-ptn")) {
		parms->optimization = BOPT_NLOPT_PTN_1;
	    } else if (!strcmp(argv[i], "lbfgsb")) {
		parms->optimization = BOPT_LBFGSB;
	    } else {
		fprintf (stderr, "Unknown optimization type: %s\n", argv[i]);
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-G")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%d" , &parms->gpuid);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-f")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    parms->implementation = argv[i][0];
	}
	else if (!strcmp (argv[i], "-m")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%d" , &parms->max_its);
	    if (rc != 1) {
		print_usage ();
	    }
	    parms->max_feval = parms->max_its;
	}
	else if (!strcmp (argv[i], "-R")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    reg_parms->implementation = argv[i][0];
	}
	else if (!strcmp (argv[i], "-S")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &reg_parms->lambda);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-B")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    unsigned int a, b;
	    rc = sscanf (argv[i], "%u %u", &a, &b);
	    if (rc == 1) {
		parms->mi_hist_fixed_bins = a;
    		parms->mi_hist_moving_bins = a;
	    } else if (rc == 2) {
		parms->mi_hist_fixed_bins = a;
    		parms->mi_hist_moving_bins = b;
	    } else {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-M")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    if (!strcmp(argv[i], "mse")) {
		parms->metric_type[0] = REGISTRATION_METRIC_MSE;
	    } else if (!strcmp(argv[i], "mi")) {
		parms->metric_type[0] = REGISTRATION_METRIC_MI_MATTES;
	    } else {
		print_usage ();
	    }
            parms->metric_lambda[0] = 1.;
	}
	else if (!strcmp (argv[i], "-s")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    unsigned int a, b, c;
	    rc = sscanf (argv[i], "%d %d %d", &a, &b, &c);
	    if (rc == 1) {
		options->vox_per_rgn[0] = a;
		options->vox_per_rgn[1] = a;
		options->vox_per_rgn[2] = a;
	    } else if (rc == 3) {
		options->vox_per_rgn[0] = a;
		options->vox_per_rgn[1] = b;
		options->vox_per_rgn[2] = c;
	    } else {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-h")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    parms->xpm_hist_dump = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-O")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_warped_fn = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-V")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_vf_fn = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-x")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_xf_fn = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-X")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->input_xf_fn = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "--debug")) {
	    parms->debug = 1;
	}
	else if (!strcmp (argv[i], "--factr")) {
	    float f;
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &f);
	    if (rc != 1) {
		print_usage ();
	    }
	    parms->lbfgsb_factr = (double) f;
	}
	else if (!strcmp (argv[i], "--pgtol")) {
	    float f;
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &f);
	    if (rc != 1) {
		print_usage ();
	    }
	    parms->lbfgsb_pgtol = (double) f;
	}
	else if (!strcmp (argv[i], "--fixed-landmarks")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->fixed_landmarks = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "--moving-landmarks")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->moving_landmarks = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "--warped-landmarks")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->warped_landmarks = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "--landmark-stiffness")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &blm->landmark_stiffness);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-F")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    blm->landmark_implementation = argv[i][0];
	}
	else if (!strcmp (argv[i], "--young-modulus")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &reg_parms->lambda);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "--rbf-radius")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &parms->rbf_radius);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "--rbf-young-modulus")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &parms->rbf_young_modulus);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "--vopt")) {
	    parms->mi_hist_type = HIST_VOPT;
	}
	else if (!strcmp (argv[i], "--list-gpu")) {
#if (CUDA_FOUND)
	    printf ("Enumerating available GPUs:\n\n");
        LOAD_LIBRARY_SAFE (libplmcuda);
        LOAD_SYMBOL(CUDA_listgpu, libplmcuda);
	    CUDA_listgpu ();
        UNLOAD_LIBRARY (libplmcuda);
#else
	    printf ("\nPlastimatch was not compiled with CUDA support!\n\n");
#endif
	    exit (0);
	}
	else {
	    print_usage ();
	    break;
	}
    }
    if (i+1 >= argc) {
	print_usage ();
    }


    options->fixed_fn = argv[i];
    options->moving_fn = argv[i+1];
    printf ("Fixed = %s\n", options->fixed_fn);
    printf ("Moving = %s\n", options->moving_fn);
}
