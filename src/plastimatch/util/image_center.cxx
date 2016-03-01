/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageMomentsCalculator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageSliceConstIteratorWithIndex.h"

#include "compiler_warnings.h"
#include "image_center.h"
#include "itk_image.h"
#include "itk_image_load.h"
#include "itk_image_save.h"
#include "itk_resample.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "volume.h"

class Image_center_private {
public:
    DoubleVector3DType center_of_mass;
    Plm_image::Pointer image;
};

Image_center::Image_center ()
{
    d_ptr = new Image_center_private;
}

Image_center::~Image_center ()
{
    delete d_ptr;
}

void 
Image_center::set_image (
    const UCharImageType::Pointer& image)
{
    d_ptr->image = Plm_image::New(image);
}

void 
Image_center::set_image (
    const Plm_image::Pointer& pli)
{
    d_ptr->image = pli;
}

void 
Image_center::run ()
{
    /* Convert image to Volume type */
    Volume::Pointer vol = d_ptr->image->get_volume_uchar ();
    double x = 0, y = 0, z = 0;
    size_t num_vox = 0;
    unsigned char *img = vol->get_raw<unsigned char>();

#pragma omp parallel for reduction(+:num_vox,x,y,z)
    LOOP_Z_OMP (k, vol) {
        plm_long ijk[3];      /* Index within image (vox) */
        float xyz[3];         /* Position within image (mm) */
        ijk[2] = k;
        xyz[2] = vol->origin[2] + ijk[2] * vol->step[2*3+2];
        LOOP_Y (ijk, xyz, vol) {
            LOOP_X (ijk, xyz, vol) {
                plm_long v = volume_index (vol->dim, ijk);
                unsigned char vox_img = img[v];

                if (vox_img) {
                    num_vox++;
                    x += xyz[0];
                    y += xyz[1];
                    z += xyz[2];
                }
            }
        }
    }

    /* Compute volume and center of mass */
    /* Voxel size is same for both images */
    if (num_vox > 0) {
        d_ptr->center_of_mass[0] = x / num_vox;
        d_ptr->center_of_mass[1] = y / num_vox;
        d_ptr->center_of_mass[2] = z / num_vox;
    }
}

DoubleVector3DType 
Image_center::get_image_center_of_mass ()
{
  return d_ptr->center_of_mass;
}
