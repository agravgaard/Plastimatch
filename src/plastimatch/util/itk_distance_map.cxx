/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "itkApproximateSignedDistanceMapImageFilter.h"
#include "itkImage.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "itk_distance_map.h"
#include "itk_image_type.h"

/* itk::ApproximateSignedDistanceMapImageFilter apparently doesn't work */
#if defined (commentout)
FloatImageType::Pointer
itk_distance_map_approximate (
    UCharImageType::Pointer& ref_image,
    bool use_squared_distance,
    bool inside_positive
)
{
    typedef itk::ApproximateSignedDistanceMapImageFilter< 
        UCharImageType, FloatImageType >  FilterType;
    FilterType::Pointer filter = FilterType::New ();

#if defined (commentout)
    if (this->use_squared_distance) {
        filter->SetSquaredDistance (true);
    } else {
        filter->SetSquaredDistance (false);
    }

    /* Always compute map in millimeters, never voxels */
    filter->SetUseImageSpacing (true);

    if (this->inside_is_positive) {
        filter->SetInsideIsPositive (true);
    } else {
        filter->SetInsideIsPositive (false);
    }
#endif

    /* ITK is very odd... */
    filter->SetOutsideValue (0);
    filter->SetInsideValue (1);

    /* Run the filter */
    filter->SetInput (this->input);
    filter->Update();
    this->output = filter->GetOutput ();
}
#endif

FloatImageType::Pointer
itk_distance_map_danielsson (
    const UCharImageType::Pointer& ref_image,
    bool use_squared_distance,
    bool inside_positive
)
{
    typedef itk::SignedDanielssonDistanceMapImageFilter< 
        UCharImageType, FloatImageType >  FilterType;
    FilterType::Pointer filter = FilterType::New ();

    if (use_squared_distance) {
        filter->SetSquaredDistance (true);
    } else {
        filter->SetSquaredDistance (false);
    }

    /* Always compute map in millimeters, never voxels */
    filter->SetUseImageSpacing (true);

    if (inside_positive) {
        filter->SetInsideIsPositive (true);
    } else {
        filter->SetInsideIsPositive (false);
    }

    /* Run the filter */
    filter->SetInput (ref_image);
    filter->Update();
    return filter->GetOutput ();
}

FloatImageType::Pointer
itk_distance_map_maurer (
    const UCharImageType::Pointer& ref_image,
    bool use_squared_distance,
    bool inside_positive
)
{
    typedef itk::SignedMaurerDistanceMapImageFilter< 
        UCharImageType, FloatImageType >  FilterType;
    FilterType::Pointer filter = FilterType::New ();

    if (use_squared_distance) {
        filter->SetSquaredDistance (true);
    } else {
        filter->SetSquaredDistance (false);
    }

    /* Always compute map in millimeters, never voxels */
    filter->SetUseImageSpacing (true);

    if (inside_positive) {
        filter->SetInsideIsPositive (true);
    } else {
        filter->SetInsideIsPositive (false);
    }

    /* Run the filter */
    filter->SetInput (ref_image);
    filter->Update();
    return filter->GetOutput ();
}
