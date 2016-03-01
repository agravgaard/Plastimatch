/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "itkImage.h"

#include "image_boundary.h"
#include "itk_image_load.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "volume.h"

class Image_boundary_private {
public:
    Image_boundary_private () {
        vbb = Image_boundary::EDGE_PADDING;
//        vbb = Image_boundary::ZERO_PADDING;
    }
public:
    UCharImageType::Pointer input_image;
    UCharImageType::Pointer output_image;
    Image_boundary::Volume_boundary_behavior vbb;
public:
    void run ();
protected:
    unsigned char classify_zp (
        const Volume::Pointer& vol_in,
        const unsigned char *img_in,
        plm_long i, plm_long j, plm_long k, plm_long v)
    {
        /* If not inside volume, then not on boundary */
        if (!img_in[v]) {
            return 0;
        }

        /* Non-zero edge pixels are boundary */
        if (k == 0 || k == vol_in->dim[2]-1
            || j == 0 || j == vol_in->dim[1]-1
            || i == 0 || i == vol_in->dim[0]-1)
        {
            return 1;
        }

        /* Look for neighboring zero voxel in six-neighborhood */
        if (img_in[volume_index (vol_in->dim, i-1, j, k)] == 0) {
            return 1;
        }
        if (img_in[volume_index (vol_in->dim, i+1, j, k)] == 0) {
            return 1;
        }
        if (img_in[volume_index (vol_in->dim, i, j-1, k)] == 0) {
            return 1;
        }
        if (img_in[volume_index (vol_in->dim, i, j+1, k)] == 0) {
            return 1;
        }
        if (img_in[volume_index (vol_in->dim, i, j, k-1)] == 0) {
            return 1;
        }
        if (img_in[volume_index (vol_in->dim, i, j, k+1)] == 0) {
            return 1;
        }
        return 0;
    }

    unsigned char classify_ep (
        const Volume::Pointer& vol_in,
        const unsigned char *img_in,
        plm_long i, plm_long j, plm_long k, plm_long v)
    {
        /* If not inside volume, then not on boundary */
        if (!img_in[v]) {
            return 0;
        }

        /* Look for neighboring zero voxel in six-neighborhood,
           ignoring voxels beyond boundary */
        if (i != 0 
            && img_in[volume_index (vol_in->dim, i-1, j, k)] == 0)
        {
            return 1;
        }
        if (i != vol_in->dim[0]-1 
            && img_in[volume_index (vol_in->dim, i+1, j, k)] == 0)
        {
            return 1;
        }
        if (j != 0 
            && img_in[volume_index (vol_in->dim, i, j-1, k)] == 0)
        {
            return 1;
        }
        if (j != vol_in->dim[1]-1
            && img_in[volume_index (vol_in->dim, i, j+1, k)] == 0)
        {
            return 1;
        }
        if (k != 0 
            && img_in[volume_index (vol_in->dim, i, j, k-1)] == 0)
        {
            return 1;
        }
        if (k != vol_in->dim[2]-1
            && img_in[volume_index (vol_in->dim, i, j, k+1)] == 0)
        {
            return 1;
        }
        return 0;
    }
};

void 
Image_boundary_private::run ()
{
    /* Convert to Plm_image type */
    Plm_image pli_in (this->input_image);
    Volume::Pointer vol_in = pli_in.get_volume_uchar ();
    unsigned char *img_in = (unsigned char*) vol_in->img;

    /* Allocate output image */
    Plm_image *pli_out = pli_in.clone ();
    Volume::Pointer vol_out = pli_out->get_volume_uchar ();
    unsigned char *img_out = (unsigned char*) vol_out->img;

    /* Compute the boundary */
    for (plm_long k = 0, v = 0; k < vol_in->dim[2]; k++) {
        for (plm_long j = 0; j < vol_in->dim[1]; j++) {
            for (plm_long i = 0; i < vol_in->dim[0]; i++, v++) {
                if (this->vbb == Image_boundary::ZERO_PADDING) {
                    img_out[v] = classify_zp (vol_in, img_in, i, j, k, v);
                } else {
                    img_out[v] = classify_ep (vol_in, img_in, i, j, k, v);
                }
            }
        }
    }

    /* Save the output image */
    this->output_image = pli_out->itk_uchar ();

    /* Clean up */
    delete pli_out;
}

Image_boundary::Image_boundary ()
{
    d_ptr = new Image_boundary_private;
}

Image_boundary::~Image_boundary ()
{
    delete d_ptr;
}

void 
Image_boundary::set_input_image (const char* image_fn)
{
    d_ptr->input_image = itk_image_load_uchar (image_fn, 0);
}

void 
Image_boundary::set_input_image (
    const UCharImageType::Pointer image)
{
    d_ptr->input_image = image;
}

void 
Image_boundary::run ()
{
    d_ptr->run ();
}

UCharImageType::Pointer
Image_boundary::get_output_image ()
{
    return d_ptr->output_image;
}

UCharImageType::Pointer
do_image_boundary (UCharImageType::Pointer image)
{
    Image_boundary ib;
    ib.set_input_image (image);
    ib.run ();
    return ib.get_output_image ();
}
