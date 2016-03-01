/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef S_RSSD_H
#define S_RSSD_H

#include "fatm.h"
#include "scorewin.h"

typedef struct s_rssd_data {
    double truncated_quadratic_threshold;
} S_Rssd_Data;


void
s_rssd_initialize (struct score_win_struct* sws,
		   Image* pattern,
		   Image* pat_mask,
		   Image* signal,
		   Image* sig_mask,
		   Image_Rect& pat_window,
		   Image_Rect& sig_window);

void s_rssd_run (FATM_Options* options);
void s_rssd_free (FATM_Options* options);
void s_rssd_score_point (FATM_Options* options,
			Scorewin_Struct* ss);

#endif
