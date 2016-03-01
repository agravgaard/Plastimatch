/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "image.h"

Image_Rect::Image_Rect () {
    this->data = 0;
}

Image_Rect::~Image_Rect () {
    delete data;
}

void
Image_Rect::set_dims (int dims[2]) {
    this->dims[0] = dims[0];
    this->dims[1] = dims[1];
    this->data = new unsigned short[dims[0] * dims[1]];
}

