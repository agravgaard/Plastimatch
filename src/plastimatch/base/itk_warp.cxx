/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <time.h>
#include "itkConfigure.h"
#include "itkImage.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkWarpImageFilter.h"

#include "itk_image.h"
#include "itk_warp.h"
#include "ss_img_extract.h"

/* Warp the image.  
    im_in:	    the image which is warped
    vf:		    the vector field.  output size = vf size.
    default_val:    the value to use for pixels outside the volume
*/
template<class T, class U>
T 
itk_warp_image (
    T im_in, 
    DeformationFieldType::Pointer vf, 
    int linear_interp,
    U default_val)
{
    typedef typename T::ObjectType TBase;
    typedef typename T::ObjectType::PixelType PixelType;
    typedef itk::WarpImageFilter < 
	TBase, TBase, DeformationFieldType > T_FilterType;
    typedef itk::LinearInterpolateImageFunction < 
	TBase, double > T_LinInterpType;
    typedef itk::NearestNeighborInterpolateImageFunction < 
	TBase, double >  T_NNInterpType;

    T im_out = TBase::New();

    typename T_FilterType::Pointer filter = T_FilterType::New();
    typename T_LinInterpType::Pointer l_interpolator = T_LinInterpType::New();
    typename T_NNInterpType::Pointer nn_interpolator = T_NNInterpType::New();

    const typename TBase::PointType& og = vf->GetOrigin();
    const typename TBase::SpacingType& sp = vf->GetSpacing();
    const typename TBase::DirectionType& di = vf->GetDirection();

    if (linear_interp) {
	filter->SetInterpolator (l_interpolator);
    } else {
	filter->SetInterpolator (nn_interpolator);
    }
    filter->SetOutputSpacing (sp);
    filter->SetOutputOrigin (og);
    filter->SetOutputDirection (di);
#if ITK_VERSION_MAJOR == 3
    filter->SetDeformationField (vf);
#else /* ITK 4 */
    filter->SetDisplacementField (vf);
#endif
    filter->SetInput (im_in);

    filter->SetEdgePaddingValue ((PixelType) default_val);
    
    try {
	filter->Update();
    } catch( itk::ExceptionObject & excp ) {
	std::cerr << "Exception thrown " << std::endl;
	std::cerr << excp << std::endl;
    }

    im_out = filter->GetOutput();
    im_out->Update();
    return im_out;
}

UCharVecImageType::Pointer 
itk_warp_image (
    UCharVecImageType::Pointer im_in, 
    DeformationFieldType::Pointer vf, 
    int linear_interp, 
    unsigned char default_val
)
{
    UCharVecImageType::Pointer im_out = UCharVecImageType::New();
    itk_image_header_copy (im_out, vf);
    im_out->SetVectorLength (im_in->GetVectorLength());
    im_out->Allocate ();

    unsigned int num_uchar = im_in->GetVectorLength();

    for (unsigned int uchar_no = 0; uchar_no < num_uchar; uchar_no++) {
	UCharImageType::Pointer uchar_img 
	    = ss_img_extract_uchar (im_in, uchar_no);
	UCharImageType::Pointer uchar_img_warped 
	    = itk_warp_image (uchar_img, vf, linear_interp, default_val);
	ss_img_insert_uchar (im_out, uchar_img_warped, uchar_no);
    }
    return im_out;
}

/* Explicit instantiations */
template PLMBASE_API UCharImageType::Pointer itk_warp_image (UCharImageType::Pointer im_in, DeformationFieldType::Pointer vf, int linear_interp, unsigned char default_val);
template PLMBASE_API UShortImageType::Pointer itk_warp_image (UShortImageType::Pointer im_in, DeformationFieldType::Pointer vf, int linear_interp, unsigned short default_val);
template PLMBASE_API ShortImageType::Pointer itk_warp_image (ShortImageType::Pointer im_in, DeformationFieldType::Pointer vf, int linear_interp, short default_val);
template PLMBASE_API UInt32ImageType::Pointer itk_warp_image (UInt32ImageType::Pointer im_in, DeformationFieldType::Pointer vf, int linear_interp, uint32_t default_val);
template PLMBASE_API FloatImageType::Pointer itk_warp_image (FloatImageType::Pointer im_in, DeformationFieldType::Pointer vf, int linear_interp, float default_val);
template PLMBASE_API DoubleImageType::Pointer itk_warp_image (DoubleImageType::Pointer im_in, DeformationFieldType::Pointer vf, int linear_interp, double default_val);
