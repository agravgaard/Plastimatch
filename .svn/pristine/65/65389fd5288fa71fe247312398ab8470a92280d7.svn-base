/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include "itkImageRegionIterator.h"

#include "itk_mask.h"

template <class T>
T
mask_image (
    T input,
    UCharImageType::Pointer mask,
    Mask_operation mask_operation,
    float mask_value
)
{
    typedef typename T::ObjectType ImageType;
    typedef typename T::ObjectType::PixelType PixelType;
    typedef typename itk::ImageRegionIterator< UCharImageType > 
	UCharIteratorType;
    typedef typename itk::ImageRegionIterator< ImageType > ImageIteratorType;

    typename ImageType::RegionType rgn_input 
	= input->GetLargestPossibleRegion();
    typename UCharImageType::RegionType rgn_mask 
	= mask->GetLargestPossibleRegion();
    const typename ImageType::PointType& og = input->GetOrigin();
    const typename ImageType::SpacingType& sp = input->GetSpacing();
    const typename ImageType::DirectionType& dc = input->GetDirection();
    
    typename ImageType::Pointer im_out = ImageType::New();
    im_out->SetRegions (rgn_input);
    im_out->SetOrigin (og);
    im_out->SetSpacing (sp);
    im_out->SetDirection (dc);
    im_out->Allocate ();

    ImageIteratorType it_in (input, rgn_input);
    UCharIteratorType it_mask (mask, rgn_mask);
    ImageIteratorType it_out (im_out, rgn_input);

    for (it_in.GoToBegin(); !it_in.IsAtEnd(); ++it_in,++it_mask,++it_out) {
	PixelType p1 = it_in.Get();
	unsigned char p2 = it_mask.Get();
	if ((p2 > 0) ^ (mask_operation == MASK_OPERATION_MASK)) {
	    it_out.Set (mask_value);
	} else {
	    it_out.Set (p1);
	}
    }
    return im_out;
}

#if defined (commentout)
void
merge_pixels (ShortImageType::Pointer im_out, ShortImageType::Pointer im_1, 
	     UCharImageType::Pointer im_2, int mask_value)
{
    typedef itk::ImageRegionIterator< UCharImageType > UCharIteratorType;
    typedef itk::ImageRegionIterator< ShortImageType > ShortIteratorType;
    ShortImageType::RegionType r_1 = im_1->GetLargestPossibleRegion();
    UCharImageType::RegionType r_2 = im_2->GetLargestPossibleRegion();

    //const ShortImageType::IndexType& st = r_1.GetIndex();
    //const ShortImageType::SizeType& sz = r_1.GetSize();
    //const InputImageType::SizeType& sz = image->GetLargestPossibleRegion().GetSize();
    const ShortImageType::PointType& og = im_1->GetOrigin();
    const ShortImageType::SpacingType& sp = im_1->GetSpacing();
    
    im_out->SetRegions(r_1);
    im_out->SetOrigin(og);
    im_out->SetSpacing(sp);
    im_out->Allocate();

    ShortIteratorType it_1 (im_1, r_1);
    UCharIteratorType it_2 (im_2, r_2);
    ShortIteratorType it_out (im_out, r_1);

    for (it_1.GoToBegin(); !it_1.IsAtEnd(); ++it_1,++it_2,++it_out) {
	short p1 = it_1.Get();
	unsigned char p2 = it_2.Get();
	if (p2 > 0) {
	    it_out.Set (p1);
	} else {
	    it_out.Set (mask_value);
	}
    }
}

void
mask_vf(DeformationFieldType::Pointer vf_out, DeformationFieldType::Pointer vf, 
	     UCharImageType::Pointer mask, float mask_value[3])
{
    typedef itk::ImageRegionIterator< UCharImageType > UCharIteratorType;
    typedef itk::ImageRegionIterator< DeformationFieldType > DeformationFieldIteratorType;
    DeformationFieldType::RegionType r_1 = vf->GetLargestPossibleRegion();
    UCharImageType::RegionType r_2 = mask->GetLargestPossibleRegion();

    //const DeformationFieldType::IndexType& st = r_1.GetIndex();
    //const DeformationFieldType::SizeType& sz = r_1.GetSize();
    const DeformationFieldType::PointType& og = vf->GetOrigin();
    const DeformationFieldType::SpacingType& sp = vf->GetSpacing();
    
    vf_out->SetRegions(r_1);
    vf_out->SetOrigin(og);
    vf_out->SetSpacing(sp);
    vf_out->Allocate();

    DeformationFieldIteratorType it_1 (vf, r_1);
    UCharIteratorType it_2 (mask, r_2);
    DeformationFieldIteratorType it_out (vf_out, r_1);

    for (it_1.GoToBegin(); !it_1.IsAtEnd(); ++it_1,++it_2,++it_out) {
	itk::Vector<float,3> p1 = it_1.Get();
	unsigned char p2 = it_2.Get();
	if (p2 > 0) {
	    it_out.Set (p1);
	} else {
	    it_out.Set (mask_value);
	}
    }
}
#endif

/* Explicit instantiations */
template PLMUTIL_API UCharImageType::Pointer mask_image (UCharImageType::Pointer, UCharImageType::Pointer, Mask_operation, float);
template PLMUTIL_API UShortImageType::Pointer mask_image (UShortImageType::Pointer, UCharImageType::Pointer, Mask_operation, float);
template PLMUTIL_API ShortImageType::Pointer mask_image (ShortImageType::Pointer, UCharImageType::Pointer, Mask_operation, float);
template PLMUTIL_API UInt32ImageType::Pointer mask_image (UInt32ImageType::Pointer, UCharImageType::Pointer, Mask_operation, float);
template PLMUTIL_API FloatImageType::Pointer mask_image (FloatImageType::Pointer, UCharImageType::Pointer, Mask_operation, float);
