/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   s_ncc:  Naive implementation of (unweighted) normalized cross correlation
   ----------------------------------------------------------------------- */
#include <string.h>
#include <math.h>
#include "scorewin.h"
#include "s_ncc.h"

static inline void s_ncc_score_point_1 (FATM_Options* options,
					Scorewin_Struct* ss);


static void
s_ncc_scorewin_1 (FATM_Options* options)
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

#if defined (commentout)
    double timestamp1, timestamp2;
    LARGE_INTEGER clock_count;
    LARGE_INTEGER clock_freq;
    QueryPerformanceFrequency (&clock_freq);
    QueryPerformanceCounter(&clock_count);
    timestamp1 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
#endif

    int* ip = ss.idx_pt;
    int* zp = ss.sco_pt;
    
    /* Iterate through each point on the output image. 
     * Dim 1 is the major ordering (i.e. columns for column-major 
     * for matlab images). */
    //mexPrintf ("---LOOPING: %d -> %d\n", 0, zv->dims[1]);
    for (ip[1] = 0, zp[1] = zv->pmin[1]; ip[1] < zv->dims[1]; ip[1]++, zp[1]++) {
	for (ip[0] = 0, zp[0] = zv->pmin[0]; ip[0] < zv->dims[0]; ip[0]++, zp[0]++) {
	    //mexPrintf ("LOOP: %d %d\n", ip[0], ip[1]);
	    s_ncc_score_point_1 (options, &ss);
	}
    }

#if defined (commentout)
    QueryPerformanceCounter(&clock_count);
    timestamp2 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
    mexPrintf ("Run: %g\n", timestamp2 - timestamp1);
#endif
}

void
s_ncc_run (FATM_Options* options)
{
    /* Initialize to zero. Skipped locations will show no correlation. */
    memset ((void *) options->score.data, 0, image_bytes(&options->score));

    /* Iterate through window */
    s_ncc_scorewin_1 (options);
}

static inline void
s_ncc_score_point_1 (FATM_Options* options,
		   Scorewin_Struct* ss)
{
#if defined (commentout)
    S_Ncc_Data* udp = (S_Ncc_Data*) options->alg_data;
#endif
    Image* signal = &options->sig;
    Image* pattern = &options->pat;
    Image* score = &options->score;
    int* ip = ss->idx_pt;
    int* zp = ss->sco_pt;
    int* d = options->pat_rect_valid.dims;
    Image_Rect* prc = &options->pat_rect_valid;
    double sd, pd;
    const double *pp, *sp;
    int y, x;
    int sig_pt[2];

    /* Compute means */
    double p_mean = 0, s_mean = 0;
    double num_pix = d[0] * d[1];

    sig_pt[0] = options->sig_rect_valid.pmin[0] + ip[0];
    sig_pt[1] = options->sig_rect_valid.pmin[1] + ip[1];
    pp = image_data(pattern) + image_index_pt (pattern->dims, prc->pmin);
    sp = image_data(signal) + image_index_pt (signal->dims, sig_pt);

    for (y = 0; y < prc->dims[0]; y++) {
	for (x = 0; x < prc->dims[1]; x++) {
	    p_mean += *pp;
	    s_mean += *sp;
	    pp++; sp++;
	}
	pp += pattern->dims[1] - prc->dims[1];
	sp += signal->dims[1] - prc->dims[1];
    }

    p_mean /= num_pix;
    s_mean /= num_pix;

    /* Calculate standard deviations & cc */
    pp = image_data(pattern) + image_index_pt (pattern->dims, prc->pmin);
    sp = image_data(signal) + image_index_pt (signal->dims, sig_pt);
    double p_var = 0, s_var = 0, cc = 0;
    for (y = 0; y < prc->dims[0]; y++) {
	for (x = 0; x < prc->dims[1]; x++) {
	    sd = (*sp - s_mean);
	    s_var += sd * sd;
	    pd = (*pp - p_mean);
	    p_var += pd * pd;
	    cc += sd * pd;
	    pp++; sp++;
	}
	pp += pattern->dims[1] - prc->dims[1];
	sp += signal->dims[1] - prc->dims[1];
    }
    double pat_std_dev = (double) sqrt (p_var);
    double sig_std_dev = (double) sqrt (s_var);

    if (pat_std_dev < options->std_dev_threshold) {
	return;
    }
    if (sig_std_dev < options->std_dev_threshold) {
	return;
    }
    cc /= pat_std_dev * sig_std_dev;

    image_data(score)[image_index_pt(score->dims,zp)] = cc;
}

#if defined (commentout)
/* Don't delete this code -- it does the partially overlapping stuff correctly */
static inline void
s_ncc_score_point (FATM_Options* options,
		   Scorewin_Struct* ss)
{
    S_Ncc_Data* udp = (S_Ncc_Data*) options->alg_data;
    Image* signal = &options->sig;
    Image* pattern = &options->pat;
    Image* score = &options->score;
    Image_Rect* cur_pwin = &ss->cur_pwin;
    Image_Rect* cur_swin = &ss->cur_swin;

    double sd, pd;
    const double *pp, *sp;
    int y, x;

    /* Compute means */
    double p_mean = 0, s_mean = 0;
    double num_pix = cur_swin->dims[0] * cur_swin->dims[1];
    pp = image_data(pattern) + image_index_pt (pattern->dims, cur_pwin->pmin);
    sp = image_data(signal) + image_index_pt (signal->dims, cur_swin->pmin);

    for (y = 0; y < cur_swin->dims[0]; y++) {
	for (x = 0; x < cur_swin->dims[1]; x++) {
	    p_mean += *pp;
	    s_mean += *sp;
	    pp++; sp++;
	}
	pp += pattern->dims[1] - cur_pwin->dims[1];
	sp += signal->dims[1] - cur_swin->dims[1];
    }

    p_mean /= num_pix;
    s_mean /= num_pix;

    /* Calculate standard deviations & cc */
    pp = image_data(pattern) + image_index_pt (pattern->dims, cur_pwin->pmin);
    sp = image_data(signal) + image_index_pt (signal->dims, cur_swin->pmin);
    double p_var = 0, s_var = 0, cc = 0;
    for (y = 0; y < cur_swin->dims[0]; y++) {
	for (x = 0; x < cur_swin->dims[1]; x++) {
	    sd = (*sp - s_mean);
	    s_var += sd * sd;
	    pd = (*pp - p_mean);
	    p_var += pd * pd;
	    cc += sd * pd;
	    pp++; sp++;
	}
	pp += pattern->dims[1] - cur_pwin->dims[1];
	sp += signal->dims[1] - cur_swin->dims[1];
    }
    double pat_std_dev = (double) sqrt (p_var);
    double sig_std_dev = (double) sqrt (s_var);

    if (pat_std_dev < options->std_dev_threshold) {
	return;
    }
    if (sig_std_dev < options->std_dev_threshold) {
	return;
    }
    cc /= pat_std_dev * sig_std_dev;

    image_data(score)[image_index_pt(score->dims,ss->p)] = cc;
}
#endif

void
s_ncc_free (FATM_Options* options)
{
}
