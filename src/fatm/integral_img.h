/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef S_INTEGRAL_IMG_H
#define S_INTEGRAL_IMG_H

#include "image.h"

void
integral_image_compute (Image *i_img, Image *i2_img, Image *in_img, Image_Rect* in_rect);
void
line_integral_image_compute (Image *li_img, Image *in_img, Image_Rect* in_rect);

#endif
