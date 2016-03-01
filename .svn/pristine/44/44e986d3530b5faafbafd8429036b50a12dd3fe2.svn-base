/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_load_h_
#define _itk_image_load_h_

#include "plmbase_config.h"
#include "itk_image_type.h"
#include "plm_image_type.h"

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
PLMBASE_API CharImageType::Pointer itk_image_load_char (const char* fname, Plm_image_type* original_type);
PLMBASE_API UCharImageType::Pointer itk_image_load_uchar (const char* fname, Plm_image_type* original_type);
PLMBASE_API UCharImageType::Pointer itk_image_load_uchar (const std::string& fname, Plm_image_type* original_type);
PLMBASE_API ShortImageType::Pointer itk_image_load_short (const char* fname, Plm_image_type* original_type);
PLMBASE_API UShortImageType::Pointer itk_image_load_ushort (const char* fname, Plm_image_type* original_type);
PLMBASE_API Int32ImageType::Pointer itk_image_load_int32 (const char* fname, Plm_image_type* original_type);
PLMBASE_API UInt32ImageType::Pointer itk_image_load_uint32 (const char* fname, Plm_image_type* original_type);
PLMBASE_API FloatImageType::Pointer itk_image_load_float (const char* fname, Plm_image_type* original_type);
PLMBASE_API FloatImageType::Pointer itk_image_load_float (const std::string& fname, Plm_image_type* original_type);
PLMBASE_API DoubleImageType::Pointer itk_image_load_double (const char* fname, Plm_image_type* original_type);

PLMBASE_API DeformationFieldType::Pointer itk_image_load_float_field (const char* fname);
PLMBASE_API DeformationFieldType::Pointer itk_image_load_float_field (const std::string& fname);
PLMBASE_API UCharVecImageType::Pointer itk_image_load_uchar_vec (const char* fname);

template<class T> PLMBASE_API void get_image_header (int dim[3], float offset[3], float spacing[3], T image);

#endif
