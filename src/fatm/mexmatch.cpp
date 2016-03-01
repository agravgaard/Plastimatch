/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------

  RMK: A = pat, AW = pat_rect
       B = sig, BW = sig_rect

  This function is called like this:
        options->command = 'run';
	score = mexmatch (options, A, AW, B, BW);

  Or like this:
        options->command = 'compile';
        pat = mexmatch (options, A, AW);
        options->command = 'run';
        score = mexmatch (options, pat, B, BW);
        options->command = 'free';
        score = mexmatch (options, pat);

  Options need to set like this:
	options->command	    { 'compile', 'run', 'free' }
	options->alg		    { 'wncc', 'ncc', 'fncc', 'fancc' }
	options->pat_rect
	options->sig_rect
	options->std_dev_threshold
	options->std_dev_expected_min
	options->wthresh
	options->truncated_quadratic_threshold
   ----------------------------------------------------------------------- */
#include <math.h>
#include <string.h>
#include "mex.h"
#include "scorewin.h"
#include "mexutils.h"
#include "fatm.h"

extern void _main();

static void
build_image_rect (Image_Rect* image_rect, const mxArray *mex_array)
{
    double *winp;
    winp = (double*) mxGetPr (mex_array);
    /* This conversion is for Matlab to C.  If the inputs were already 
       in C format, we wouldn't need to subtract 1. */
    image_rect->pmin[1] = (int) winp[0] - 1;      // rmin (matlab->c)
    image_rect->pmin[0] = (int) winp[1] - 1;      // cmin (matlab->c)

    image_rect->dims[1] = (int) winp[2];          // nrow
    image_rect->dims[0] = (int) winp[3];          // ncol
}

void
build_image (Image* image, const mxArray *mex_array)
{
    image->dims[1] = mxGetM (mex_array);          // nrow
    image->dims[0] = mxGetN (mex_array);	  // ncol
    image->data = (double*) mxGetPr (mex_array);
}

mxArray *
alloc_output_image (FATM_Options* fopt)
{
    int i;
    Image_Rect* zrf = &fopt->score_rect.score_rect_full;

    for (i = 0; i < 2; i++) {
	zrf->dims[i] = fopt->sig_rect.dims[i] - fopt->pat_rect.dims[i] + 1;
    }
    return mxCreateDoubleMatrix (zrf->dims[1], zrf->dims[0], mxREAL);
}

