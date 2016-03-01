/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   s_fancc: Fast approximate (unweighted) normalized cross correlation
            using the linescan algorithm.
   ----------------------------------------------------------------------- */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fatm.h"
#include "scorewin.h"
#include "integral_img.h"
#include "s_utils.h"
#include "s_fancc.h"

static void s_fancc_scorewin_initialize (FATM_Options* options);
static void s_fancc_scorewin_alloc (FATM_Options* options);
static void s_fancc_scorewin_free (FATM_Options* options);
static void inline s_fancc_score_point (FATM_Options* options,
					Scorewin_Struct* ss);

// #define USE_EXCHANGE 1       /* Anecdotally, this causes things to slow down */
// #define USE_ACCUMULATE 1     /* Anecdotally, this causes things to slow down */

///////////////////////////////////////////////////////////////////
//  Row operations
///////////////////////////////////////////////////////////////////
/* GCS FIX: The num_bins here shadows num_partitions in options */
static void
row_ops_init (S_Fancc_Data* udp, int nrow)
{
    Fannc_Row_Ops* row_ops = (Fannc_Row_Ops*) malloc (sizeof (Fannc_Row_Ops) * nrow);
    udp->num_bins = 3;
    udp->bin_mean = (double*) malloc (udp->num_bins * sizeof(double));
    udp->bin_counts = (int*) malloc (udp->num_bins * sizeof(int));
    udp->cc_acc = (double*) malloc (udp->num_bins * sizeof(double));
    memset (udp->bin_mean, 0, udp->num_bins * sizeof(double));
    memset (udp->bin_counts, 0, udp->num_bins * sizeof(int));
    udp->row_ops = row_ops;
}

static int
row_ops_quant (FATM_Options* options, double value)
{
    for (int i = 0; i < options->num_partitions; i++) {
	double part = options->bin_partitions[i];
	if (value <= part) return i;
    }
    return options->num_partitions;
}

static void
row_op_add (S_Fancc_Data* udp, int row, int col, enum Op_Type type, int bin)
{
    Fannc_Row_Ops* row_op = &udp->row_ops[row];
    Fannc_Ops* op = &row_op->ops[row_op->num_ops];
    op->bin = bin;
    op->pos = col;
    op->type = type;

    row_op->num_ops++;
    if (row_op->num_ops == MAX_ROW_OPS) {
	//mexPrintf ("Error. Too many Ops\n");
	return;
    }
}

static void
row_ops_free (S_Fancc_Data* udp)
{
    free (udp->bin_mean);
    free (udp->bin_counts);
    free (udp->cc_acc);
    free (udp->row_ops);
}

///////////////////////////////////////////////////////////////////
//  Compilation
///////////////////////////////////////////////////////////////////
static void
s_fancc_prepare (FATM_Options* options)
{
    /* Allocate memory */
    S_Fancc_Data* udp = (S_Fancc_Data*) malloc (sizeof(S_Fancc_Data));
    options->alg_data = (void*) udp;

    /* Compute pattern statistics */
    s_pattern_statistics (&udp->p_stats, options);
}

/* Use range to quantize pattern */
static void
s_fancc_partition (FATM_Options* options)
{
    int bc[3];
    int x;
    const double *ps;
#if defined (commentout)
    Image_Rect* prc = &options->pat_rect.pat_rect_clipped;
#endif
    Image_Rect* prc = &options->pat_rect_valid;
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
    Pattern_Stats* p_stats = &udp->p_stats;

    options->num_partitions = 2;
    options->bin_partitions = (double*) malloc (2 * sizeof(double));
    options->bin_partitions[0] = (p_stats->p_min + p_stats->p_mean) / 2.0;
    options->bin_partitions[1] = (p_stats->p_max + p_stats->p_mean) / 2.0;

    ps = image_data(&options->pat) + image_index_pt (options->pat.dims, prc->pmin);
    bc[0] = bc[1] = bc[2] = 0;
    for (x = 0; x < prc->dims[0] * prc->dims[1]; x++) {
	bc[row_ops_quant (options, *ps++)]++;
    }

    /*  (This is the matlab code)
	m = length(A0);
	n = length(A2);
	p = length(A1);
	vv(2) = sqrt(m * (m + n + p) / (m*n + n*n));
	vv(1) = - n * vv(2) / m;
    */
    options->bin_values = (double*) malloc (3 * sizeof(double));
    options->bin_values[2] = sqrt((double) (bc[0] * (bc[0] + bc[1] + bc[2])) / (bc[0] * bc[2] + bc[2] * bc[2]));
    options->bin_values[1] = 0.0;
    options->bin_values[0] = - bc[2] * options->bin_values[2] / bc[0];
}

