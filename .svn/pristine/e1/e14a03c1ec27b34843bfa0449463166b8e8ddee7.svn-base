/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <math.h>
#include "fatm.h"

void
s_pattern_statistics (Pattern_Stats* p_stats, FATM_Options* options)
{
    int x, y;
    int num_pix;
    double p_sq;
    const double *ps;
#if defined (commentout)
    Image_Rect* prc = &options->pat_rect.pat_rect_clipped;
#endif
    Image_Rect* prc = &options->pat_rect_valid;

    ps = image_data(&options->pat) 
	    + image_index_pt (options->pat.dims, prc->pmin);
    p_stats->p_mean = 0.0;
    num_pix = prc->dims[0] * prc->dims[1];
    p_sq = 0.0;
    p_stats->p_max = *ps;
    p_stats->p_min = *ps;
    for (y = 0; y < prc->dims[0]; y++) {
	for (x = 0; x < prc->dims[1]; x++) {
	    p_stats->p_mean += *ps;
	    p_sq += (*ps) * (*ps);
	    if (*ps > p_stats->p_max) p_stats->p_max = *ps;
	    else if (*ps < p_stats->p_min) p_stats->p_min = *ps;
	    ps++;
	}
	ps += options->pat.dims[1] - prc->dims[1];
    }
    p_stats->p_mean /= num_pix;
    p_stats->p_var = p_sq / num_pix - (p_stats->p_mean) * (p_stats->p_mean);
    p_stats->p_std = sqrt(p_stats->p_var);
}
