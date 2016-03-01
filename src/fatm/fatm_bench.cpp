/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
  Options need to set like this:
	fopt.command	    { 'compile', 'run', 'free' }
	fopt.alg	    { 'wncc', 'ncc', 'fncc', 'fancc' }
	fopt.pat_rect
	fopt.sig_rect
	fopt.std_dev_threshold
	fopt.std_dev_expected_min
	fopt.wthresh

  A relevant matlab script for regression testing
  Verified revision 707 (Nov 8, 2007)

    clear; close all;
    pat = loadpfm('pat.pfm');
    sig = loadpfm('sig.pfm');
    fncc_sco = loadpfm('fncc_sco.pfm');
    fancc_sco = loadpfm('fancc_sco.pfm');
    nxc_sco = normxcorr2(pat,sig);
    nxc_sco_1 = nxc_sco(21:end-20,21:end-20);
    dsp(nxc_sco_1,1);dsp(fncc_sco,1);

  A relevant matlab script for regression testing forward fft
  Verified version ??? (Nov 8, 2007)

    clear; close all;
    pat = loadpfm('pat.pfm');
    %f0 = fft2(pat);
    f1 = load('pat_fft.txt');
    f1 = f1(:,1:2:end-1) + i * f1(:,2:2:end);
    f1a = [f1, conj(fliplr([f1(1,2:end);flipud(f1(2:end,2:end))]))];
    %dsp(log(abs(f0)),1);
    %dsp(log(abs(f1a)),1);
    f1ap = ifft2(f1a);
    %f1ap = f1ap(1:21,1:21);
    f1ap = f1ap(end-20:end,end-20:end);
    dsp(pat,1);
    dsp(fliplr(flipud(f1ap)),1);

  A relevant matlab script for regression testing fft multiplication
  Verified version ??? (Nov 8, 2007)

    clear; close all;
    sig = loadpfm('sig.pfm');
    sig_fft = fft2(sig);
    f1 = load('pat_fft.txt');
    f1 = f1(:,1:2:end-1) + i * f1(:,2:2:end);
    pat_fft = [f1, conj(fliplr([f1(1,2:end);flipud(f1(2:end,2:end))]))];
    score_1 = pat_fft .* sig_fft;
    score_2 = load('sco_fft.txt');
    score_2 = score_2(:,1:2:end-1) + i * score_2(:,2:2:end);
    score_2a = [score_2, conj(fliplr([score_2(1,2:end);flipud(score_2(2:end,2:end))]))];
    dsp(ifft2(score_1),1);
    dsp(ifft2(score_2a),1);

  A relevant matlab script for regression testing ifft'd score
  Verified version ??? (Nov 8, 2007)

    clear; close all;
    sig = loadpfm('sig.pfm');
    sig_fft = fft2(sig);
    f1 = load('pat_fft.txt');
    f1 = f1(:,1:2:end-1) + i * f1(:,2:2:end);
    pat_fft = [f1, conj(fliplr([f1(1,2:end);flipud(f1(2:end,2:end))]))];
    score_1 = pat_fft .* sig_fft;
    score_2 = load('sco_ifftd.txt');
    dsp(ifft2(score_1),1);
    dsp(score_2,1);

  A relevant matlab script for regression testing ifft'd score vs. conv
  Verified version ??? (Nov 8, 2007)

    clear; close all;
    pat = loadpfm('pat.pfm');
    sig = loadpfm('sig.pfm');
    cc_sco = conv2(sig,pat,'valid');
    cc_sco1 = fliplr(flipud(cc_sco));
    score_2 = load('sco_ifftd.txt');
    score_2 = score_2 / length(score_2(:));
    %score_2 = score_2(size(pat,1):end,size(pat,2):end);
    score_2 = score_2(1:end-20,1:end-20);
    dsp(cc_sco,1);dsp(score_2,1);
    figure;hold on;plot(cc_sco(end,:));plot(score_2(end,:),'r');

   ----------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES 1  /* Stupid Microsoft */
#include <math.h>
#if !defined (M_PI)
#define M_PI 3.14159265358979323846
#endif
#include "config.h"
#include "fatm.h"
#include "image.h"
#include "clip_pat.h"
#include "timer.h"

static void
build_image_rect (Image_Rect* image_rect, int rows, int cols)
{
    image_rect->dims[0] = cols;
    image_rect->dims[1] = rows;
    image_rect->pmin[0] = 0;
    image_rect->pmin[1] = 0;
}

static void
free_arrays (FATM_Options* fopt)
{
    image_free (&fopt->pat);
    image_free (&fopt->sig);
    image_free (&fopt->score);
}