#if defined (commentout)
/* Use std to quantize pattern - it fails in a bunch of cases */
static void
s_fancc_partition_std (FATM_Options* options)
{
    int bc[3];
    int x;
    const double *ps;
#if defined (commentout)
    Image_Rect* prc = &options->pat_rect.pat_rect_clipped;
#endif
    Image_Rect* prc = &options->pat_rect_valid;
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
    Pattern_Stats* p_stats = &udp->p_stats;

    options->num_partitions = 2;
    options->bin_partitions = (double*) malloc (2 * sizeof(double));
    options->bin_partitions[0] = p_stats->p_mean - p_stats->p_std;
    options->bin_partitions[1] = p_stats->p_mean + p_stats->p_std;

    ps = image_data(&options->pat) + image_index_pt (options->pat.dims, prc->pmin);
    bc[0] = bc[1] = bc[2] = 0;
    for (x = 0; x < prc->dims[0] * prc->dims[1]; x++) {
	bc[row_ops_quant (options, *ps++)]++;
    }

    /*  (This is the matlab code)
	m = length(A0);
	n = length(A2);
	p = length(A1);
	vv(2) = sqrt(m * (m + n + p) / (m*n + n*n));
	vv(1) = - n * vv(2) / m;
    */
    options->bin_values = (double*) malloc (3 * sizeof(double));
    options->bin_values[2] = sqrt((double) (bc[0] * (bc[0] + bc[1] + bc[2])) / (bc[0] * bc[2] + bc[2] * bc[2]));
    options->bin_values[1] = 0.0;
    options->bin_values[0] = - bc[2] * options->bin_values[2] / bc[0];
}
#endif

/* Quantization must be done before calling this */
void
s_fancc_generate_row_ops (FATM_Options* options)
{
    int x, y;
    const double *ps;
#if defined (commentout)
    Image_Rect* prc = &options->pat_rect.pat_rect_clipped;
#endif
    Image_Rect* prc = &options->pat_rect_valid;
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;

    /* Build row operations that describe pattern */
    row_ops_init (udp, prc->dims[0]);
    ps = image_data(&options->pat) + image_index_pt (options->pat.dims, prc->pmin);
    for (y = 0; y < prc->dims[0]; y++) {
	int quant;
	double val;
	udp->row_ops[y].num_ops = 0;
	quant = row_ops_quant (options, *ps);
	val = options->bin_values[quant];
	if (val != 0.0) {
	    row_op_add (udp, y, 0, OP_SUBTRACTION, quant);
	}
	ps ++;
	for (x = 1; x < prc->dims[1]; x++) {
	    int new_quant = row_ops_quant (options, *ps);
	    if (new_quant != quant) {
		double new_val = options->bin_values[new_quant];
#if defined (USE_EXCHANGE)
		if (val == 0.0) {
		    row_op_add (udp, y, x, OP_SUBTRACTION, new_quant);
		} else if (new_val == 0.0) {
		    row_op_add (udp, y, x, OP_ADDITION, quant);
		} else {
		    row_op_add (udp, y, x, OP_EXCHANGE, new_quant);
		}
#else
		if (val == 0.0) {
		    row_op_add (udp, y, x, OP_SUBTRACTION, new_quant);
		} else if (new_val == 0.0) {
		    row_op_add (udp, y, x, OP_ADDITION, quant);
		} else {
		    row_op_add (udp, y, x, OP_SUBTRACTION, new_quant);
		    row_op_add (udp, y, x, OP_ADDITION, quant);
		}
#endif
		quant = new_quant;
		val = new_val;
	    }
	    ps++;
	}
	if (val != 0.0) {
	    row_op_add (udp, y, prc->dims[1], OP_ADDITION, quant);
	}
	ps += options->pat.dims[1] - prc->dims[1];
    }
    for (y = 0; y < udp->num_bins; y++) {
	udp->bin_mean[y] = options->bin_values[y];
    }
}

///////////////////////////////////////////////////////////////////
void
s_fancc_compile (FATM_Options* options)
{
    s_fancc_prepare (options);
    if (options->partition_method == PARTITION_METHOD_AUTOMATIC) {
	s_fancc_partition (options);
    }
    s_fancc_generate_row_ops (options);
    s_fancc_scorewin_alloc (options);
}

void
s_fancc_free (FATM_Options* options)
{
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
    row_ops_free (udp);
    if (options->partition_method == PARTITION_METHOD_AUTOMATIC) {
	free (options->bin_partitions);
	free (options->bin_values);
    }
    s_fancc_scorewin_free (options);
    free (udp);
    options->alg_data = 0;
}

