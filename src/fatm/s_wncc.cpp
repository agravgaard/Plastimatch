/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <math.h>
#include "scorewin.h"
#include "s_wncc.h"

/* ** Weighted Normalized Cross Correlation ** */

void
s_wncc_initialize (struct score_win_struct* sws,
		   Image* pattern,
		   Image* pat_mask,
		   Image* signal,
		   Image* sig_mask,
		   Image_Rect& pat_window,
		   Image_Rect& sig_window)
{
    /* Initialize to zero so that skipped locations will 
       show no correlation.  */
    memset ((void *) sws->score->data, 0, image_bytes(sws->score));
}

/* -------------------------------------------------------------------
 * GCS: May 6, 2004.  Updated to use combined mask for weighting.
 * Note: size of swin and pwin must be the same. */
void
s_wncc_score_point (struct score_win_struct* sws,
		    Image* pattern,
		    Image* pat_mask,
		    Image* signal,
		    Image* sig_mask,
		    Image_Rect& cur_pwin,
		    Image_Rect& cur_swin,
		    int* p
		    )
{
    S_Wncc_Data* udp = (S_Wncc_Data*) sws->user_data;
    //const ImageDims &pdims = ImageDims(pattern->rows,pattern->cols);
    //const ImageDims &sdims = ImageDims(signal->rows,signal->cols);

    double sd, pd;
    const double *ps, *pw, *ss, *sw;
    double *u, *umask;
    int y, x;

    /* Allocate user data */
    // sws->user_data = (void*) udp;

    /* Allocate memory for unified mask */
    umask = (double*) malloc(sizeof(double)*cur_swin.dims[0]*cur_swin.dims[1]);

    /* Unify the mask & compute means */
    double p_mean = 0, s_mean = 0, uweight_sum = 0;
    ps = image_data(pattern) + image_index (pattern->dims, cur_pwin.pmin);
    pw = image_data(pat_mask) + image_index (pattern->dims, cur_pwin.pmin);
    ss = image_data(signal) + image_index (signal->dims, cur_swin.pmin);
    sw = image_data(sig_mask) + image_index (signal->dims, cur_swin.pmin);
    u = umask;

    for (y = 0; y < cur_swin.dims[0]; y++) {
	for (x = 0; x < cur_swin.dims[1]; x++) {
	    double this_weight = (*pw) * (*sw);
	    *u++ = this_weight;
	    p_mean += *ps * this_weight;
	    s_mean += *ss * this_weight;
	    uweight_sum += this_weight;
	    ps++; pw++; ss++; sw++;
	}
	ps += pattern->dims[1] - cur_pwin.dims[1];
	pw += pattern->dims[1] - cur_pwin.dims[1];
	ss += signal->dims[1] - cur_swin.dims[1];
	sw += signal->dims[1] - cur_swin.dims[1];
    }
    if (uweight_sum < udp->weight_threshold) {
	free(umask);
	return;
    }

    p_mean /= uweight_sum;
    s_mean /= uweight_sum;
    
    /* Calculate standard deviations & cc */
    ps = image_data(pattern) + image_index (pattern->dims, cur_pwin.pmin);
    ss = image_data(signal) + image_index (signal->dims, cur_swin.pmin);
    u = umask;
    double p_var = 0, s_var = 0, cc = 0;
    for (y = 0; y < cur_swin.dims[0]; y++) {
	for (x = 0; x < cur_swin.dims[1]; x++) {
	    double this_weight = *u++;
	    sd = this_weight * (*ss - s_mean);
	    s_var += sd * sd;
	    pd = this_weight * (*ps - p_mean);
	    p_var += pd * pd;
	    cc += sd * pd;
	    ps++; ss++;
	}
	ps += pattern->dims[1] - cur_pwin.dims[1];
	ss += signal->dims[1] - cur_swin.dims[1];
    }
    double pat_std_dev = (double) sqrt (p_var);
    double sig_std_dev = (double) sqrt (s_var);

    free (umask);
    if (pat_std_dev < udp->std_dev_threshold) {
	return;
    }
    if (sig_std_dev < udp->std_dev_threshold) {
	return;
    }
    cc /= pat_std_dev * sig_std_dev;

    image_data(sws->score)[image_index(sws->score->dims,p)] = cc;
}
