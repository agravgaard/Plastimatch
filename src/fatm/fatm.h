/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef MATCH_H
#define MATCH_H

#include "image.h"

#define MATCH_DEFAULT_STHRESH   0.0001
#define MATCH_DEFAULT_WTHRESH   0.0001
#define MATCH_DEFAULT_TQTHRESH	10000.0

enum match_command {
    MATCH_COMMAND_COMPILE,
    MATCH_COMMAND_RUN,
    MATCH_COMMAND_FREE,
    MATCH_COMMAND_TIMING_TEST,
};
typedef enum match_command Match_Command;

enum match_algorithm {
    MATCH_ALGORITHM_WNCC,
    MATCH_ALGORITHM_NCC,
    MATCH_ALGORITHM_FNCC,
    MATCH_ALGORITHM_FANCC,
    MATCH_ALGORITHM_NCC_FFT,
    MATCH_ALGORITHM_RSSD
};
typedef enum match_algorithm Match_Algorithm;

enum enum_partition_method {
    PARTITION_METHOD_PRECOMPUTED,
    PARTITION_METHOD_AUTOMATIC
};
typedef enum enum_partition_method Partition_Method;

#if defined (commentout)
typedef struct pat_rect_info Pat_Rect_Info;
struct pat_rect_info {
    Image_Rect pat_rect_ori;
    Image_Rect pat_rect_clipped;
    Image_Rect pat_rect_diff;
};

typedef struct sig_rect_info Sig_Rect_Info;
struct sig_rect_info {
    Image_Rect sig_rect_ori;
    //Image_Rect sig_rect_clipped_to_pat;
    Image_Rect sig_rect_valid_scores;
};
#endif

typedef struct score_rect_info Score_Rect_Info;
struct score_rect_info {
    Image_Rect score_rect_full;
    Image_Rect score_rect_valid;
};

/*
class Score_Rect_Info {

 public:
    Image_Rect score_rect_full;
    Image_Rect score_rect_valid;

 public:
    Score_Rect_Info ();
    ~Score_Rect_Info ();
};
*/
typedef struct pattern_stats Pattern_Stats;
struct pattern_stats {
    double p_mean;
    double p_var;
    double p_std;
    double p_min;
    double p_max;
};

typedef struct fatm_options FATM_Options;
struct fatm_options {
    /* Command specification */
    Match_Command command;
    Match_Algorithm alg;

    /* Input images & regions */
    Image pat;
    Image patw;
    Image_Rect pat_rect;
    Image sig;
    Image sigw;
    Image_Rect sig_rect;

    /* Output image & region */
    Image score;
    Score_Rect_Info score_rect;

    /* Clipping rectangles */
    Image_Rect pat_rect_valid;
    Image_Rect sig_rect_scan;
    Image_Rect sig_rect_valid;

    /* Scoring options */
    int match_partials;
    int have_weights;
    double wthresh;
    double std_dev_threshold;
    double std_dev_expected_min;
    double truncated_quadratic_threshold;

    /* Special options for fancc */
    Partition_Method partition_method;
    int num_partitions;        /* =0 when no partitions spec'd by caller */
    double* bin_partitions;
    double* bin_values;

    /* Private data - stored between compile and run */
    void* alg_data;
};

FATM_Options* fatm_initialize (void);
void fatm_compile (FATM_Options* fopt);
void fatm_run (FATM_Options* fopt);
void fatm_free (FATM_Options* fopt);

void match_initialize_options (FATM_Options *options);
void match_process_render (FATM_Options *options, double* qpat);
void match_process_free (FATM_Options *options);


#endif