///////////////////////////////////////////////////////////////////
//  Scoring
///////////////////////////////////////////////////////////////////
void
s_fancc_scorewin_1 (FATM_Options* options)
{
#if defined (commentout)
    Image_Rect* prc = &options->pat_rect.pat_rect_clipped;
    Image_Rect* prc = &options->pat_rect_valid;
    Image* pat = &options->pat;
    Image* sig = &options->sig;
    Image* score = &options->score;
#endif
    Image_Rect* zv = &options->score_rect.score_rect_valid;
    Scorewin_Struct ss;

    int* ip = ss.idx_pt;
    int* zp = ss.sco_pt;
    
    /* Iterate through each point on the output image. 
     * Dim 1 is the major ordering (i.e. columns for column-major 
     * for matlab images). */
    for (ip[1] = 0, zp[1] = zv->pmin[1]; ip[1] < zv->dims[1]; ip[1]++, zp[1]++) {
	for (ip[0] = 0, zp[0] = zv->pmin[0]; ip[0] < zv->dims[0]; ip[0]++, zp[0]++) {
	    //mexPrintf ("LOOP: %d %d\n", ip[0], ip[1]);
	    s_fancc_score_point (options, &ss);
	}
    }
}

void
s_fancc_run (FATM_Options* options)
{
#if defined (commentout)
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
#endif

    /* Initialize to zero. Skipped locations will show no correlation. */
    memset ((void *) options->score.data, 0, image_bytes(&options->score));

    /* Make integral images, etc. */
    s_fancc_scorewin_initialize (options);

    /* Score the window */
    options->match_partials = 1;
    s_fancc_scorewin_1 (options);
}

///////////////////////////////////////////////////////////////////
static void inline
s_fancc_score_point (FATM_Options* options,
		     Scorewin_Struct* ss)
{
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
    Pattern_Stats* p_stats = &udp->p_stats;
#if defined (commentout)
    Image* signal = &options->sig;
    Image* score = &options->score;
#endif
    const double* ii;
    const double* ii2;
    const double* li;
    int y, x;
    int* ip = ss->idx_pt;
    int* zp = ss->sco_pt;
    int* d = options->pat_rect_valid.dims;
    int p_ul[2] = { ip[0], ip[1] };
    int p_ur[2] = { ip[0], ip[1] + d[1] };
    int p_ll[2] = { ip[0] + d[0], ip[1] };
    int p_lr[2] = { ip[0] + d[0], ip[1] + d[1] };

    /* Compute window means */
    double p_mean = 0, s_mean = 0;
    double num_pix = d[0] * d[1];
    ((void) p_mean); /* Suppress compiler warning for unused variable */
    p_mean = p_stats->p_mean;
    ii = image_data(&udp->integral_image);
    s_mean = ii[image_index_pt(udp->integral_image.dims,p_ul)]
	- ii[image_index_pt(udp->integral_image.dims,p_ur)]
	- ii[image_index_pt(udp->integral_image.dims,p_ll)]
	+ ii[image_index_pt(udp->integral_image.dims,p_lr)];
    s_mean /= num_pix;

    /* Calculate cc */
    int once = 0;
    if (ip[0] != 0 || ip[1] != 0) once = 0;
    li = image_data(&udp->line_integral_image) 
	    + image_index (udp->line_integral_image.dims, ip[0]+1, ip[1]);
    double cc = 0.0;
#if defined (USE_ACCUMULATE)
    //double* cc_acc = udp->cc_acc;
    double cc_acc[3];
    for (y = 0; y < udp->num_bins; y++) {
	cc_acc[y] = 0.0;
    }
#endif
    //mexPrintf ("3\n");

    for (y = 0; y < d[0]; y++) {
#if defined (USE_EXCHANGE)
	int old_bin = 0;
#endif
	for (x = 0; x < udp->row_ops[y].num_ops; x++) {
	    //mexPrintf ("    %d %d \n", y, x);
	    int type = udp->row_ops[y].ops[x].type;
	    int pos = udp->row_ops[y].ops[x].pos;
	    int bin = udp->row_ops[y].ops[x].bin;
	    if (once) {
		//mexPrintf ("(%d %d) %s %d", y, x, type == OP_ADDITION ? "ADD" : "SUB", pos);
	    }
#if defined (USE_ACCUMULATE)
	    switch (type) {
	    case OP_ADDITION:
		cc_acc[bin] += li[pos];
		break;
#if defined (USE_EXCHANGE)
	    case OP_EXCHANGE:
		cc_acc[old_bin] += li[pos];
		cc_acc[bin] -= li[pos];
		break;
#endif
	    case OP_SUBTRACTION:
		cc_acc[bin] -= li[pos];
		break;
	    }
	    if (once) {
		//mexPrintf (" %g %g\n", ss[pos], cc_acc[bin]);
	    }
#else
	    switch (type) {
	    case OP_ADDITION:
		cc += li[pos] * udp->bin_mean[bin];
		break;
#if defined (USE_EXCHANGE)
	    case OP_EXCHANGE:
		cc += li[pos] * (udp->bin_mean[old_bin] - udp->bin_mean[bin]);
		break;
#endif
	    case OP_SUBTRACTION:
		cc -= li[pos] * udp->bin_mean[bin];
		break;
	    }
#endif
#if defined (USE_EXCHANGE)
	    old_bin = bin;
#endif
	}
	li += udp->line_integral_image.dims[1];
    }

    /* Calculate std */
    double p_var = 0, s_var = 0;
    ii2 = image_data(&udp->integral_sq_image);
    s_var = ii2[image_index_pt(udp->integral_sq_image.dims,p_ul)]
	- ii2[image_index_pt(udp->integral_sq_image.dims,p_ur)]
	- ii2[image_index_pt(udp->integral_sq_image.dims,p_ll)]
	+ ii2[image_index_pt(udp->integral_sq_image.dims,p_lr)];
    s_var -= s_mean * s_mean * num_pix;

    p_var = num_pix * p_stats->p_var;

    double pat_std_dev = (double) sqrt (p_var);
    double sig_std_dev = (double) sqrt (s_var);

    /* GCS MASSIVE HACK */
    /* Need to fix this later.  Pvar is based on the original 
       template rather than the quantized version.  Currently, 
       the quantized version is guaranteed to have a variance of 1.0.  */
    pat_std_dev = sqrt(num_pix);

    if (sig_std_dev < options->std_dev_threshold) {
	return;
    } else if (sig_std_dev < options->std_dev_expected_min) {
	sig_std_dev = options->std_dev_expected_min;
    }
#if defined (USE_ACCUMULATE)
    for (y = 0; y < udp->num_bins; y++) {
	cc += cc_acc[y] * udp->bin_mean[y];
    }
#endif
    cc /= pat_std_dev * sig_std_dev;

    image_data(&options->score)[image_index_pt(options->score.dims,zp)] = cc;
}

