/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   Assume: columns (x) are fast-moving, rows (y) are slow moving
   Assume: square pixels
   ----------------------------------------------------------------------- */
#include <math.h>
#include "image.h"
#include "clip_pat.h"

void
weighted_clip_pat_generate (
		Image* image,
		double length,   /* in pixels */
		double width,    /* in pixels */
		double falloff,  /* in pixels */
		double angle,    /* in radians, usu [0,pi) */
		double fc,	 /* foreground color */
		double bc,	 /* background color */
		double w_falloff /* in pixels */
		)
{
    int i;
    Image w_image;

    image_malloc (&w_image, image->dims);
    clip_pat_generate (image, length, width, falloff, angle, fc, bc);
    clip_pat_generate (&w_image, length+falloff, 
	width+falloff, w_falloff, angle, 1.0, 0.0);
    for (i = 0; i < image_size(&w_image); i++) {
	image_data(image)[i] *= image_data(&w_image)[i];
    }
    image_free (&w_image);
}

void
clip_pat_generate (Image* image,
		   double length,   /* in pixels */
		   double width,    /* in pixels */
		   double falloff,  /* in pixels */
		   double angle,    /* in radians, usu [0,pi) */
		   double fc,	    /* foreground color */
		   double bc	    /* background color */
		   )
{
    int i, x, y;
    double icx, icy;       /* Image center */
#if defined (commentout)
    double len_x, len_y;   /* End-point offset (length) */
    double wid_x, wid_y;   /* End-point offset (width) */
#endif
    double costh, sinth;

    length = 0.5 * length;  /* Convert to half-lengths */
    width = 0.5 * width;

    icx = image->dims[0] / 2.0 - 0.5;
    icy = image->dims[1] / 2.0 - 0.5;
    costh = cos (angle);
    sinth = sin (angle);
#if defined (commentout)
    len_x = length * costh;
    len_y = length * sinth;
    wid_x = - width * sinth;
    wid_y = width * costh;
#endif

    for (i = 0, y = 0; y < image->dims[1]; y++) {
        for (x = 0; x < image->dims[0]; x++, i++) {
	    double wcoord, lcoord, dcoord;
	    lcoord = fabs((x-icx) * costh + (icy-y) * sinth) - length;
	    wcoord = fabs((x-icx) * - sinth + (icy-y) * costh);
	    if (lcoord <= 0.0) {
		dcoord = wcoord - width;
	    } else {
		dcoord = sqrt(lcoord * lcoord + wcoord * wcoord) - width;
	    }
	    if (dcoord <= 0.0) {
		image_data(image)[i] = fc;
	    } else if (dcoord <= falloff) {
		image_data(image)[i] = (fc * (falloff - dcoord) + bc * dcoord) / falloff;
	    } else {
		image_data(image)[i] = bc;
	    }
	}
    }
}
