/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bragg_curve_opts.h"
#include "plm_math.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

void
print_usage (void)
{
    printf (
	"Usage: bragg_curve [options]\n"
	"Options:\n"
	" -z z_max          Maximum range (in mm)\n"
	" -s z_spacing      Spacing of points of depth dose curve (in mm)\n"
	" -E energy         Energy of beam (in MeV)\n"
	" -e energy_spread  Sigma of beam energy (in MeV)\n"
	" -O output_file    Write output to a file\n"
    );
    exit (1);
}

void
bragg_curve_opts_init (Bragg_curve_options* options)
{
    options->have_z_max = 0;
    options->z_max = 0.0;
    options->z_spacing = 1.0;
    options->E_0 = 200.0;
    options->have_e_sigma = 0;
    options->e_sigma = 0.0;
    options->output_file = 0;
}

void
parse_args (Bragg_curve_options* options, int argc, char* argv[])
{
    int i, rc;

    bragg_curve_opts_init (options);
    for (i = 1; i < argc; i++) {
	//printf ("ARG[%d] = %s\n", i, argv[i]);
	if (argv[i][0] != '-') break;
	if (!strcmp (argv[i], "-e")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%f", &options->e_sigma);
	    if (rc != 1) {
		print_usage ();
	    }
	    options->have_e_sigma = 1;
	}
	else if (!strcmp (argv[i], "-E")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%lf", &options->E_0);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-O")) {
	    i++;
	    options->output_file = strdup (argv[i]);
	}
	else if (!strcmp (argv[i], "-s")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%f", &options->z_spacing);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-z")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%f", &options->z_max);
	    if (rc != 1) {
		print_usage ();
	    }
	    options->have_z_max = 1;
	}
	else {
	    print_usage ();
	    break;
	}
    }

    if (i < argc) {
	print_usage ();
    }
}
