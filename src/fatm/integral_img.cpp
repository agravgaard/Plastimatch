/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "integral_img.h"

void
integral_image_compute (Image *i_img, Image *i2_img, Image *in_img, Image_Rect* in_rect)
{
    int ii_dims[2] = { in_rect->dims[0] + 1, in_rect->dims[1] + 1 };
    const double *sw;
    double *ii, *ii2;
    int x, y;

    /* Compute integral images of signal.  Note that the integral image 
       has an extra row/column of zeros at the top/left. */
    ii = image_data(i_img);
    ii2 = image_data(i2_img);
    for (y = 0; y < ii_dims[0]; y++) {
	ii[y * ii_dims[1]] = 0.0;
	ii2[y * ii_dims[1]] = 0.0;
    }
    for (x = 0; x < ii_dims[1]; x++) {
	ii[x] = 0.0;
	ii2[x] = 0.0;
    }
    ii += image_index (ii_dims, 1, 1);
    ii2 += image_index (ii_dims, 1, 1);
    sw = image_data(in_img) + image_index_pt (in_img->dims, in_rect->pmin);
    for (y = 1; y < ii_dims[0]; y++) {
	for (x = 1; x < ii_dims[1]; x++) {
	    *ii = ii[-1] + ii[-ii_dims[1]] + *sw - ii[-1-ii_dims[1]];
	    *ii2 = ii2[-1] + ii2[-ii_dims[1]] + (*sw) * (*sw) - ii2[-1-ii_dims[1]];
	    ii++; ii2++; sw++;
	}
	ii++; ii2++;
	sw += in_img->dims[1] - in_rect->dims[1];
    }
}

void
line_integral_image_compute (Image *li_img, Image *in_img, Image_Rect* in_rect)
{
    int ii_dims[2] = { in_rect->dims[0] + 1, in_rect->dims[1] + 1 };
    const double *sw;
    double *li;
    int x, y;

    /* Compute integral images of signal.  Note that the integral image 
       has an extra row/column of zeros at the top/left. */
    li = image_data (li_img);
    for (y = 0; y < ii_dims[0]; y++) {
	li[y * ii_dims[1]] = 0.0;
    }
    li += image_index (ii_dims, 1, 1);
    sw = image_data(in_img) + image_index_pt (in_img->dims, in_rect->pmin);
    for (y = 1; y < ii_dims[0]; y++) {
	for (x = 1; x < ii_dims[1]; x++) {
	    *li = li[-1] + *sw;
	    li++; sw++;
	}
	li++;
	sw += in_img->dims[1] - in_rect->dims[1];
    }
}
