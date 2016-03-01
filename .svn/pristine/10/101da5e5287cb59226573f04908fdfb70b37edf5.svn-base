/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkAndConstantToImageFilter.h"

#include "itk_image.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "ss_img_extract.h"

UCharImageType::Pointer
ss_img_extract_bit (UInt32ImageType::Pointer image, unsigned int bit)
{
    typedef itk::AndConstantToImageFilter< UInt32ImageType, 
	uint32_t, UCharImageType > AndFilterType;

    AndFilterType::Pointer and_filter = AndFilterType::New();

    and_filter->SetInput (image);
    and_filter->SetConstant (1 << bit);
    try {
	and_filter->Update ();
    }
    catch (itk::ExceptionObject &err) {
	std::cout << "Exception during and operation." << std::endl;
	std::cout << err << std::endl;
	exit (1);
    }
    return and_filter->GetOutput ();
}

UCharImageType::Pointer
ss_img_extract_bit (
    UCharVecImageType::Pointer im_in, 
    unsigned int bit
)
{
    const UCharVecImageType::RegionType rgn_in_alt
	= im_in->GetLargestPossibleRegion();

    UCharImageType::Pointer im_out = UCharImageType::New ();
    itk_image_header_copy (im_out, im_in);
    im_out->Allocate ();

    typedef itk::ImageRegionIterator< UCharVecImageType > UCharVecIteratorType;
    const UCharVecImageType::RegionType rgn_in
	= im_in->GetLargestPossibleRegion();
    UCharVecIteratorType it_in (im_in, rgn_in);
    typedef itk::ImageRegionIterator< UCharImageType > UCharIteratorType;
    const UCharImageType::RegionType rgn_out 
	= im_out->GetLargestPossibleRegion();
    UCharIteratorType it_out (im_out, rgn_out);

    unsigned int uchar_no = bit / 8;
    unsigned int bit_no = bit % 8;
    unsigned char bit_mask = 1 << bit_no;
    if (uchar_no > im_in->GetVectorLength()) {
	print_and_exit (
	    "Error: bit %d was requested from image that has %d bits\n", 
	    bit, im_in->GetVectorLength() * 8);
    }
    for (it_in.GoToBegin(), it_out.GoToBegin();
	 !it_in.IsAtEnd();
	 ++it_in, ++it_out)
    {
	itk::VariableLengthVector<unsigned char> v_in = it_in.Get ();
	unsigned char v_in_uchar = v_in[uchar_no];
	it_out.Set ((v_in_uchar & bit_mask) ? 1 : 0);
    }
    return im_out;
}

UCharImageType::Pointer
ss_img_extract_bit (
    const Plm_image::Pointer& image,
    unsigned int bit
)
{
    if (image->m_type == PLM_IMG_TYPE_GPUIT_UCHAR_VEC
        || image->m_type == PLM_IMG_TYPE_ITK_UCHAR_VEC) 
    {
	image->convert (PLM_IMG_TYPE_ITK_UCHAR_VEC);
        return ss_img_extract_bit (image->m_itk_uchar_vec, bit);
    }
    else {
	image->convert (PLM_IMG_TYPE_ITK_ULONG);
        return ss_img_extract_bit (image->m_itk_uint32, bit);
    }
}

template<class T>
typename itk::Image<
    typename T::ObjectType::IOPixelType, T::ObjectType::ImageDimension
    >::Pointer
ss_img_extract_uchar (
    T im_in, 
    unsigned int uchar_no
)
{
    typedef typename T::ObjectType InImageType;
    typedef typename itk::Image<
	typename T::ObjectType::IOPixelType, T::ObjectType::ImageDimension
	> OutImageType;
    const typename InImageType::RegionType rgn_in_alt
	= im_in->GetLargestPossibleRegion();

    typename OutImageType::Pointer im_out = OutImageType::New ();
    itk_image_header_copy (im_out, im_in);
    im_out->Allocate ();

    typedef itk::ImageRegionIterator< InImageType > InImageIteratorType;
    const typename InImageType::RegionType rgn_in 
	= im_in->GetLargestPossibleRegion();
    InImageIteratorType it_in (im_in, rgn_in);
    typedef itk::ImageRegionIterator< OutImageType > OutImageIteratorType;
    const typename OutImageType::RegionType rgn_out 
	= im_out->GetLargestPossibleRegion();
    OutImageIteratorType it_out (im_out, rgn_out);

    if (uchar_no > im_in->GetVectorLength()) {
	print_and_exit (
	    "Error: uchar %d was requested from image that has %d uchars\n", 
	    uchar_no, im_in->GetVectorLength());
    }
    for (it_in.GoToBegin(), it_out.GoToBegin();
	 !it_in.IsAtEnd();
	 ++it_in, ++it_out)
    {
	itk::VariableLengthVector<unsigned char> v_in = it_in.Get ();
	unsigned char v_in_uchar = v_in[uchar_no];
	it_out.Set (v_in_uchar);
    }
    return im_out;
}

void
ss_img_insert_uchar (
    UCharVecImageType::Pointer vec_img, 
    UCharImageType::Pointer uchar_img, 
    unsigned int uchar_no
)
{
    typedef itk::ImageRegionIterator< UCharVecImageType > UCharVecIteratorType;
    const UCharVecImageType::RegionType vec_rgn
	= vec_img->GetLargestPossibleRegion();
    UCharVecIteratorType vec_it (vec_img, vec_rgn);
    typedef itk::ImageRegionIterator< UCharImageType > UCharIteratorType;
    const UCharImageType::RegionType uchar_rgn
	= uchar_img->GetLargestPossibleRegion();
    UCharIteratorType uchar_it (uchar_img, uchar_rgn);

    if (uchar_no > vec_img->GetVectorLength()) {
	print_and_exit (
	    "Error: uchar %d was requested from image that has %d uchars\n", 
	    uchar_no, vec_img->GetVectorLength());
    }
    for (vec_it.GoToBegin(), uchar_it.GoToBegin();
	 !vec_it.IsAtEnd();
	 ++vec_it, ++uchar_it)
    {
	itk::VariableLengthVector<unsigned char> vec = vec_it.Get ();
	unsigned char uch = uchar_it.Get ();
	vec[uchar_no] = uch;
	vec_it.Set (vec);
    }
}

/* Explicit instantiation */
template PLMBASE_API UCharImageType::Pointer ss_img_extract_uchar (UCharVecImageType::Pointer, unsigned int);
template PLMBASE_API UCharImage2DType::Pointer ss_img_extract_uchar (UCharVecImage2DType::Pointer, unsigned int);
