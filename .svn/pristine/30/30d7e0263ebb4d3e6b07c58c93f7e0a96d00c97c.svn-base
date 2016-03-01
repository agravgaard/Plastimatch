/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   s_fncc: Fast implementation of (unweighted) normalized cross correlation
   ----------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include "mex.h"
#include "fatm.h"
#include "integral_img.h"
#include "scorewin.h"
#include "s_utils.h"
#include "s_fncc.h"

static void s_fncc_scorewin_initialize (FATM_Options* fopt);
static void s_fncc_scorewin_free (FATM_Options* fopt);
static void s_fncc_scorewin_alloc (FATM_Options* fopt);
static void s_fncc_score_point (FATM_Options* fopt,
				Scorewin_Struct* ss);

void
s_fncc_compile (FATM_Options* fopt)
{
    /* Allocate memory */
    S_Fncc_Data* udp = (S_Fncc_Data*) malloc (sizeof(S_Fncc_Data));
    fopt->alg_data = (void*) udp;

    /* Alloc more memory */
    s_fncc_scorewin_alloc (fopt);

    /* Compute pattern statistics */
    s_pattern_statistics (&udp->p_stats, fopt);
}

void
s_fncc_free (FATM_Options* fopt)
{
    S_Fncc_Data* udp = (S_Fncc_Data*) fopt->alg_data;
    s_fncc_scorewin_free (fopt);
    free (udp);
}

static void
s_fncc_scorewin_initialize (FATM_Options* fopt)
{
    S_Fncc_Data* udp = (S_Fncc_Data*) fopt->alg_data;

    integral_image_compute (&udp->integral_image, &udp->integral_sq_image, 
			    &fopt->sig, &fopt->sig_rect_valid);
}

static void
s_fncc_scorewin_1 (FATM_Options* fopt)
{
#if defined (commentout)
    Image_Rect* prc = &fopt->pat_rect.pat_rect_clipped;
    Image_Rect* prc = &fopt->pat_rect_valid;
    Image* pat = &fopt->pat;
    Image* sig = &fopt->sig;
    Image* score = &fopt->score;
#endif
    Image_Rect* zv = &fopt->score_rect.score_rect_valid;
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
    for (ip[1] = 0, zp[1] = zv->pmin[1]; ip[1] < zv->dims[1]; ip[1]++, zp[1]++) {
	for (ip[0] = 0, zp[0] = zv->pmin[0]; ip[0] < zv->dims[0]; ip[0]++, zp[0]++) {
	    //mexPrintf ("LOOP: %d %d\n", ip[0], ip[1]);
	    s_fncc_score_point (fopt, &ss);
	}
    }

#if defined (commentout)
    QueryPerformanceCounter(&clock_count);
    timestamp2 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
    mexPrintf ("Run: %g\n", timestamp2 - timestamp1);
#endif
}

void
s_fncc_run (FATM_Options* fopt)
{
#if defined (commentout)
    S_Fncc_Data* udp = (S_Fncc_Data*) fopt->alg_data;
#endif

    /* Initialize to zero. Skipped locations will show no correlation. */
    memset ((void *) fopt->score.data, 0, image_bytes(&fopt->score));

    /* Make integral images, etc. */
    s_fncc_scorewin_initialize (fopt);

    /* Iterate through window */
    fopt->match_partials = 1;
    s_fncc_scorewin_1 (fopt);
}

