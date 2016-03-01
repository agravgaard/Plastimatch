/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#define PLM_CUDA_COMPILE 1
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "demons_misc.h"

/* This function creates the smoothing kernel */
float*
create_ker (float coeff, int half_width)
{
    int i,j=0;
    float sum = 0.0;
    int width = 2*half_width + 1;

    float* ker = (float*) malloc (sizeof(float) * width);
    if (!ker) {
	printf("Allocation failed 5.....Exiting\n");
	exit(-1);
    }

    for (i = -half_width, j = 0; i <= half_width; i++, j++) {
	ker[j] = exp((((float)(-(i*i)))/(2*coeff*coeff)));
	sum = sum + ker[j];
    }

    for (i = 0; i < width; i++) {
	ker[i] = ker[i] / sum;
    }

    return ker;
}

void
validate_filter_widths (int *fw_out, int *fw_in)
{
    int i;

    for (i = 0; i < 3; i++) {
	if (fw_in[i] < 3) {
	    fw_out[i] = 3;
	} else {
	    fw_out[i] = 2 * (fw_in[i] / 2) + 1;
	}
    }
}

void
kernel_stats (float* kerx, float* kery, float* kerz, int fw[])
{
    int i;

    printf ("kerx: ");
    for (i = 0; i < fw[0]; i++) {
	printf ("%.10f ", kerx[i]);
    }
    printf ("\n");
    printf ("kery: ");
    for (i = 0; i < fw[1]; i++) {
	printf ("%.10f ", kery[i]);
    }
    printf ("\n");
    printf ("kerz: ");
    for (i = 0; i < fw[2]; i++) {
	printf ("%.10f ", kerz[i]);
    }
    printf ("\n");
}
