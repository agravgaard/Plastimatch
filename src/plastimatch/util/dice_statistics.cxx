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
#include "dice_statistics.h"
#include "itk_image.h"
#include "itk_image_load.h"
#include "itk_image_save.h"
#include "itk_resample.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "volume.h"

class Dice_statistics_private {
public:
    Dice_statistics_private () {
        TP = TN = FP = FN = 0;
    }
public:
    size_t TP, TN, FP, FN;
    float dice;
    size_t ref_num_vox;
    size_t cmp_num_vox;
    DoubleVector3DType ref_cog;
    DoubleVector3DType cmp_cog;
    double ref_vol;
    double cmp_vol;
    UCharImageType::Pointer ref_image;
    UCharImageType::Pointer cmp_image;
};

Dice_statistics::Dice_statistics ()
{
    d_ptr = new Dice_statistics_private;
}

Dice_statistics::~Dice_statistics ()
{
    delete d_ptr;
}

void 
Dice_statistics::set_reference_image (const char* image_fn)
{
    d_ptr->ref_image = itk_image_load_uchar (image_fn, 0);
}

void 
Dice_statistics::set_reference_image (
    const UCharImageType::Pointer& image)
{
    d_ptr->ref_image = image;
}

void 
Dice_statistics::set_compare_image (const char* image_fn)
{
    d_ptr->cmp_image = itk_image_load_uchar (image_fn, 0);
}

void 
Dice_statistics::set_compare_image (
    const UCharImageType::Pointer& image)
{
    d_ptr->cmp_image = image;
}

void 
Dice_statistics::run ()
{
    /* Resample and/or expand images based on geometry of reference */
    if (!itk_image_header_compare (d_ptr->ref_image, d_ptr->cmp_image)) {
        Plm_image_header pih;
        pih.set_geometry_to_contain (
            Plm_image_header (d_ptr->cmp_image),
            Plm_image_header (d_ptr->ref_image));
        d_ptr->cmp_image = resample_image (d_ptr->cmp_image, pih, 0, 0);
        d_ptr->ref_image = resample_image (d_ptr->ref_image, pih, 0, 0);
    }

    /* Initialize counters */
    d_ptr->ref_num_vox = 0;
    d_ptr->cmp_num_vox = 0;
    d_ptr->TP = 0;
    d_ptr->TN = 0;
    d_ptr->FP = 0;
    d_ptr->FN = 0;

    /* Convert to Plm_image type */
    Plm_image ref (d_ptr->ref_image);
    Volume::Pointer vol_ref = ref.get_volume_uchar ();
    unsigned char *img_ref = (unsigned char*) vol_ref->img;
    Plm_image cmp (d_ptr->cmp_image);
    Volume::Pointer vol_cmp = cmp.get_volume_uchar ();
    unsigned char *img_cmp = (unsigned char*) vol_cmp->img;

    size_t tp = 0;
    size_t tn = 0;
    size_t fp = 0;
    size_t fn = 0;
    double rx = 0, ry = 0, rz = 0;
    double cx = 0, cy = 0, cz = 0;

#pragma omp parallel for reduction(+:tp,tn,fp,fn,cx,cy,cz,rx,ry,rz)
    LOOP_Z_OMP (k, vol_ref) {
        plm_long fijk[3];      /* Index within fixed image (vox) */
        float fxyz[3];         /* Position within fixed image (mm) */
        fijk[2] = k;
        fxyz[2] = vol_ref->origin[2] + fijk[2] * vol_ref->step[2*3+2];
        LOOP_Y (fijk, fxyz, vol_ref) {
            LOOP_X (fijk, fxyz, vol_ref) {
                plm_long v = volume_index (vol_ref->dim, fijk);
                unsigned char vox_ref = img_ref[v];
                unsigned char vox_cmp = img_cmp[v];

                if (vox_ref) {
                    if (vox_cmp) {
                        tp++;
                    } else {
                        fn++;
                    }
                } else {
                    if (vox_cmp) {
                        fp++;
                    } else {
                        tn++;
                    }
                }
                if (vox_ref) {
                    rx += fxyz[0];
                    ry += fxyz[1];
                    rz += fxyz[2];
                }
                if (vox_cmp) {
                    cx += fxyz[0];
                    cy += fxyz[1];
                    cz += fxyz[2];
                }
            }
        }
    }

    d_ptr->TP = tp;
    d_ptr->FP = fp;
    d_ptr->TN = tn;
    d_ptr->FN = fn;
    d_ptr->ref_num_vox = tp + fn;
    d_ptr->cmp_num_vox = tp + fp;

    /* Compute volume and center of mass */
    /* Voxel size is same for both images */
    double vox_size = vol_ref->spacing[0] * vol_ref->spacing[1] 
        * vol_ref->spacing[2];
    d_ptr->ref_vol = d_ptr->ref_num_vox * vox_size;
    d_ptr->cmp_vol = d_ptr->cmp_num_vox * vox_size;
    d_ptr->ref_cog[0] = d_ptr->ref_cog[1] = d_ptr->ref_cog[2] = 0.f;
    d_ptr->cmp_cog[0] = d_ptr->cmp_cog[1] = d_ptr->cmp_cog[2] = 0.f;
    if (d_ptr->ref_num_vox > 0) {
        d_ptr->ref_cog[0] = rx / d_ptr->ref_num_vox;
        d_ptr->ref_cog[1] = ry / d_ptr->ref_num_vox;
        d_ptr->ref_cog[2] = rz / d_ptr->ref_num_vox;
    }
    if (d_ptr->ref_num_vox > 0) {
        d_ptr->cmp_cog[0] = cx / d_ptr->cmp_num_vox;
        d_ptr->cmp_cog[1] = cy / d_ptr->cmp_num_vox;
        d_ptr->cmp_cog[2] = cz / d_ptr->cmp_num_vox;
    }
}

