/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include "itkImageRegionIterator.h"

#include "itk_image.h"
#include "itk_union.h"
#include "print_and_exit.h"

UCharImageType::Pointer
itk_union (
    const UCharImageType::Pointer image_1,
    const UCharImageType::Pointer image_2)
{
    typedef itk::ImageRegionIterator< UCharImageType > IteratorType;
    UCharImageType::RegionType r_1 = image_1->GetLargestPossibleRegion();
    UCharImageType::RegionType r_2 = image_2->GetLargestPossibleRegion();

    const UCharImageType::PointType& og = image_1->GetOrigin();
    const UCharImageType::SpacingType& sp = image_1->GetSpacing();

    if (!itk_image_header_compare (image_1, image_2)) {
        print_and_exit ("Sorry, input images to itk_union must have "
            "the same geometry.");
    }

    UCharImageType::Pointer im_out = UCharImageType::New();
    im_out->SetRegions(r_1);
    im_out->SetOrigin(og);
    im_out->SetSpacing(sp);
    im_out->Allocate();

    IteratorType it_1 (image_1, r_1);
    IteratorType it_2 (image_2, r_2);
    IteratorType it_out (im_out, r_2);

    for (it_1.GoToBegin(); !it_1.IsAtEnd(); ++it_1,++it_2,++it_out) {
	unsigned char p1 = it_1.Get();
	unsigned char p2 = it_2.Get();
	it_out.Set (p1 | p2);
    }

    return im_out;
}
