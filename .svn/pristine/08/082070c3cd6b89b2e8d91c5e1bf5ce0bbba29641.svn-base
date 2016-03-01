/* =======================================================================*
   Copyright (c) 2005-2006 Massachusetts General Hospital.
   All rights reserved.
 * =======================================================================*/
#include <windows.h>
#include "mex.h"
#include "match.h"
#include "timer.h"
#include "s_fancc.h"
#include "s_fncc.h"
#include "s_ncc.h"

void
match_initialize_options (Match_Options *options)
{
    options->command = MATCH_COMMAND_RUN;
    options->alg = MATCH_ALGORITHM_FNCC;
    options->alg_data = 0;
    options->std_dev_threshold = MATCH_DEFAULT_STHRESH;
    options->std_dev_expected_min = MATCH_DEFAULT_STHRESH;
    options->wthresh = MATCH_DEFAULT_WTHRESH;
    options->num_partitions = 0;
    image_init (&options->pat);
    image_init (&options->patw);
    image_init (&options->sig);
    image_init (&options->sigw);
    image_init (&options->score);
}

void
match_process_free (Match_Options *options)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	s_fancc_free (options);
	break;
    case MATCH_ALGORITHM_FNCC:
	s_fncc_free (options);
	break;
    default:
	break;
    }
}

void
match_process_compile (Match_Options *options)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	s_fancc_compile (options);
	break;
    case MATCH_ALGORITHM_FNCC:
	s_fncc_compile (options);
	break;
    default:
	break;
    }
}

void
match_timing_run (Match_Options *options)
{
    const int num_runs = 100;
    const int num_warmup = 10;
    int i;
    double timestamp1, timestamp2;
    LARGE_INTEGER clock_count;
    LARGE_INTEGER clock_freq;
    QueryPerformanceFrequency (&clock_freq);

    for (i = 0; i < num_warmup; i++) {
	s_fancc_run (options);
    }

    QueryPerformanceCounter(&clock_count);
    timestamp1 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;

    for (i = 0; i < num_runs; i++) {
	s_fancc_run (options);
    }

    QueryPerformanceCounter(&clock_count);
    timestamp2 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
    mexPrintf ("Run: %g\n", timestamp2 - timestamp1);
}

void
match_timing_compile_and_run (Match_Options *options)
{
    const int num_runs = 100;
    const int num_warmup = 10;
    int i;
    double timestamp1, timestamp2;
    LARGE_INTEGER clock_count;
    LARGE_INTEGER clock_freq;
    QueryPerformanceFrequency (&clock_freq);

    for (i = 0; i < num_warmup; i++) {
	s_fancc_compile (options);
	s_fancc_run (options);
	s_fancc_free (options);
    }

    QueryPerformanceCounter(&clock_count);
    timestamp1 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;

    for (i = 0; i < num_runs; i++) {
	s_fancc_compile (options);
	s_fancc_run (options);
	s_fancc_free (options);
    }

    QueryPerformanceCounter(&clock_count);
    timestamp2 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
    mexPrintf ("Run: %g\n", timestamp2 - timestamp1);
}

void
match_process_run (Match_Options *options)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	if (options->alg_data) {
	    s_fancc_run (options);
	} else {
	    s_fancc_compile (options);
	    s_fancc_run (options);
	    s_fancc_free (options);
	}
	break;
    case MATCH_ALGORITHM_NCC:
	s_ncc_run (options);
	break;
    case MATCH_ALGORITHM_FNCC:
	if (options->alg_data) {
	    s_fncc_run (options);
	} else {
	    s_fncc_compile (options);
	    s_fncc_run (options);
	    s_fncc_free (options);
	}
	break;
    default:
	break;
    }
}

void
match_process_render (Match_Options *options, double* qpat)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	s_fancc_render (options, qpat);
	break;
    default:
	break;
    }
}

void
match_process_timing_test (Match_Options *options, double* timing_results)
{
    int i;
    double compile_acc = 0.0;
    double run_acc = 0.0;
    const int NUM_RUNS = 100;

    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	for (i = 0; i < NUM_RUNS; i++) {
	    static_timer_reset ();
	    s_fancc_compile (options);
	    compile_acc += static_timer_get_time ();
	    static_timer_reset ();
	    s_fancc_run (options);
	    run_acc += static_timer_get_time ();
	    s_fancc_free (options);
	}
	timing_results[0] = compile_acc / NUM_RUNS;
	timing_results[1] = run_acc / NUM_RUNS;
	break;
    case MATCH_ALGORITHM_FNCC:
	for (i = 0; i < NUM_RUNS; i++) {
	    static_timer_reset ();
	    s_fncc_compile (options);
	    compile_acc += static_timer_get_time ();
	    static_timer_reset ();
	    s_fncc_run (options);
	    run_acc += static_timer_get_time ();
	    s_fncc_free (options);
	}
	timing_results[0] = compile_acc / NUM_RUNS;
	timing_results[1] = run_acc / NUM_RUNS;
	break;
    case MATCH_ALGORITHM_NCC:
	for (i = 0; i < NUM_RUNS; i++) {
	    static_timer_reset ();
	    s_ncc_run (options);
	    run_acc += static_timer_get_time ();
	}
	timing_results[0] = compile_acc / NUM_RUNS;
	timing_results[1] = run_acc / NUM_RUNS;
	break;
    default:
	break;
    }
}