///////////////////////////////////////////////////////////////////
void
s_fancc_render (FATM_Options *options, double* qpat)
{
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
#if defined (commentout)
    Image_Rect* prc = &options->pat_rect.pat_rect_clipped;
#endif
    Image_Rect* prc = &options->pat_rect_valid;
    int x, y;

    //mexPrintf ("Rendering...\n");
    for (y = 0; y < prc->dims[0]; y++) {
	int opno = 0;
	double val = 0.0;
	int old_bin = 0;
	Fannc_Row_Ops* row_ops = &udp->row_ops[y];
	//mexPrintf ("Row %d, %d ops\n",y,udp->row_ops[y].num_ops);
	for (x = 0; x < row_ops->num_ops; x++) {
	    //mexPrintf ("  op# %d, type %d, pos %d\n", x, row_ops->ops[x].type, row_ops->ops[x].pos);
	}
	for (x = 0; x < prc->dims[1]; x++) {
	    if (opno < row_ops->num_ops) {
		while (row_ops->ops[opno].pos == x) {
		    //mexPrintf ("Row %d, op %d, pos %d\n",y,opno,x);
		    int bin = row_ops->ops[opno].bin;
		    switch (row_ops->ops[opno].type) {
		    case OP_ADDITION:
			val -= udp->bin_mean[bin];
			break;
		    case OP_EXCHANGE:
			val -= udp->bin_mean[old_bin];
			val += udp->bin_mean[bin];
			break;
		    case OP_SUBTRACTION:
			val += udp->bin_mean[bin];
		    }
		    old_bin = bin;
		    opno++;
		    //mexPrintf ("val <- %g\n", val);
		}
	    }
	    if (y == 8) {
		//mexPrintf ("val = %g\n", val);
	    }
	    *qpat++ = val;
	}
    }
}

///////////////////////////////////////////////////////////////////
static void
s_fancc_scorewin_initialize (FATM_Options* options)
{
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;

    integral_image_compute (&udp->integral_image,
			    &udp->integral_sq_image, 
			    &options->sig,
			    &options->sig_rect_valid);
    line_integral_image_compute (&udp->line_integral_image, 
			    &options->sig,
			    &options->sig_rect_valid);
}

static void
s_fancc_scorewin_alloc (FATM_Options* options)
{
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
    //int* sw_dims = options->sig_rect.dims;
    Image_Rect* srv = &options->sig_rect_valid;
    int ii_dims[2] = { srv->dims[0] + 1, srv->dims[1] + 1 };

    /* Compute integral images of signal.  Note that the integral image 
       has an extra row/column of zeros at the top/left. */
    image_malloc (&udp->integral_image, ii_dims);
    image_malloc (&udp->integral_sq_image, ii_dims);
    image_malloc (&udp->line_integral_image, ii_dims);
}

static void
s_fancc_scorewin_free (FATM_Options* options)
{
    S_Fancc_Data* udp = (S_Fancc_Data*) options->alg_data;
    free (udp->integral_image.data);
    free (udp->integral_sq_image.data);
    free (udp->line_integral_image.data);
}
