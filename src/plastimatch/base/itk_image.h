/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_h_
#define _itk_image_h_

#include "plmbase_config.h"
#include "plm_int.h"

#include "itk_image_type.h"

class Plm_image_header;
class Volume_header;

/* Other types */
typedef itk::Matrix < double, 3, 3 > DirectionType;
typedef itk::Index < 3 >  IndexType;
typedef itk::Point < double, 3 >  OriginType;
typedef itk::ImageRegion < 3 > RegionType;
typedef itk::Size < 3 > SizeType;
typedef itk::Vector < double, 3 > SpacingType;
typedef itk::VariableLengthVector<unsigned char> UCharVecType;

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
PLMBASE_C_API void itk_image_get_props (
    const std::string& fileName,
    int *num_dimensions, 
    itk::ImageIOBase::IOPixelType *pixel_type, 
    itk::ImageIOBase::IOComponentType *component_type, 
    int *num_components
);

template<class T> PLMBASE_API void get_image_header (plm_long dim[3], float offset[3], float spacing[3], T image);
template<class T> PLMBASE_API void itk_image_get_volume_header (Volume_header *vh, T image);
template<class T> PLMBASE_API void itk_image_set_header (T image, const Plm_image_header *pih);
template<class T> PLMBASE_API void itk_image_set_header (T image, const Plm_image_header& pih);
template<class T, class U> PLMBASE_API void itk_image_header_copy (T dest, U src);
template<class T, class U> PLMBASE_API bool itk_image_header_compare (T image1, U image2);
template<class T> PLMBASE_API void itk_volume_center (float center[3], const T image);
template<class T> PLMBASE_API T itk_image_fix_negative_spacing (T img);

#endif
