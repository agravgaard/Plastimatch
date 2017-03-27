/* =======================================================================*
   Copyright (c) 2005-2006 Massachusetts General Hospital.
   All rights reserved.
 * =======================================================================*/
#ifndef MATCH_H
#define MATCH_H

#include "image.h"

#define MATCH_DEFAULT_STHRESH   0.0001
#define MATCH_DEFAULT_WTHRESH   0.0001

typedef enum match_command Match_Command;
enum match_command {
    MATCH_COMMAND_COMPILE,
    MATCH_COMMAND_RUN,
    MATCH_COMMAND_FREE,
    MATCH_COMMAND_TIMING_TEST,
};

typedef enum match_algorithm Match_Algorithm;
enum match_algorithm {
    MATCH_ALGORITHM_WNCC,
    MATCH_ALGORITHM_NCC,
    MATCH_ALGORITHM_FNCC,
    MATCH_ALGORITHM_FANCC
};

typedef struct pat_rect_info Pat_Rect_Info;
struct pat_rect_info {
    Image_Rect pat_rect_ori;
    Image_Rect pat_rect_clipped;
    Image_Rect pat_rect_diff;
};

typedef struct sig_rect_info Sig_Rect_Info;
struct sig_rect_info {
    Image_Rect sig_rect_ori;
    Image_Rect sig_rect_clipped_to_pat;
    Image_Rect sig_rect_valid_scores;
};

typedef struct score_rect_info Score_Rect_Info;
struct score_rect_info {
    Image_Rect score_rect_full;
    Image_Rect score_rect_valid;
};

typedef struct match_options Match_Options;
struct match_options {
    Match_Command command;
    Match_Algorithm alg;
    Image pat;
    Image patw;
    Image sig;
    Image sigw;
    Image score;
    Sig_Rect_Info sig_rect;
    Pat_Rect_Info pat_rect;
    Score_Rect_Info score_rect;
    int match_partials;
    int have_weights;
    void* alg_data;
    double wthresh;
    double std_dev_threshold;
    double std_dev_expected_min;

    /* For fancc */
    int num_partitions;        /* =0 when no partitions spec'd by caller */
    double* bin_partitions;
    double* bin_values;
};

void match_initialize_options (Match_Options *options);
void match_process_free (Match_Options *options);
void match_process_compile (Match_Options *options);
void match_process_run (Match_Options *options);
void match_process_render (Match_Options *options, double* qpat);
void match_process_timing_test (Match_Options *options, double* timing_results);

#endif
