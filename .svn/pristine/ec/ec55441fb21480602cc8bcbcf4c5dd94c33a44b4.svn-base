/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   There are two basic strategies for scoring the partial overlapped 
   area of a window.

   1) Give a score of 0 (or -1) for regions of partial overlap.
   2) Score based on the portions that overlap.

   Strategy 1 is called "clip_partials", and strategy 2 is called 
   "match_partials".  Strategy 2 is preferred, esp for large templates,
   but only strategy 1 is available for fancc.

   Normally we would clip the pattern window, but for decoupled compile
   and run that isn't appropriate.
   ----------------------------------------------------------------------- */
#include <math.h>
#include "fatm.h"
#include "shared.h"
#include "scorewin.h"
#include "mex.h"
#include <windows.h>

#if defined (commentout)
void
scorewin_clip_pattern (FATM_Options *options)
{
    Image* pat = &options->pat;
    Image_Rect* pr = &options->pat_rect;
    Image_Rect* sr = &options->sig_rect;

    /* Clip the pattern window and adjust the signal window accordingly.  */
    for (int i=0; i<2; i++) {
	if (pr->pmin[i] < 0) {
	    sr->pmin[i] -= pr->pmin[i];
	    pr->dims[i] += pr->pmin[i];
	    pr->pmin[i] = 0;
	}
	if (pr->pmin[i] + pr->dims[i] > pat->dims[i]) {
	    pr->dims[i] = pat->dims[i] - pr->pmin[i];
	}
    }
}
#endif

inline void
scorewin_match_partials (FATM__Options* options, Score_Function score_fn)
{
    Image_Rect* pr = &options->pat_rect;
    Image_Rect* sr = &options->sig_rect;
    Image* pat = &options->pat;
    Image* sig = &options->sig;
    Image* score = &options->score;
    Scorewin_Struct ss;

#if defined (commentout)
    double timestamp1, timestamp2;
    LARGE_INTEGER clock_count;
    LARGE_INTEGER clock_freq;
    QueryPerformanceFrequency (&clock_freq);
    QueryPerformanceCounter(&clock_count);
    timestamp1 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
#endif

    /* Iterate through each point on the output image. 
     * Dim 1 is the major ordering (i.e. columns for column-major 
     * for matlab images). */
    for (ss.p[1] = 0; ss.p[1] < score->dims[1]; ss.p[1]++) {
	for (ss.p[0] = 0; ss.p[0] < score->dims[0]; ss.p[0]++) {
	    /* Calculate the current pattern and signal windows.  */
	    ss.cur_swin.pmin[0] = sr->pmin[0] + ss.p[0];
	    ss.cur_swin.pmin[1] = sr->pmin[1] + ss.p[1];
	    ss.cur_pwin = (*pr);

	    if (ss.cur_swin.pmin[0] < 0) {
		ss.cur_pwin.pmin[0] -= ss.cur_swin.pmin[0];
		ss.cur_pwin.dims[0] += ss.cur_swin.pmin[0];
		ss.cur_swin.pmin[0] = 0;
	    }
	    if (ss.cur_swin.pmin[1] < 0) {
		ss.cur_pwin.pmin[1] -= ss.cur_swin.pmin[1];
		ss.cur_pwin.dims[1] += ss.cur_swin.pmin[1];
		ss.cur_swin.pmin[1] = 0;
	    }
	    ss.cur_swin.dims[0] = ss.cur_pwin.dims[0];
	    ss.cur_swin.dims[1] = ss.cur_pwin.dims[1];
	    if (ss.cur_swin.pmin[0] + ss.cur_swin.dims[0] > sig->dims[0])
		ss.cur_swin.dims[0] = sig->dims[0] - ss.cur_swin.pmin[0];
	    if (ss.cur_swin.pmin[1] + ss.cur_swin.dims[1] > sig->dims[1])
		ss.cur_swin.dims[1] = sig->dims[1] - ss.cur_swin.pmin[1];
	    ss.cur_pwin.dims[0] = ss.cur_swin.dims[0];
	    ss.cur_pwin.dims[1] = ss.cur_swin.dims[1];

	    score_fn (options, &ss);

#if defined (commentout)
	    sw->score_point (sw,
			     pat,
			     pat_mask,
			     sig,
			     sig_mask,
			     cur_pwin,
			     cur_swin,
			     pt);
#endif
	}
    }
#if defined (commentout)
    QueryPerformanceCounter(&clock_count);
    timestamp2 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
    mexPrintf ("Run: %g\n", timestamp2 - timestamp1);
#endif
}

void
scorewin_clip_partials (FATM_Options* options, Score_Function score_fn)
{
}

void
scorewin (FATM_Options* options, Score_Function score_fn)
{
#if defined (commentout)
    if (options->match_partials) {
	scorewin_match_partials (options, score_fn);
    } else {
	scorewin_clip_partials (options, score_fn);
    }
#endif
    scorewin_match_partials (options, score_fn);
}
