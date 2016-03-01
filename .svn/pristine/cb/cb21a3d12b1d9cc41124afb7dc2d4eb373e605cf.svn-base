/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef SCOREWIN_2_H
#define SCOREWIN_2_H

#include "image.h"

typedef struct score_win_struct {

    /* Initialization function.  Called at beginning. */
    void (*initialize) 
	(struct score_win_struct* sws,
	 Image* pattern,
	 Image* pat_mask,
	 Image* signal,
	 Image* sig_mask,
	 Image_Rect& pat_window,
	 Image_Rect& sig_window);

    /* Scoring function.  Called for each window overlay. */
    void (*score_point)
	(struct score_win_struct* sws,
	 Image* pattern,
	 Image* pat_mask,
	 Image* signal,
	 Image* sig_mask,
	 Image_Rect& cur_pwin,
	 Image_Rect& cur_swin,
	 int* p);

    /* Cleanup function.  Called at end. */
    void (*cleanup) (struct score_win_struct* sws);

    /* Output image */
    Image* score;

    /* User defined, different for each algorithm */
    void* user_data;

} Score_Win;


void scorewin_2 (Image* pattern,
	       Image* pat_mask,
	       Image* signal,
	       Image* sig_mask,
	       Image_Rect &pat_window,
	       Image_Rect &sig_window,
	       Score_Win* sw);

#endif
