/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   s_ncc_fft:  NCC using FFT for correlation term.
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fatm.h"
#include "integral_img.h"
#include "scorewin.h"
#include "s_utils.h"
#include "s_ncc_fft.h"

static void s_ncc_fft_scorewin_alloc (FATM_Options* fopt);
static void s_ncc_fft_scorewin_initialize (FATM_Options* fopt);
static void dump_fft (fftw_complex* fft, int nx, int ny, char* fn);
static void dump_txt (double* img, int nx, int ny, char* fn);

/* =======================================================================*
    Public Functions
 * =======================================================================*/
void
s_ncc_fft_compile (FATM_Options* fopt)
{
    fftw_plan pat_plan;
    double* temp;
    int i, j;
    Image_Rect* prv = &fopt->pat_rect_valid;
    fftw_iodim fftw_dims[2];
    int fft_nx = fopt->sig_rect_scan.dims[1];    /* In fftw3, nx is rows */
    int fft_ny = fopt->sig_rect_scan.dims[0];    /* In fftw3, ny is cols */

    /* Allocate memory */
    S_Ncc_Fft_Data* udp = (S_Ncc_Fft_Data*) malloc (sizeof(S_Ncc_Fft_Data));
    fopt->alg_data = (void*) udp;

    /* Alloc memory for integral images */
    s_ncc_fft_scorewin_alloc (fopt);

    /* Compute pattern statistics */
//    s_pattern_statistics (&udp->p_stats, fopt);

    /* Alloc memory for fft of pat */
    udp->pat_fft = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) * fft_nx * (fft_ny/2+1));
    memset (udp->pat_fft, 0, sizeof(fftw_complex) * fft_nx * (fft_ny/2+1));

    /* Copy pattern into fft memory.  Flip it so that convolution 
	becomes correlation */
    temp = (double*) udp->pat_fft + (fft_nx-1) * (2*(fft_ny/2+1)) + fft_ny - 1;
    for (j = 0; j < prv->dims[0]; j++) {
	for (i = 0; i < prv->dims[1]; i++) {
	    *temp-- = image_data(&fopt->pat)[image_index(prv->dims, j, i)];
	}
	temp -= (2*(fft_ny/2+1)) - prv->dims[1];
    }

    /* Peform fft */
    pat_plan = fftw_plan_dft_r2c_2d (fft_nx, fft_ny, 
	(double*) udp->pat_fft, udp->pat_fft, FFTW_ESTIMATE);
    fftw_execute (pat_plan);
    fftw_destroy_plan (pat_plan);

    /* Debugging info */
    dump_fft (udp->pat_fft, fft_nx, fft_ny, "pat_fft.txt");

    /* Alloc memory for fft of sig */
    udp->sig_fft = (fftw_complex*) fftw_malloc (sizeof(fftw_complex) 
						* fft_nx * (fft_ny/2+1));

    /* Create plan for sig -> sig_fft */
    fftw_dims[0].n = fft_nx;
    fftw_dims[0].is = fopt->sig.dims[0];
    fftw_dims[0].os = (fft_ny/2+1);
    fftw_dims[1].n = fft_ny;
    fftw_dims[1].is = 1;
    fftw_dims[1].os = 1;

    /* NOTE: Using FFTW_MEASURE overwrites input.  So I need to allocate 
	a temporary array. */
    udp->sig_fftw3_plan = fftw_plan_guru_dft_r2c (
	2, fftw_dims, 0, 0, 
	(double*) fopt->sig.data, udp->sig_fft, 
	FFTW_ESTIMATE | FFTW_UNALIGNED | FFTW_PRESERVE_INPUT);
    if (udp->sig_fftw3_plan == 0) {
	printf ("Error: couldn't make plan\n");
    }
    printf ("SRS: %d %d\n", fopt->sig_rect_scan.dims[0], fopt->sig_rect_scan.dims[1]);
    printf ("SIG: %d %d\n", fopt->sig.dims[0], fopt->sig.dims[1]);

    /* Alloc memory for temporary score */
    udp->padded_score = (double*) fftw_malloc (sizeof(double) * fft_nx * fft_ny);

    /* Create plan for pat_fft * sig_fft -> score */
    udp->sco_fftw3_plan = fftw_plan_dft_c2r_2d (fft_nx, fft_ny, 
	udp->sig_fft, udp->padded_score, FFTW_MEASURE);
    if (udp->sco_fftw3_plan == 0) {
	printf ("Error: couldn't make plan\n");
    }
}

