/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_origin.h"

template<class T> 
OriginType
itk_image_origin (const T& image)
{
    OriginType origin;
    image->TransformIndexToPhysicalPoint (
 	image->GetLargestPossibleRegion().GetIndex(), origin);
    return origin;
}

template<class T> 
OriginType
itk_image_origin (const T* image)
{
    OriginType origin;
    image->TransformIndexToPhysicalPoint (
 	image->GetLargestPossibleRegion().GetIndex(), origin);
    return origin;
}

/* GCS FIX: The below does not work, because OriginType is a 4D vector */
//template PLMBASE_API OriginType itk_image_origin (const UCharImage4DType::Pointer& image);


/* Explicit instantiations */
template PLMBASE_API OriginType itk_image_origin (const CharImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const UCharImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const ShortImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const UShortImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const Int32ImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const UInt32ImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const FloatImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const DoubleImageType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const DeformationFieldType::Pointer& image);
template PLMBASE_API OriginType itk_image_origin (const UCharVecImageType::Pointer& image);

template PLMBASE_API OriginType itk_image_origin (const CharImageType* image);
template PLMBASE_API OriginType itk_image_origin (const UCharImageType* image);
template PLMBASE_API OriginType itk_image_origin (const ShortImageType* image);
template PLMBASE_API OriginType itk_image_origin (const UShortImageType* image);
template PLMBASE_API OriginType itk_image_origin (const Int32ImageType* image);
template PLMBASE_API OriginType itk_image_origin (const UInt32ImageType* image);
template PLMBASE_API OriginType itk_image_origin (const FloatImageType* image);
template PLMBASE_API OriginType itk_image_origin (const DoubleImageType* image);
template PLMBASE_API OriginType itk_image_origin (const DeformationFieldType* image);
template PLMBASE_API OriginType itk_image_origin (const UCharVecImageType* image);
