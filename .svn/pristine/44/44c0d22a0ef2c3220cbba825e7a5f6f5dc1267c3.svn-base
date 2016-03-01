/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef S_WNCC_H
#define S_WNCC_H

typedef struct s_wncc_data {
    double weight_threshold;
    double std_dev_threshold;
} S_Wncc_Data;


void
s_wncc_initialize (struct score_win_struct* sws,
		   Image* pattern,
		   Image* pat_mask,
		   Image* signal,
		   Image* sig_mask,
		   Image_Rect& pat_window,
		   Image_Rect& sig_window);

void
s_wncc_score_point (struct score_win_struct* sws,
		    Image* pattern,
		    Image* pat_mask,
		    Image* signal,
		    Image* sig_mask,
		    Image_Rect& cur_pwin,
		    Image_Rect& cur_swin,
		    int* p
		    );

#endif
