/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "drr.h"
#include "drr_opts.h"
#include "plm_math.h"
#include "threading.h"

void
print_usage (void)
{
    printf (
	"Usage: drr [options] [infile]\n"
	"Options:\n"
	" -A hardware       Either \"cpu\" or \"cuda\" (default=cpu)\n"
	" -y angle          Gantry angle for source\n"
	" -a num            Generate num equally spaced angles\n"
	" -N angle          Difference between neighboring angles (in degrees)\n"
	" -nrm \"x y z\"      Set the normal vector for the panel\n"
	" -vup \"x y z\"      Set the vup vector (toward top row) for the panel\n"
	" -g \"sad sid\"      Set the sad, sid (in mm)\n"
	" -r \"r c\"          Set output resolution (in pixels)\n"
	" -s scale          Scale the intensity of the output file\n"
	" -e                Do exponential mapping of output values\n"
	" -c \"r c\"          Set the image center (in pixels)\n"
	" -z \"s1 s2\"        Set the physical size of imager (in mm)\n"
	" -w \"r1 r2 c1 c2\"  Only produce image for pixes in window (in pix)\n"
	" -t outformat      Select output format: pgm, pfm or raw\n"
	" -S outprefix      Output ray tracing details\n"
	//" -S                Output multispectral output files\n"
	//" -i algorithm      Choose algorithm {exact,uniform,tri_exact,tri_approx}\n"
	" -i algorithm      Choose algorithm {exact,uniform}\n"
	" -o \"o1 o2 o3\"     Set isocenter position\n"
	" -G                Create geometry files only, not drr images.\n"
	" -P                Suppress attenuation preprocessing.\n"
	" -I infile         Set the input file in mha format\n"
	" -O outprefix      Generate output files using the specified prefix\n"
    );
    exit (1);
}

void
drr_opts_init (Drr_options* options)
{
    options->threading = THREADING_CPU_OPENMP;
    options->image_resolution[0] = 128;
    options->image_resolution[1] = 128;
    options->image_size[0] = 600;
    options->image_size[1] = 600;
    options->have_image_center = 0;
    options->have_image_window = 0;
    options->isocenter[0] = 0.0f;
    options->isocenter[1] = 0.0f;
    options->isocenter[2] = 0.0f;

    options->start_angle = 0.f;
    options->num_angles = 1;
    options->have_angle_diff = 0;
    options->angle_diff = 1.0f;

    options->have_nrm = 0;
    options->nrm[0] = 1.0f;
    options->nrm[1] = 0.0f;
    options->nrm[2] = 0.0f;
    options->vup[0] = 0.0f;
    options->vup[1] = 0.0f;
    options->vup[2] = 1.0f;

    options->sad = 1000.0f;
    options->sid = 1630.0f;
    options->scale = 1.0f;

    options->exponential_mapping = 0;
    options->output_format= OUTPUT_FORMAT_PFM;
    options->preprocess_attenuation = true;
    options->output_details_prefix = "";
    options->output_details_fn = "";
    options->algorithm = DRR_ALGORITHM_EXACT;
    options->input_file = 0;
    options->geometry_only = 0;
    options->output_prefix = "out_";
}

void
set_image_parms (Drr_options* options)
{
    if (!options->have_image_center) {
	options->image_center[0] = (options->image_resolution[0]-1)/2.0;
	options->image_center[1] = (options->image_resolution[1]-1)/2.0;
    }
    if (!options->have_image_window) {
	options->image_window[0] = 0;
	options->image_window[1] = options->image_resolution[1] - 1;
	options->image_window[2] = 0;
	options->image_window[3] = options->image_resolution[0] - 1;
    }
    if (options->have_angle_diff) {
	options->angle_diff *= (float) (M_TWOPI / 360.0);
    } else {
	options->angle_diff = M_TWOPI / options->num_angles;
    }
}

