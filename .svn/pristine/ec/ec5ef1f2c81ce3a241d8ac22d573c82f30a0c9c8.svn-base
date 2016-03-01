/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef S_FANCC_H
#define S_FANCC_H

#include "fatm.h"
#include "scorewin.h"

#define MAX_ROW_OPS 40

enum Op_Type 
{
	OP_ADDITION,
	OP_SUBTRACTION,
	OP_EXCHANGE
};

typedef struct fannc_ops {
    enum Op_Type type;
    int bin;
    int pos;
} Fannc_Ops;

typedef struct fannc_row_ops {
    int num_ops;
    Fannc_Ops ops[MAX_ROW_OPS];
} Fannc_Row_Ops;

typedef struct s_fancc_data {
    Image integral_image;
    Image integral_sq_image;
    Image line_integral_image;

    Pattern_Stats p_stats;
#if defined (commentout)
    double p_mean;
    double p_min;
    double p_max;
    double p_var;
    double p_std;
#endif

    int num_bins;
    double* bin_mean;
    int* bin_counts;
    Fannc_Row_Ops* row_ops;
    double* cc_acc;
} S_Fancc_Data;


void s_fancc_compile (FATM_Options* options);
void s_fancc_run (FATM_Options* options);
void s_fancc_free (FATM_Options* options);
void s_fancc_render (FATM_Options *options, double* qpat);

#endif
