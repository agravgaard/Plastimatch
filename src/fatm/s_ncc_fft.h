/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef S_NCC_FFT_H
#define S_NCC_FFT_H

#include "fftw3.h"
#include "fatm.h"
#include "scorewin.h"

typedef struct s_ncc_fft_data {
    Image integral_image;
    Image integral_sq_image;

    /* FFTW3 stuff */
    fftw_complex *pat_fft;
    fftw_complex *sig_fft;
    fftw_plan sig_fftw3_plan;
    double *padded_score;
    fftw_plan sco_fftw3_plan;
} S_Ncc_Fft_Data;

void
s_ncc_fft_initialize (struct score_win_struct* sws,
		      Image* pattern,
		      Image* pat_mask,
		      Image* signal,
		      Image* sig_mask,
		      Image_Rect& pat_window,
		      Image_Rect& sig_window);

void s_ncc_fft_compile (FATM_Options* options);
void s_ncc_fft_run (FATM_Options* options);
void s_ncc_fft_free (FATM_Options* options);
void s_ncc_fft_score_point (FATM_Options* options,
			    Scorewin_Struct* ss);

#endif