static void
malloc_and_clip (FATM_Options* fopt, int* pat_size, int* sig_size)
{
    build_image_rect (&fopt->pat_rect, pat_size[0], pat_size[1]);
    build_image_rect (&fopt->sig_rect, sig_size[0], sig_size[1]);

    image_malloc_rand (&fopt->pat, pat_size);
    image_malloc_rand (&fopt->sig, sig_size);

    for (int i = 0; i < 2; i++) {
	fopt->score_rect.score_rect_full.dims[i] = 
	    fopt->sig_rect.dims[i] - fopt->pat_rect.dims[i] + 1;
    }
    image_malloc_rand (&fopt->score, fopt->score_rect.score_rect_full.dims);

#if defined (commentout)
    printf ("Pat Ori: [%d %d], [%d %d]\n" 
	    "Pat Cli: [%d %d], [%d %d]\n" 
	    "Pat Dif: [%d %d], [%d %d]\n" ,
	    fopt.pat_rect.pat_rect_ori.pmin[0],
	    fopt.pat_rect.pat_rect_ori.pmin[1],
	    fopt.pat_rect.pat_rect_ori.dims[0],
	    fopt.pat_rect.pat_rect_ori.dims[1],
	    fopt.pat_rect.pat_rect_clipped.pmin[0],
	    fopt.pat_rect.pat_rect_clipped.pmin[1],
	    fopt.pat_rect.pat_rect_clipped.dims[0],
	    fopt.pat_rect.pat_rect_clipped.dims[1],
	    fopt.pat_rect.pat_rect_diff.pmin[0],
	    fopt.pat_rect.pat_rect_diff.pmin[1],
	    fopt.pat_rect.pat_rect_diff.dims[0],
	    fopt.pat_rect.pat_rect_diff.dims[1]);
    printf ("Sig Ori: [%d %d], [%d %d]\n" 
	    "Sig Cli: [%d %d], [%d %d]\n" 
	    "Sig Val: [%d %d], [%d %d]\n" ,
	    fopt.sig_rect.sig_rect_ori.pmin[0],
	    fopt.sig_rect.sig_rect_ori.pmin[1],
	    fopt.sig_rect.sig_rect_ori.dims[0],
	    fopt.sig_rect.sig_rect_ori.dims[1],
	    fopt.sig_rect.sig_rect_clipped_to_pat.pmin[0],
	    fopt.sig_rect.sig_rect_clipped_to_pat.pmin[1],
	    fopt.sig_rect.sig_rect_clipped_to_pat.dims[0],
	    fopt.sig_rect.sig_rect_clipped_to_pat.dims[1],
	    fopt.sig_rect.sig_rect_valid_scores.pmin[0],
	    fopt.sig_rect.sig_rect_valid_scores.pmin[1],
	    fopt.sig_rect.sig_rect_valid_scores.dims[0],
	    fopt.sig_rect.sig_rect_valid_scores.dims[1]);
    printf ("Sco Ful: [%d %d], [%d %d]\n" 
	    "Sco Val: [%d %d], [%d %d]\n" ,
	    fopt.score_rect.score_rect_full.pmin[0],
	    fopt.score_rect.score_rect_full.pmin[1],
	    fopt.score_rect.score_rect_full.dims[0],
	    fopt.score_rect.score_rect_full.dims[1],
	    fopt.score_rect.score_rect_valid.pmin[0],
	    fopt.score_rect.score_rect_valid.pmin[1],
	    fopt.score_rect.score_rect_valid.dims[0],
	    fopt.score_rect.score_rect_valid.dims[1]);
#endif
}

void
compile_run_test_fancc (FATM_Options* fopt, int* pat_size, int* sig_size)
{
    Image qpat;

    fopt->command = MATCH_COMMAND_COMPILE;
    fopt->alg = MATCH_ALGORITHM_FANCC;

    fatm_compile (fopt);
    fatm_run (fopt);

    image_malloc (&qpat, fopt->pat.dims);
    match_process_render (fopt, image_data(&qpat));

    image_write (&fopt->score, "fancc_sco.pfm");
    image_write (&qpat, "fancc_qpat.pfm");

    image_free (&qpat);
}

void
compile_run_test_fncc (FATM_Options* fopt, int* pat_size, int* sig_size)
{
    fopt->command = MATCH_COMMAND_COMPILE;
    fopt->alg = MATCH_ALGORITHM_FNCC;

    fatm_compile (fopt);
    fatm_run (fopt);

    image_write (&fopt->score, "fncc_sco.pfm");
}

