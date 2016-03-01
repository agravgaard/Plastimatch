/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   s_rssd:  Naive implementation of robust sum of squared difference
   ----------------------------------------------------------------------- */
#include <string.h>
#include <math.h>
#include "scorewin.h"
#include "s_rssd.h"

static inline void s_rssd_score_point_1 (FATM_Options* options,
					 Scorewin_Struct* ss);

void
s_rssd_scorewin_1 (FATM_Options* options)
{
#if defined (commentout)
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
    //mexPrintf ("---LOOPING: %d -> %d\n", 0, zv->dims[1]);
    for (ip[1] = 0, zp[1] = zv->pmin[1]; ip[1] < zv->dims[1]; ip[1]++, zp[1]++) {
	for (ip[0] = 0, zp[0] = zv->pmin[0]; ip[0] < zv->dims[0]; ip[0]++, zp[0]++) {
	    //mexPrintf ("LOOP: %d %d\n", ip[0], ip[1]);
	    s_rssd_score_point_1 (options, &ss);
	}
    }
}

void
s_rssd_run (FATM_Options* options)
{
    /* Initialize to zero. Skipped locations will show no correlation. */
    memset ((void *) options->score.data, 0, image_bytes(&options->score));

    /* Iterate through window */
    s_rssd_scorewin_1 (options);
}

static inline void
s_rssd_score_point_1 (FATM_Options* options,
		   Scorewin_Struct* ss)
{
#if defined (commentout)
    S_Rssd_Data* udp = (S_Rssd_Data*) options->alg_data;
#endif
    Image* signal = &options->sig;
    Image* pattern = &options->pat;
    Image* score = &options->score;
    int* ip = ss->idx_pt;
    int* zp = ss->sco_pt;
    int* prvdims = options->pat_rect_valid.dims;
    Image_Rect* prc = &options->pat_rect_valid;
    const double *pp, *sp;
    int y, x;
    int sig_pt[2];

    /* Compute means */
    double num_pix = prvdims[0] * prvdims[1];

    sig_pt[0] = options->sig_rect_valid.pmin[0] + ip[0];
    sig_pt[1] = options->sig_rect_valid.pmin[1] + ip[1];
    pp = image_data(pattern) + image_index_pt (pattern->dims, prc->pmin);
    sp = image_data(signal) + image_index_pt (signal->dims, sig_pt);

    /* Loop through pixels in overlapping region */
    pp = image_data(pattern) + image_index_pt (pattern->dims, prc->pmin);
    sp = image_data(signal) + image_index_pt (signal->dims, sig_pt);
    double sc = 0;
    for (y = 0; y < prc->dims[0]; y++) {
	for (x = 0; x < prc->dims[1]; x++) {
	    double d = *sp - *pp;
	    d = d * d;
	    if (d > options->truncated_quadratic_threshold) {
		d = options->truncated_quadratic_threshold;
	    }
	    sc += d;
	    pp++; sp++;
	}
	pp += pattern->dims[1] - prc->dims[1];
	sp += signal->dims[1] - prc->dims[1];
    }

    /* Negative, so that best score at function maximum */
    image_data(score)[image_index_pt(score->dims,zp)] = - sc / num_pix;
}


void
s_rssd_free (FATM_Options* options)
{
}