void
s_ncc_fft_run (FATM_Options* fopt)
{
    S_Ncc_Fft_Data* udp = (S_Ncc_Fft_Data*) fopt->alg_data;
    int fft_nx = fopt->sig_rect_scan.dims[1];    /* In fftw3, nx is rows */
    int fft_ny = fopt->sig_rect_scan.dims[0];    /* In fftw3, ny is cols */
    int fftw_size = fft_nx * (fft_ny/2+1);
    int i;

    /* Make integral images, etc. */
    s_ncc_fft_scorewin_initialize (fopt);

    /* Take fft of signal */
    fftw_execute_dft_r2c (udp->sig_fftw3_plan, 
	(double*) fopt->sig.data, udp->sig_fft);

    /* Debugging info */
    dump_fft (udp->sig_fft, fft_nx, fft_ny, "sig_fft.txt");

    /* Multiply fft of signal by fft of pattern */
    for (i = 0; i < fftw_size; i++) {
	double re = udp->sig_fft[i][0] * udp->pat_fft[i][0] 
		    - udp->sig_fft[i][1] * udp->pat_fft[i][1];
	double im = udp->sig_fft[i][0] * udp->pat_fft[i][1] 
		    + udp->sig_fft[i][1] * udp->pat_fft[i][0];
	udp->sig_fft[i][0] = re;
	udp->sig_fft[i][1] = im;
    }

    /* Debugging info */
    dump_fft (udp->sig_fft, fft_nx, fft_ny, "sco_fft.txt");

    /* Take ifft of signal */
    fftw_execute_dft_c2r (udp->sco_fftw3_plan, 
	udp->sig_fft, udp->padded_score);

    dump_txt (udp->padded_score, fft_nx, fft_ny, "sco_ifftd.txt");

}

void
s_ncc_fft_free (FATM_Options* fopt)
{
    S_Ncc_Fft_Data* udp = (S_Ncc_Fft_Data*) fopt->alg_data;

//    fftw_destroy_plan (udp->fftw3_plan);
    fftw_free (udp->pat_fft);
}

/* =======================================================================*
    Private Functions
 * =======================================================================*/
static void
s_ncc_fft_scorewin_alloc (FATM_Options* fopt)
{
    S_Ncc_Fft_Data* udp = (S_Ncc_Fft_Data*) fopt->alg_data;
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
s_ncc_fft_scorewin_initialize (FATM_Options* fopt)
{
    S_Ncc_Fft_Data* udp = (S_Ncc_Fft_Data*) fopt->alg_data;

    integral_image_compute (&udp->integral_image,
			    &udp->integral_sq_image, 
			    &fopt->sig,
			    &fopt->sig_rect_valid);
}

static void
dump_fft (fftw_complex* fft, int nx, int ny, char* fn)
{
    FILE* fp = fopen (fn, "wt");
    int i, j;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < (ny/2+1); j++) {
	    fprintf (fp, "%g %g ", fft[i*(ny/2+1)+j][0], fft[i*(ny/2+1)+j][1]);
        }
	fprintf (fp, "\n");
    }
    fclose (fp);
}

static void
dump_txt (double* img, int nx, int ny, char* fn)
{
    FILE* fp = fopen (fn, "wt");
    int i, j, p;

    p = 0;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
	    fprintf (fp, "%g ", img[p++]);
        }
	fprintf (fp, "\n");
    }
    fclose (fp);
}