float
Dice_statistics::get_dice ()
{
    float dice = 0.f;
    if ((d_ptr->ref_num_vox + d_ptr->cmp_num_vox) > 0) {
        dice = ((float) (2 * d_ptr->TP))
            / ((float) (d_ptr->ref_num_vox + d_ptr->cmp_num_vox));
    }
    return dice;
}

float
Dice_statistics::get_sensitivity()
{
    return ((float) d_ptr->TP) / ((float) (d_ptr->TP + d_ptr->FN));
}

float
Dice_statistics::get_specificity()
{
    return ((float) d_ptr->TN) / ((float) (d_ptr->TN + d_ptr->FP));
}

size_t
Dice_statistics::get_true_positives ()
{
  return d_ptr->TP;
}

size_t
Dice_statistics::get_true_negatives ()
{
  return d_ptr->TN;
}

size_t
Dice_statistics::get_false_positives ()
{
  return d_ptr->FP;
}

size_t
Dice_statistics::get_false_negatives ()
{
  return d_ptr->FN;
}

DoubleVector3DType 
Dice_statistics::get_reference_center ()
{
  return d_ptr->ref_cog;
}

DoubleVector3DType 
Dice_statistics::get_compare_center ()
{
  return d_ptr->cmp_cog;
}

double
Dice_statistics::get_reference_volume ()
{
  return d_ptr->ref_vol;
}

double
Dice_statistics::get_compare_volume ()
{
  return d_ptr->cmp_vol;
}


void 
Dice_statistics::debug ()
{
    lprintf ("CENTER_OF_MASS\n");
    lprintf ("ref\t %13g\t %13g\t %13g\n", 
        d_ptr->ref_cog[0], d_ptr->ref_cog[1], d_ptr->ref_cog[2]);
    lprintf ("cmp\t %13g\t %13g\t %13g\n", 
        d_ptr->cmp_cog[0], d_ptr->cmp_cog[1], d_ptr->cmp_cog[2]);

    lprintf ("TP: %13d\n",d_ptr->TP);
    lprintf ("TN: %13d\n",d_ptr->TN);
    lprintf ("FN: %13d\n",d_ptr->FN);
    lprintf ("FP: %13d\n",d_ptr->FP);

    lprintf ("DICE: %13f\n", this->get_dice());
    
    lprintf ("SE: %13f\n", this->get_sensitivity());
    lprintf ("SP: %13f\n", this->get_specificity());
}