void
parse_args (Drr_options* options, int argc, char* argv[])
{
    int i, rc;

    drr_opts_init (options);
    for (i = 1; i < argc; i++) {
	//printf ("ARG[%d] = %s\n", i, argv[i]);
	if (argv[i][0] != '-') break;
	if (!strcmp (argv[i], "-A")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    if (++i >= argc) { print_usage(); }
#if CUDA_FOUND
	    if (!strcmp(argv[i], "cuda") || !strcmp(argv[i], "CUDA")
		|| !strcmp(argv[i], "gpu") || !strcmp(argv[i], "GPU")) {
		options->threading = THREADING_CUDA;
		continue;
	    }
#endif
#if OPENCL_FOUND
	    if (!strcmp(argv[i], "opencl") || !strcmp(argv[i], "OPENCL")
		|| !strcmp(argv[i], "gpu") || !strcmp(argv[i], "GPU")) {
		options->threading = THREADING_OPENCL;
		continue;
	    }
#endif
	    /* Default */
	    options->threading = THREADING_CPU_OPENMP;
	}
	else if (!strcmp (argv[i], "-r")) {
	    /* Note: user inputs row, then column.  But internally they 
	       are stored as column, then row. */
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%d %d", 
		&options->image_resolution[1], 
		&options->image_resolution[0]);
	    if (rc == 1) {
		options->image_resolution[0] = options->image_resolution[1];
	    } else if (rc != 2) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-I")) {
	    if (++i >= argc) { print_usage(); }
	    options->input_file = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-O")) {
	    if (++i >= argc) { print_usage(); }
	    options->output_prefix = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-y")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%g" , &options->start_angle);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-a")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%d" , &options->num_angles);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-N")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%g" , &options->angle_diff);
	    if (rc != 1) {
		print_usage ();
	    }
	    options->have_angle_diff = 1;
	}
	else if (!strcmp (argv[i], "-nrm")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%f %f %f", &options->nrm[0],
		&options->nrm[1], &options->nrm[2]);
	    if (rc != 3) {
		print_usage ();
	    }
	    options->have_nrm = 1;
	}
	else if (!strcmp (argv[i], "-vup")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%f %f %f", &options->vup[0],
		&options->vup[1], &options->vup[2]);
	    if (rc != 3) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-s")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%g" , &options->scale);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-t")) {
	    if (++i >= argc) { print_usage(); }
	    if (!strcmp (argv[i], "pfm")) {
		options->output_format = OUTPUT_FORMAT_PFM;
	    }
	    else if (!strcmp (argv[i], "pgm")) {
		options->output_format = OUTPUT_FORMAT_PGM;
	    }
	    else if (!strcmp (argv[i], "raw")) {
		options->output_format = OUTPUT_FORMAT_RAW;
	    }
	    else {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-c")) {
	    /* Note: user inputs row, then column.  But internally they 
	       are stored as column, then row. */
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%g %g", 
		&options->image_center[1],
		&options->image_center[0]);
	    if (rc == 1) {
		options->image_center[0] = options->image_center[1];
	    } else if (rc != 2) {
		print_usage ();
	    }
	    options->have_image_center = 1;
	}
	else if (!strcmp (argv[i], "-z")) {
	    /* Note: user inputs row, then column.  But internally they 
	       are stored as column, then row. */
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%g %g", 
		&options->image_size[1],
		&options->image_size[0]);
	    if (rc == 1) {
		options->image_size[0] = options->image_size[1];
	    } else if (rc != 2) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-g")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%g %g", &options->sad, &options->sid);
	    if (rc != 2) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-w")) {
	    /* Note: user inputs row start, row end, 
               column start, column end */
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%d %d %d %d",
		&options->image_window[0],
		&options->image_window[1],
		&options->image_window[2],
		&options->image_window[3]);
	    if (rc == 2) {
		options->image_window[2] = options->image_window[0];
		options->image_window[3] = options->image_window[1];
	    } else if (rc != 4) {
		print_usage ();
	    }
	    options->have_image_window = 1;
	}
	else if (!strcmp (argv[i], "-i")) {
	    if (++i >= argc) { print_usage(); }
	    if (!strcmp(argv[i], "exact")) {
		options->algorithm = DRR_ALGORITHM_EXACT;
	    } else if (!strcmp(argv[i], "tri_exact")) {
		options->algorithm = DRR_ALGORITHM_TRILINEAR_EXACT;
	    } else if (!strcmp(argv[i], "tri_approx")) {
		options->algorithm = DRR_ALGORITHM_TRILINEAR_APPROX;
	    } else if (!strcmp(argv[i], "uniform")) {
		options->algorithm = DRR_ALGORITHM_UNIFORM;
	    } else {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-o")) {
	    if (++i >= argc) { print_usage(); }
	    rc = sscanf (argv[i], "%g %g %g" , 
		&options->isocenter[0],
		&options->isocenter[1],
		&options->isocenter[2]);
	    if (rc != 3) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-S")) {
	    if (++i >= argc) { print_usage(); }
	    options->output_details_prefix = argv[i];
	}
	else if (!strcmp (argv[i], "-e")) {
	    options->exponential_mapping = 1;
	}
	else if (!strcmp (argv[i], "-G")) {
	    options->geometry_only = 1;
	}
	else if (!strcmp (argv[i], "-P")) {
	    options->preprocess_attenuation = false;
	}
	else {
	    print_usage ();
	    break;
	}
    }

    if (!options->input_file) {
	if (i < argc) {
	    options->input_file = strdup (argv[i++]);
	}
    }
    if (i < argc) {
	print_usage ();
    }
    if (!options->input_file && !options->geometry_only) {
	print_usage ();
    }

    set_image_parms (options);
}
