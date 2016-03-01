/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fdk.h"
#include "fdk_opts.h"
#include "threading.h"

void 
print_usage (void)
{
    printf (
	"Usage: fdk [options]\n"
	"Options:\n"
	" -A hardware            One of \"cpu\", \"cuda\" or \"opencl\" (default=cpu)\n"
	" -a \"num ((num) num)\"   Use this range of images\n"
	" -r \"r1 r2 r3\"          Set output resolution (in voxels)\n"
	" -f filter              Either \"none\" or \"ramp\" (default=ramp)\n"
	" -s scale               Scale the intensity of the output file\n"
	" -z \"s1 s2 s3\"          Physical size of the reconstruction (in mm)\n"
	" -I indir               The input directory\n"
	" -O outfile             The output file\n"
        " -x \"x0 y0\"             Panel offset (in pixels)\n"
        " -X flavor              Implementation flavor (0,a,b,c,d) (default=c)\n"
    );
    exit (1);
}

void 
set_default_parms (Fdk_parms* parms)
{
    parms->threading = THREADING_CPU_OPENMP;
    parms->image_range_requested = 0;
    parms->first_img = 0;
    parms->last_img = 119;
    parms->skip_img = 1;
    parms->dim[0] = 256;
    parms->dim[1] = 256;
    parms->dim[2] = 100;
    parms->vol_size[0] = 300.0f;
    parms->vol_size[1] = 300.0f;
    parms->vol_size[2] = 150.0f;
    parms->xy_offset[0] = 0.f;
    parms->xy_offset[1] = 0.f;
    parms->scale = 1.0f;
    parms->filter = FDK_FILTER_TYPE_RAMP;
    parms->input_dir = ".";
    parms->output_file = "output.mha";
    parms->flavor = 'c';
    parms->full_fan=1;
    parms->Full_normCBCT_name="Full_norm.mh5";
    parms->Full_radius=120;
    parms->Half_normCBCT_name="Half_norm.mh5";
    parms->Half_radius=220;
}

void 
fdk_parse_args (Fdk_parms* parms, int argc, char* argv[])
{
    int i, rc;
	
    if (argc < 2)
    { print_usage(); exit(1); }

    set_default_parms (parms);
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
	    parms->image_range_requested = 1;
	    rc = sscanf (argv[i], "%d %d %d" , 
		&parms->first_img,
		&parms->skip_img,
		&parms->last_img);
	    if (rc == 1) {
		parms->last_img = parms->first_img;
		parms->skip_img = 1;
	    } else if (rc == 2) {
		parms->last_img = parms->skip_img;
		parms->skip_img = 1;
	    } else if (rc != 3) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-f")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    if (!strcmp(argv[i], "none") || !strcmp(argv[i], "NONE")) {
		parms->filter = FDK_FILTER_TYPE_NONE;
	    }
	    else if (!strcmp(argv[i], "ramp") || !strcmp(argv[i], "RAMP")) {
		parms->filter = FDK_FILTER_TYPE_RAMP;
	    }
	    else {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-X")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    parms->flavor = argv[i][0];
	    if (parms->flavor != '0' && parms->flavor != 'a'
		&& parms->flavor != 'b' && parms->flavor != 'c'
		&& parms->flavor != 'd') {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-I")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    parms->input_dir = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-O")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    parms->output_file = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-r")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    unsigned int a, b, c;
	    rc = sscanf (argv[i], "%d %d %d", &a, &b, &c);
	    if (rc == 1) {
		parms->dim[0] = a;
		parms->dim[1] = a;
		parms->dim[2] = a;
	    } else if (rc == 3) {
		parms->dim[0] = a;
		parms->dim[1] = b;
		parms->dim[2] = c;
	    } else {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-s")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g" , &parms->scale);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-x")) {
	    i++;
	    rc = sscanf (argv[i], "%f %f", 
		&parms->xy_offset[0],
		&parms->xy_offset[1]);
	    if (rc != 2) {
                if (i == (argc-1) || argv[i+1][0] == '-') {
                    fprintf(stderr, 
                        "option %s requires an argument\n", argv[i]);
                    exit(1);
                } else {
                    print_usage ();
                }
	    }
	}
	else if (!strcmp (argv[i], "-z")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g %g %g", 
		&parms->vol_size[0],
		&parms->vol_size[1],
		&parms->vol_size[2]);
	    if (rc == 1) {
		parms->vol_size[1] = parms->vol_size[0];
		parms->vol_size[2] = parms->vol_size[0];
	    } else if (rc != 3) {
		print_usage ();
	    }
	}
	else {
	    print_usage ();
	}
    }
}
