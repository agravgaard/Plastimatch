/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "demons_opts.h"

void
print_usage (void)
{
    printf (
	"Usage: demons [options] fixed moving\n"
	"Options:\n"
	" -A algorithm         Either \"cpu\" or \"cuda\" (default=cpu)\n"
	" -a accel             Acceleration factor (default=1)\n"
	" -e denom_eps         Minimum allowed denominator magnitude (default=1)\n"
	" -f \"i j k\"           Width of smoothing kernel (voxels)\n"
	" -h homogenization    Cachier's alpha^2 homogenization (default=1)\n"
	" -m iterations        Maximum iterations (default=10)\n"
	" -s std               Std dev (mm) of smoothing kernel (default=5)\n"
	" -O outfile           Output warped image file\n"
	" -V outfile           Output vector field\n"
    );
    exit (1);
}

void
parse_args (Demons_options* options, int argc, char* argv[])
{
    int i, rc;
    Demons_parms* parms = &options->parms;

    options->output_img_fn = 0;
    options->output_vf_fn = 0;
    demons_default_parms (parms);

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') break;
	if (!strcmp (argv[i], "-A")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
#if CUDA_FOUND
	    if (!strcmp(argv[i], "cuda") || !strcmp(argv[i], "CUDA")) {
		parms->threading = THREADING_CUDA;
		continue;
	    }
#endif
#if OPENCL_FOUND
	    if (!strcmp(argv[i], "opencl") || !strcmp(argv[i], "OPENCL")) {
		parms->threading = THREADING_OPENCL;
		continue;
	    }
#endif
	    /* Default */
	    parms->threading = THREADING_CPU_OPENMP;
	}
	else if (!strcmp (argv[i], "-a")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g" , &parms->accel);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-e")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g" , &parms->denominator_eps);
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
	    rc = sscanf (argv[i], "%d %d %d", 
			 &parms->filter_width[0],
			 &parms->filter_width[1],
			 &parms->filter_width[2]);
	    if (rc != 3) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-h")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g" , &parms->homog);
	    if (rc != 1) {
		print_usage ();
	    }
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
	}
        else if (!strcmp (argv[i], "-s")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g" , &parms->filter_std);
	    if (rc != 1) {
		print_usage ();
	    }
	}
        else if (!strcmp (argv[i], "-O")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_img_fn = strdup (argv[i]);
	}
        else if (!strcmp (argv[i], "-V")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_vf_fn = strdup (argv[i]);
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
