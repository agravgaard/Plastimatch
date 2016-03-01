/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_region.h"

template<class T> 
RegionType
itk_image_region (const T& image)
{
    RegionType region = image->GetLargestPossibleRegion();
    IndexType index;
    index[0] = 0;
    index[1] = 0;
    index[2] = 0;
    region.SetIndex (index);
    return region;
}

template<class T> 
RegionType
itk_image_region (const T* image)
{
    RegionType region = image->GetLargestPossibleRegion();
    IndexType index;
    index[0] = 0;
    index[1] = 0;
    index[2] = 0;
    region.SetIndex (index);
    return region;
}

/* Explicit instantiations */
template PLMBASE_API RegionType itk_image_region (const CharImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const UCharImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const ShortImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const UShortImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const Int32ImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const UInt32ImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const FloatImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const DoubleImageType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const DeformationFieldType::Pointer& image);
template PLMBASE_API RegionType itk_image_region (const UCharVecImageType::Pointer& image);

template PLMBASE_API RegionType itk_image_region (const CharImageType* image);
template PLMBASE_API RegionType itk_image_region (const UCharImageType* image);
template PLMBASE_API RegionType itk_image_region (const ShortImageType* image);
template PLMBASE_API RegionType itk_image_region (const UShortImageType* image);
template PLMBASE_API RegionType itk_image_region (const Int32ImageType* image);
template PLMBASE_API RegionType itk_image_region (const UInt32ImageType* image);
template PLMBASE_API RegionType itk_image_region (const FloatImageType* image);
template PLMBASE_API RegionType itk_image_region (const DoubleImageType* image);
template PLMBASE_API RegionType itk_image_region (const DeformationFieldType* image);
template PLMBASE_API RegionType itk_image_region (const UCharVecImageType* image);