static void
s_fncc_score_point (FATM_Options* fopt,
		     Scorewin_Struct* ss)
{
    S_Fncc_Data* udp = (S_Fncc_Data*) fopt->alg_data;
    Pattern_Stats* p_stats = &udp->p_stats;
    Image* signal = &fopt->sig;
    Image* pattern = &fopt->pat;
    Image* score = &fopt->score;
    double sd, pd;
    const double *ii;
    const double* ii2;
    const double *pp, *sp;
    int y, x;
    int* ip = ss->idx_pt;
    int* zp = ss->sco_pt;
    int* d = fopt->pat_rect_valid.dims;
#if defined (commentout)
    Image_Rect* prc = &fopt->pat_rect.pat_rect_clipped;
#endif
    Image_Rect* prc = &fopt->pat_rect_valid;
    int p_ul[2] = { ip[0], ip[1] };
    int p_ur[2] = { ip[0], ip[1] + d[1] };
    int p_ll[2] = { ip[0] + d[0], ip[1] };
    int p_lr[2] = { ip[0] + d[0], ip[1] + d[1] };
    int sig_pt[2];

    /* Compute window means */
    double s_mean = 0;
    double num_pix = d[0] * d[1];
    double p_mean = p_stats->p_mean;
    ii = image_data(&udp->integral_image);
    s_mean = ii[image_index_pt(udp->integral_image.dims,p_ul)]
	- ii[image_index_pt(udp->integral_image.dims,p_ur)]
	- ii[image_index_pt(udp->integral_image.dims,p_ll)]
	+ ii[image_index_pt(udp->integral_image.dims,p_lr)];
    s_mean /= num_pix;

    /* Calculate standard deviations & cc */
    sig_pt[0] = fopt->sig_rect_valid.pmin[0] + ip[0];
    sig_pt[1] = fopt->sig_rect_valid.pmin[1] + ip[1];
    pp = image_data(pattern) + image_index_pt (pattern->dims, prc->pmin);
    sp = image_data(signal) + image_index_pt (signal->dims, sig_pt);
    double p_var = 0, s_var = 0, cc = 0;
    for (y = 0; y < prc->dims[0]; y++) {
	for (x = 0; x < prc->dims[1]; x++) {
	    /* GCS: Note, we don't have to use (s - s_mean) here, 
	            because if p is zero mean, then 
			sum((s - smean) * p)
			    = sum(s*p) - smean*sum(p)
			    = sum(s*p)
	     */
	    //sd = (*sp - s_mean);
	    sd = *sp;
	    pd = (*pp - p_mean);
	    //pd = *ps;
	    cc += sd * pd;
	    pp++; sp++;
	}
	pp += pattern->dims[1] - prc->dims[1];
	sp += signal->dims[1] - prc->dims[1];
    }
    
    ii2 = image_data(&udp->integral_sq_image);
    s_var = ii2[image_index_pt(udp->integral_sq_image.dims,p_ul)]
	- ii2[image_index_pt(udp->integral_sq_image.dims,p_ur)]
	- ii2[image_index_pt(udp->integral_sq_image.dims,p_ll)]
	+ ii2[image_index_pt(udp->integral_sq_image.dims,p_lr)];
    s_var -= s_mean * s_mean * num_pix;

    p_var = num_pix * p_stats->p_var;
    double pat_std_dev = (double) sqrt (p_var);
    double sig_std_dev = (double) sqrt (s_var);
    double snp = sqrt(num_pix);

    if (sig_std_dev < fopt->std_dev_threshold * snp) {
	return;
    } else if (sig_std_dev/sqrt(num_pix) < fopt->std_dev_expected_min * snp) {
	sig_std_dev = fopt->std_dev_expected_min * snp;
    }
    cc /= pat_std_dev * sig_std_dev;

    image_data(score)[image_index_pt(score->dims,zp)] = cc;
}

static void
s_fncc_scorewin_alloc (FATM_Options* fopt)
{
    S_Fncc_Data* udp = (S_Fncc_Data*) fopt->alg_data;
    //int* sw_dims = fopt->sig_rect.dims;
    //int* pw_dims = fopt->pat_rect.dims;
    Image_Rect* srv = &fopt->sig_rect_valid;
    int ii_dims[2] = { srv->dims[0] + 1, srv->dims[1] + 1 };

    /* Compute integral images of signal.  Note that the integral image 
       has an extra row/column of zeros at the top/left. */
    image_malloc (&udp->integral_image, ii_dims);
    image_malloc (&udp->integral_sq_image, ii_dims);
}

static void
s_fncc_scorewin_free (FATM_Options* fopt)
{
    S_Fncc_Data* udp = (S_Fncc_Data*) fopt->alg_data;
    free (udp->integral_image.data);
    free (udp->integral_sq_image.data);
}
