/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdlib.h>
#include "plm_config.h"
#include "fatm.h"
#include "timer.h"
#include "s_fancc.h"
#include "s_fncc.h"
#include "s_ncc.h"
#if (FFTW_FOUND)
#include "s_ncc_fft.h"
#endif
#include "s_rssd.h"

void match_initialize_options (FATM_Options *options);
void match_process_free (FATM_Options *options);
void match_process_compile (FATM_Options *options);
void match_process_run (FATM_Options *options);
void match_process_render (FATM_Options *options, double* qpat);
void clip_pattern (FATM_Options* options);
void clip_signal (FATM_Options* options);
void clip_score_rect (FATM_Options *options);

/* =======================================================================*
    Public Functions
 * =======================================================================*/
/*
Score_Rect_Info::Score_Rect_Info ()
{
}

Score_Rect_Info::~Score_Rect_Info ()
{
}
*/
FATM_Options* 
fatm_initialize (void)
{
    FATM_Options* fopt = (FATM_Options*) malloc (sizeof(FATM_Options));
    if (!fopt) return fopt;

    match_initialize_options (fopt);
    return fopt;
}

void
fatm_compile (FATM_Options* fopt)
{
    clip_pattern (fopt);
    clip_signal (fopt);
    clip_score_rect (fopt);
    match_process_compile (fopt);
}

void
fatm_run (FATM_Options* fopt)
{
    match_process_run (fopt);
}

void
fatm_free (FATM_Options* fopt)
{
    free (fopt);
}

/* =======================================================================*
    Private Functions
 * =======================================================================*/
void
clip_pattern (FATM_Options* options)
{
    Image_Rect* pro = &options->pat_rect;
    Image_Rect* prc = &options->pat_rect_valid;

    Image* pat = &options->pat;
    int tmp;

    /* Clip the pattern rect to the edges of the pattern image.  */
    for (int i=0; i<2; i++) {
	prc->pmin[i] = pro->pmin[i];
	prc->dims[i] = pro->dims[i];
	if (pro->pmin[i] < 0) {
	    prc->pmin[i] = 0;
	    prc->dims[i] += pro->pmin[i];
	}
	tmp = pat->dims[i] - (pro->pmin[i] + pro->dims[i]);
	if (tmp < 0) {
	    prc->dims[i] -= tmp;
	}
    }
}

void
clip_signal (FATM_Options* options)
{
    int i;
    Image_Rect* pro = &options->pat_rect;
    Image_Rect* prv = &options->pat_rect_valid;
    Image_Rect* sro = &options->sig_rect;
    Image_Rect* srs = &options->sig_rect_scan;
    Image_Rect* srv = &options->sig_rect_valid;
    Image* sig = &options->sig;
    Image_Rect prd;    /* prd = pat_rect difference, is *added* to signal rect */

    for (i=0; i<2; i++) {
	prd.pmin[i] = prv->pmin[i] - pro->pmin[i];
	prd.dims[i] = prv->dims[i] - pro->dims[i];
    }

    /* Clip the signal rect based on pattern clipping and signal image */
    for (i=0; i<2; i++) {
	int tmp;
	srs->pmin[i] = sro->pmin[i] + prd.pmin[i];
	srs->dims[i] = sro->dims[i] + prd.dims[i];
	srv->pmin[i] = srs->pmin[i];
	srv->dims[i] = srs->dims[i];

	if (srs->pmin[i] < 0) {
	    srv->pmin[i] = 0;
	    srv->dims[i] += srs->pmin[i];
	}
	tmp = sig->dims[i] - (srs->pmin[i] + srs->dims[i]);
	if (tmp < 0) {
	    srv->dims[i] += tmp;
	}
    }
}

void
clip_score_rect (FATM_Options *options)
{
    Image_Rect* srs = &options->sig_rect_scan;
    Image_Rect* srv = &options->sig_rect_valid;
    Image_Rect* prc = &options->pat_rect_valid;
    Image_Rect* zrf = &options->score_rect.score_rect_full;
    Image_Rect* zrv = &options->score_rect.score_rect_valid;

    for (int i=0; i<2; i++) {
	zrf->pmin[i] = 0;
	zrf->dims[i] = srs->dims[i] - prc->dims[i] + 1;
	zrv->pmin[i] = srv->pmin[i] - srs->pmin[i];
	zrv->dims[i] = srv->dims[i] - prc->dims[i] + 1;
    }
    if (zrf->dims[0] <= 0 || zrf->dims[1] <= 0) {
	zrf->dims[0] = 0;
	zrf->dims[1] = 0;
    }
    if (zrv->dims[0] <= 0 || zrv->dims[1] <= 0) {
	zrv->dims[0] = 0;
	zrv->dims[1] = 0;
    }
}

void
match_initialize_options (FATM_Options *options)
{
    options->command = MATCH_COMMAND_RUN;
    options->alg = MATCH_ALGORITHM_FNCC;
    options->alg_data = 0;
    options->std_dev_threshold = MATCH_DEFAULT_STHRESH;
    options->std_dev_expected_min = MATCH_DEFAULT_STHRESH;
    options->wthresh = MATCH_DEFAULT_WTHRESH;
    options->num_partitions = 0;
    options->partition_method = PARTITION_METHOD_AUTOMATIC;
    options->truncated_quadratic_threshold = MATCH_DEFAULT_TQTHRESH;
    image_init (&options->pat);
    image_init (&options->patw);
    image_init (&options->sig);
    image_init (&options->sigw);
    image_init (&options->score);
}

void
match_process_free (FATM_Options *options)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	s_fancc_free (options);
	break;
#if (FFTW_FOUND)
    case MATCH_ALGORITHM_NCC_FFT:
	s_ncc_fft_free (options);
	break;
#endif
    case MATCH_ALGORITHM_FNCC:
	s_fncc_free (options);
	break;
    case MATCH_ALGORITHM_RSSD:
	s_rssd_free (options);
	break;
    default:
	break;
    }
}

void
match_process_compile (FATM_Options *options)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	s_fancc_compile (options);
	break;
#if (FFTW_FOUND)
    case MATCH_ALGORITHM_NCC_FFT:
	s_ncc_fft_compile (options);
	break;
#endif
    case MATCH_ALGORITHM_FNCC:
	s_fncc_compile (options);
	break;
    default:
	break;
    }
}

void
match_process_run (FATM_Options *options)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	if (options->alg_data) {
	    s_fancc_run (options);
	} else {
	    s_fancc_compile (options);
	    s_fancc_run (options);
	    s_fancc_free (options);
	}
	break;
#if (FFTW_FOUND)
    case MATCH_ALGORITHM_NCC_FFT:
	if (options->alg_data) {
	    s_ncc_fft_run (options);
	} else {
	    s_ncc_fft_compile (options);
	    s_ncc_fft_run (options);
	    s_ncc_fft_free (options);
	}
	break;
#endif
    case MATCH_ALGORITHM_NCC:
	s_ncc_run (options);
	break;
    case MATCH_ALGORITHM_FNCC:
	if (options->alg_data) {
	    s_fncc_run (options);
	} else {
	    s_fncc_compile (options);
	    s_fncc_run (options);
	    s_fncc_free (options);
	}
	break;
    case MATCH_ALGORITHM_RSSD:
	s_rssd_run (options);
	break;
    default:
	break;
    }
}

void
match_process_render (FATM_Options *options, double* qpat)
{
    switch (options->alg) {
    case MATCH_ALGORITHM_FANCC:
	s_fancc_render (options, qpat);
	break;
    default:
	break;
    }
}