void
mexFunction (int          nlhs,
	     mxArray      *plhs[],
	     int          nrhs,
	     const mxArray *prhs[])
{
    FATM_Options* options;
    mxArray* tmp_mxa;
    char* tmp_buf;

    /* Initialize options array */
    options = fatm_initialize ();

    /* Check for proper number of arguments */
    verify_mex_nargs ("mexmatch", nlhs, nrhs, 2, 5, 0, 2);

    /* Check if the first argument is a struct */
    if (!mxIsStruct(prhs[0])) {
	mexErrMsgTxt("The first argument must be an options struct");
    }

    /* Get options->command */
    tmp_mxa = mxGetField (prhs[0], 0, "command");
    if (!tmp_mxa) {
	mexErrMsgTxt("No command specified in options");
    }
    tmp_buf = mex_strdup (tmp_mxa);
    if (strcmp (tmp_buf,"compile") == 0) {
	options->command = MATCH_COMMAND_COMPILE;
    } else if (strcmp (tmp_buf,"run") == 0) {
	options->command = MATCH_COMMAND_RUN;
    } else if (strcmp (tmp_buf,"free") == 0) {
	options->command = MATCH_COMMAND_FREE;
    } else if (strcmp (tmp_buf,"timing_test") == 0) {
	options->command = MATCH_COMMAND_TIMING_TEST;
    } else {
	mexErrMsgTxt("Unknown options->command value");
    }
    mxFree (tmp_buf);

    /* Get options->alg */
    tmp_mxa = mxGetField (prhs[0], 0, "alg");
    if (!tmp_mxa) {
	mexErrMsgTxt("No alg specified in options");
    }
    tmp_buf = mex_strdup (tmp_mxa);
    if (strcmp (tmp_buf,"wncc") == 0) {
	options->alg = MATCH_ALGORITHM_WNCC;
    } else if (strcmp (tmp_buf,"ncc") == 0) {
	options->alg = MATCH_ALGORITHM_NCC;
    } else if (strcmp (tmp_buf,"fncc") == 0) {
	options->alg = MATCH_ALGORITHM_FNCC;
    } else if (strcmp (tmp_buf,"fancc") == 0) {
	options->alg = MATCH_ALGORITHM_FANCC;
    } else if (strcmp (tmp_buf,"ncc_fft") == 0) {
	options->alg = MATCH_ALGORITHM_NCC_FFT;
    } else if (strcmp (tmp_buf,"rssd") == 0) {
	options->alg = MATCH_ALGORITHM_RSSD;
    } else {
	mexPrintf("options->alg was (%s)\n", tmp_buf);
	mexErrMsgTxt("Unknown options->alg value");
    }
    mxFree (tmp_buf);

    /* Get options->wthresh */
    tmp_mxa = mxGetField (prhs[0], 0, "wthresh");
    if (tmp_mxa) {
	if (!verify_scalar_double (tmp_mxa)) {
	    mexErrMsgTxt("Input options->wthresh should be a scalar double");
	}
	options->wthresh = mxGetScalar (tmp_mxa);
    }

    /* Get options->std_dev_threshold */
    tmp_mxa = mxGetField (prhs[0], 0, "std_dev_threshold");
    if (tmp_mxa) {
	if (!verify_scalar_double (tmp_mxa)) {
	    mexErrMsgTxt("Input options->std_dev_threshold should be a scalar double");
	}
	options->std_dev_threshold = mxGetScalar (tmp_mxa);
	options->std_dev_expected_min = options->std_dev_threshold;
    }

    /* Get options->std_dev_expected_min */
    tmp_mxa = mxGetField (prhs[0], 0, "std_dev_expected_min");
    if (tmp_mxa) {
	if (!verify_scalar_double (tmp_mxa)) {
	    mexErrMsgTxt("Input options->std_dev_expected_min should be a scalar double");
	}
	options->std_dev_expected_min = mxGetScalar (tmp_mxa);
    }

    /* Get options->truncated_quadratic_threshold */
    tmp_mxa = mxGetField (prhs[0], 0, "truncated_quadratic_threshold");
    if (tmp_mxa) {
	if (!verify_scalar_double (tmp_mxa)) {
	    mexErrMsgTxt("Input options->truncated_quadratic_threshold should be a scalar double");
	}
	options->truncated_quadratic_threshold = mxGetScalar (tmp_mxa);
    }

    /* Get options->pat_rect */
    tmp_mxa = mxGetField (prhs[0], 0, "pat_rect");
    if (tmp_mxa) {
	if (!verify_mex_rda_4 (tmp_mxa)) {
	    mexErrMsgTxt("Input options->pat_rect should be a vector with 4 values");
	}
        build_image_rect (&options->pat_rect, tmp_mxa);
    } else {
	mexErrMsgTxt("Input options->pat_rect is missing");
    }

    /* Get options->sig_rect */
    tmp_mxa = mxGetField (prhs[0], 0, "sig_rect");
    if (tmp_mxa) {
	if (!verify_mex_rda_4 (tmp_mxa)) {
	    mexErrMsgTxt("Input options->sig_rect should be a vector with 4 values");
	}
        build_image_rect (&options->sig_rect, tmp_mxa);
    } else {
	mexErrMsgTxt("Input options->sig_rect is missing");
    }

    /* Get options->bin_partitions */
    tmp_mxa = mxGetField (prhs[0], 0, "bin_partitions");
    if (tmp_mxa) {
	if (!verify_mex_rda (tmp_mxa)) {
	    mexErrMsgTxt("Input options->bin_partitions should be a real double array");
	}
	int mrows = mxGetM (tmp_mxa);
	int ncols = mxGetN (tmp_mxa);
	if (mrows > ncols) {
	    options->num_partitions = mrows;
	} else {
	    options->num_partitions = ncols;
	}
	options->bin_partitions = mxGetPr (tmp_mxa);
    }

    /* Get options->bin_values */
    tmp_mxa = mxGetField (prhs[0], 0, "bin_values");
    if (tmp_mxa) {
	if (!verify_mex_rda (tmp_mxa)) {
	    mexErrMsgTxt("Input options->bin_values should be a real double array");
	}
	int mrows = mxGetM (tmp_mxa);
	int ncols = mxGetN (tmp_mxa);
	if (mrows <= options->num_partitions && ncols <= options->num_partitions) {
	    mexErrMsgTxt("Input options->bin_values should be longer than bin_partitions");
	}
	options->bin_values = mxGetPr (tmp_mxa);
    } else {
	if (options->num_partitions > 0) {
	    mexErrMsgTxt("Input options->bin_values required with bin_partitions");
	}
    }

    /* Decode pattern & signal images */
    if (options->command == MATCH_COMMAND_RUN) {
	if (nlhs != 1) {
	    mexErrMsgTxt("Usage: mexmatch ('run') requires one output.");
	}
	if (check_bundled_pointer_for_matlab (prhs[1])) {
	    /* Check if the input contains a precompiled template */
	    if (options->alg != MATCH_ALGORITHM_FANCC) {
		mexErrMsgTxt("Template compilation only valid for fancc");
	    }
	    options->alg_data = unbundle_pointer_for_matlab (prhs[1]);
	    if (nrhs == 4) {
		build_image (&options->pat, prhs[2]);
		build_image (&options->sig, prhs[3]);
	    } else if (nrhs == 4) {
		build_image (&options->pat, prhs[2]);
		build_image (&options->patw, prhs[3]);
		build_image (&options->sig, prhs[4]);
		build_image (&options->sigw, prhs[5]);
	    } else {
		mexErrMsgTxt("Usage: mexmatch (options, pat, A, [AW], B, [BW])");
	    }
	} else {
	    /* GCS This is buggy.  Need to check if inputs are arrays, 
		    and if weight matrices are empty. */
	    if (nrhs == 3) {
		build_image (&options->pat, prhs[1]);
		build_image (&options->sig, prhs[2]);
	    } else if (nrhs == 5) {
		build_image (&options->pat, prhs[1]);
		build_image (&options->patw, prhs[2]);
		build_image (&options->sig, prhs[3]);
		build_image (&options->sigw, prhs[4]);
	    } else {
		mexErrMsgTxt("Usage: mexmatch (options, A, [AW], B, [BW])");
	    }
	}

	/* Create matrix for output */
	//mexPrintf("alloc_output_image\n");
	plhs[0] = alloc_output_image (options);
	if (mxGetM (plhs[0]) == 0) {
	    fatm_free (options);
	    return;
	}
	build_image (&options->score, plhs[0]);

	/* Compile */
	//mexPrintf("fatm_compile\n");
	fatm_compile (options);

	/* Run */
	//mexPrintf("fatm_run\n");
	fatm_run (options);

	/* Free */
	//mexPrintf("match_process_free\n");
	fatm_free (options);

    }
    else if (options->command == MATCH_COMMAND_COMPILE) {
#if defined (commentout)
	if (nlhs < 1 || nlhs > 2) {
	    mexErrMsgTxt("Usage: mexmatch ('compile') requires one or two outputs.");
	}
	if (options->alg != MATCH_ALGORITHM_FANCC) {
	    mexErrMsgTxt("Template compilation only valid for fancc");
	}
	/* GCS This is buggy.  Need to combine image and mask if 
		requested. */
	build_image (&options->pat, prhs[1]);
        clip_pattern (&options);

	/* Do it! */
	match_process_compile (&options);

	/* Bundle and return the pointer */
	plhs[0] = bundle_pointer_for_matlab (options->alg_data);

	if (nlhs == 2) {
	    Image_Rect* pro = &options->pat_rect.pat_rect_ori;
	    plhs[1] = mxCreateDoubleMatrix (pro->dims[1], 
					    pro->dims[0], mxREAL);
	    match_process_render (&options, mxGetPr (plhs[1]));
	}
#endif
	mexErrMsgTxt("Sorry, compile option is not implemented");

    }
    else if (options->command == MATCH_COMMAND_FREE) {
#if defined (commentout)
	if (nlhs != 0) {
	    mexErrMsgTxt("Usage: mexmatch ('free') requires zero outputs.");
	}
	if (options->alg != MATCH_ALGORITHM_FANCC) {
	    mexErrMsgTxt("Template compilation only valid for fancc");
	}
	if (!check_bundled_pointer_for_matlab (prhs[1])) {
	    mexErrMsgTxt("Only bundled pointers may be freed");
	}
	options->alg_data = unbundle_pointer_for_matlab (prhs[1]);

	/* Do it! */
	match_process_free (&options);
#endif
	mexErrMsgTxt("Sorry, free option is not implemented");
    }
}