void
compile_run_test_ncc_fft (FATM_Options* fopt, int* pat_size, int* sig_size)
{
    fopt->command = MATCH_COMMAND_COMPILE;
    fopt->alg = MATCH_ALGORITHM_NCC_FFT;

    fatm_compile (fopt);
    fatm_run (fopt);

    image_write (&fopt->score, "ncc_fft_sco.pfm");
}

void
regression_test_main (void)
{
    FATM_Options *fopt;
    int pat_size[] = { 21, 21 };
    int sig_size[] = { 151, 151 };

    /* Initialize fopt array */
    fopt = fatm_initialize ();

    /* Create random test images */
    malloc_and_clip (fopt, pat_size, sig_size);

    /* Save pattern and signal */
    image_write (&fopt->pat, "pat.pfm");
    image_write (&fopt->sig, "sig.pfm");

    /* Run tests */
    compile_run_test_fancc (fopt, pat_size, sig_size);
    memset (fopt->score.data, 0, fopt->score_rect.score_rect_full.dims[0] *
	    fopt->score_rect.score_rect_full.dims[1] * sizeof(double));
    compile_run_test_fncc (fopt, pat_size, sig_size);
    memset (fopt->score.data, 0, fopt->score_rect.score_rect_full.dims[0] *
	    fopt->score_rect.score_rect_full.dims[1] * sizeof(double));
    compile_run_test_fncc (fopt, pat_size, sig_size);
    memset (fopt->score.data, 0, fopt->score_rect.score_rect_full.dims[0] *
	    fopt->score_rect.score_rect_full.dims[1] * sizeof(double));
    compile_run_test_ncc_fft (fopt, pat_size, sig_size);

    /* Clean up */
    free_arrays (fopt);
    fatm_free (fopt);
}

void
dump_clip (int aidx, int nangle)
{
    char filename[1024];
    Image image;
    int dims[] = { 31, 31 };
    double length = 12;
    double width = 3;
    double falloff = 3;
    //double angle = 0;
    double fc = -1;
    double bc = 1;
    double w_falloff = 5;

    double angle = aidx * M_PI / nangle; 

    image_malloc (&image, dims);
    // clip_pat_generate (&image, length, width, falloff, angle, fc, bc);
    weighted_clip_pat_generate (&image, length, width, falloff, angle, fc, bc, w_falloff);
    sprintf (filename, "clip_%02d.pfm", aidx);
    image_write (&image, filename);
    image_free (&image);
}

void
dump_clips (void)
{
    int aidx, nangle;

    nangle = 16;
    for (aidx = 0; aidx < nangle; aidx++) {
	dump_clip (aidx, nangle);
    }
}

#if defined (commentout)
void
match_process_timing_test (FATM_Options *fopt, double* timing_results)
{
    int i;
    double compile_acc = 0.0;
    double run_acc = 0.0;
    const int NUM_RUNS = 10;

    for (i = 0; i < NUM_RUNS; i++) {
	static_timer_reset ();
	match_process_compile (fopt);
	compile_acc += static_timer_get_time ();
	static_timer_reset ();
	match_process_run (fopt);
	run_acc += static_timer_get_time ();
	match_process_free (fopt);
    }
    timing_results[0] = compile_acc / NUM_RUNS;
    timing_results[1] = run_acc / NUM_RUNS;
}
#endif

void
timing_test_main (void)
{
    FATM_Options *fopt;
    int pat_size[] = { 31, 31 };
    int sig_size[] = { 151, 151 };
    //double timing_results_fncc [2];
    //double timing_results_fancc [2];

    /* Initialize options array */
    fopt = fatm_initialize ();

    /* Create random test images */
    malloc_and_clip (fopt, pat_size, sig_size);

    /* Run tests */
    compile_run_test_fancc (fopt, pat_size, sig_size);
    compile_run_test_fncc (fopt, pat_size, sig_size);
    compile_run_test_ncc_fft (fopt, pat_size, sig_size);

    /* Clean up */
    free_arrays (fopt);
    fatm_free (fopt);

#if defined (commentout)
    printf ("Results: %g %g\n", timing_results_fncc[0], timing_results_fncc[1]);
    printf ("Results: %g %g\n", timing_results_fancc[0], timing_results_fancc[1]);
#endif

#if defined (commentout)
    Image qpat;
    image_malloc (&qpat, fopt.pat.dims);
    match_process_render (&fopt, image_data(&qpat));

    image_write (&fopt.pat, "pat.pfm");
    image_write (&fopt.sig, "sig.pfm");
    image_write (&fopt.score, "sco.pfm");
    image_write (&qpat, "qpat.pfm");
    image_free (&qpat);
#endif
}

int
main (int argc, char* argv[])
{
    regression_test_main ();
    //timing_test_main ();
    return 0;
}
