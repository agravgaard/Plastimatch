/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_save_h_
#define _itk_image_save_h_

#include "plmbase_config.h"
#include "itk_image_type.h"
#include "plm_image_type.h"

class Metadata;
class Rt_study_metadata;
class Slice_index;

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
PLMBASE_API void itk_image_save (const FloatImageType::Pointer& img_ptr, 
    const std::string& fname, Plm_image_type image_type);
PLMBASE_API void itk_image_save (const FloatImageType::Pointer& img_ptr, 
    const char* fname, Plm_image_type image_type);
template<class T> PLMBASE_API void itk_image_save (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save (T img_ptr, const std::string& fname);
template<class T> PLMBASE_API void itk_image_save_short_dicom (T image, const char* dir_name, Rt_study_metadata *);

template<class T> PLMBASE_API void itk_image_save_char (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save_uchar (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save_short (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save_ushort (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save_int32 (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save_uint32 (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save_float (T img_ptr, const char* fname);
template<class T> PLMBASE_API void itk_image_save_double (T img_ptr, const char* fname);
#endif
