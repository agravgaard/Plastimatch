/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include "itkImageMomentsCalculator.h"

#include "logfile.h"
#include "print_and_exit.h"
#include "registration_data.h"
#include "shared_parms.h"
#include "stage_parms.h"
#include "xform.h"

static void
itk_align_center (
    Registration_data* regd, Xform *xf_out, 
    const Xform *xf_in, const Stage_parms* stage);

static void
itk_align_center_of_gravity (
    Registration_data* regd, Xform *xf_out, 
    const Xform *xf_in, const Stage_parms* stage);

static void
itk_align_center (
    Registration_data* regd, Xform *xf_out, 
    const Xform *xf_in, const Stage_parms* stage)
{
    Plm_image::Pointer fixed_image = regd->get_fixed_image();
    Plm_image::Pointer moving_image = regd->get_moving_image();
    float fixed_center[3];
    float moving_center[3];
    itk_volume_center (fixed_center, fixed_image->itk_float());
    itk_volume_center (moving_center, moving_image->itk_float());

    itk::Array<double> trn_parms (3);
    trn_parms[0] = moving_center[0] - fixed_center[0];
    trn_parms[1] = moving_center[1] - fixed_center[1];
    trn_parms[2] = moving_center[2] - fixed_center[2];
    xf_out->set_trn (trn_parms);
}

static void
itk_align_center_of_gravity (
    Registration_data* regd, Xform *xf_out, 
    const Xform *xf_in, const Stage_parms* stage)
{
    if (regd->get_fixed_roi() && regd->get_moving_roi()) {
        typedef itk::ImageMomentsCalculator<UCharImageType>
            ImageMomentsCalculatorType;

        ImageMomentsCalculatorType::Pointer fixedCalculator
            = ImageMomentsCalculatorType::New();
        fixedCalculator->SetImage(regd->get_fixed_roi()->itk_uchar());
        fixedCalculator->Compute();

        ImageMomentsCalculatorType::Pointer movingCalculator
            = ImageMomentsCalculatorType::New();
        movingCalculator->SetImage(regd->get_moving_roi()->itk_uchar());
        movingCalculator->Compute();

        ImageMomentsCalculatorType::VectorType fixedCenter; 
        ImageMomentsCalculatorType::VectorType movingCenter; 
    
        fixedCenter = fixedCalculator->GetCenterOfGravity();
        movingCenter = movingCalculator->GetCenterOfGravity();

        itk::Array<double> trn_parms (3);
        trn_parms[0] = movingCenter[0] - fixedCenter[0];
        trn_parms[1] = movingCenter[1] - fixedCenter[1];
        trn_parms[2] = movingCenter[2] - fixedCenter[2];
        xf_out->set_trn (trn_parms);
    }

    else {
        print_and_exit("NO ROIs SET!");
    }
}

Xform::Pointer
do_itk_align_center (
    Registration_data* regd, 
    const Xform::Pointer& xf_in, 
    Stage_parms* stage
)
{
    Xform::Pointer xf_out = Xform::New ();
    itk_align_center (regd, xf_out.get(), xf_in.get(), stage);
    return xf_out;
}

Xform::Pointer
do_itk_align_center_of_gravity (
    Registration_data* regd, 
    const Xform::Pointer& xf_in, 
    Stage_parms* stage
)
{
    Xform::Pointer xf_out = Xform::New ();
    itk_align_center_of_gravity (regd, xf_out.get(), xf_in.get(), stage);
    return xf_out;
}
