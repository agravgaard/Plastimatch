/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageRegionIterator.h>

#include "float_pair_list.h"
#include "itk_image_create.h"
#include "itk_threshold.h"
#include "plm_image_header.h"

UCharImageType::Pointer
itk_threshold_above (
    const FloatImageType::Pointer& image, 
    float threshold)
{
    typedef itk::BinaryThresholdImageFilter< 
        FloatImageType, UCharImageType > ThresholdFilterType;
    ThresholdFilterType::Pointer thresh_filter = ThresholdFilterType::New ();
    thresh_filter->SetInput (image);
    thresh_filter->SetLowerThreshold (threshold);
    thresh_filter->SetOutsideValue (0);
    thresh_filter->SetInsideValue (1);
    try {
        thresh_filter->Update ();
    } catch (itk::ExceptionObject & excep) {
        std::cerr << "Exception caught !" << std::endl;
        std::cerr << excep << std::endl;
    }
    return thresh_filter->GetOutput ();
}

UCharImageType::Pointer
itk_threshold (
    const FloatImageType::Pointer& image_in, 
    const Float_pair_list& fpl)
{
    UCharImageType::Pointer image_out = itk_image_create<unsigned char> (
        Plm_image_header (image_in));

    typedef itk::ImageRegionIterator< FloatImageType > FloatIteratorType;
    typedef itk::ImageRegionIterator< UCharImageType > UCharIteratorType;
    FloatImageType::RegionType rg = image_out->GetLargestPossibleRegion ();
    FloatIteratorType it_in (image_in, rg);
    UCharIteratorType it_out (image_out, rg);

    for (it_in.GoToBegin(), it_out.GoToBegin(); 
         !it_in.IsAtEnd(); 
         ++it_in, ++it_out)
    {
        float vin = it_in.Get();
        unsigned char vout = 0;

        Float_pair_list::const_iterator fpl_it = fpl.begin();
        while (fpl_it != fpl.end()) {
            if (vin >= fpl_it->first && vin <= fpl_it->second) {
                vout = 1;
                break;
            }
            fpl_it ++;
        }
        it_out.Set (vout);
    }
    return image_out;
}

UCharImageType::Pointer
itk_threshold (
    const FloatImageType::Pointer& image_in, 
    const std::string& fpl_string)
{
    Float_pair_list fpl = parse_float_pairs (fpl_string);
    return itk_threshold (image_in, fpl);
}
