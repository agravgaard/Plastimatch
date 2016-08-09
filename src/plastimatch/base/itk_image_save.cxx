/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "itkCastImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkOrientImageFilter.h"

#include "file_util.h"
#include "itk_dicom_save.h"
#include "itk_image_cast.h"
#include "itk_image_save.h"
#include "logfile.h"
#include "print_and_exit.h"
#include "path_util.h"

/* -----------------------------------------------------------------------
   Writing image files
   ----------------------------------------------------------------------- */
void itk_image_save (const FloatImageType::Pointer& img_ptr, 
    const std::string& fname, Plm_image_type image_type)
{
    itk_image_save (img_ptr, fname.c_str(), image_type);
}

void itk_image_save (const FloatImageType::Pointer& img_ptr, 
    const char* fname, Plm_image_type image_type)
{
    switch (image_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
        itk_image_save_uchar (img_ptr, fname);
        break;
    case PLM_IMG_TYPE_ITK_SHORT:
        itk_image_save_short (img_ptr, fname);
        break;
    case PLM_IMG_TYPE_ITK_USHORT:
        itk_image_save_ushort (img_ptr, fname);
        break;
    case PLM_IMG_TYPE_ITK_LONG:
        itk_image_save_int32 (img_ptr, fname);
        break;
    case PLM_IMG_TYPE_ITK_ULONG:
        itk_image_save_uint32 (img_ptr, fname);
        break;
    case PLM_IMG_TYPE_ITK_FLOAT:
        itk_image_save_float (img_ptr, fname);
        break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
        itk_image_save_double (img_ptr, fname);
        break;
    default:
        print_and_exit ("Output type is not supported.\n");
        break;
    }
}

template<class T> 
void
itk_image_save (T image, const char* fname)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ImageFileWriter< ImageType >  WriterType;

    logfile_printf ("Trying to write image to %s\n", fname);

    typename WriterType::Pointer writer = WriterType::New();
    writer->SetInput (image);
    writer->SetFileName (fname);
    make_parent_directories (fname);

    if (extension_is (fname, "nrrd")) {
	writer->SetUseCompression (true);
    }
    try {
	writer->Update();
    }
    catch (itk::ExceptionObject& excp) {
	printf ("ITK exception writing image file.\n");
	std::cout << excp << std::endl;
    }
}

template<class T> 
void
itk_image_save (T image, const std::string& fname)
{
    itk_image_save (image, fname.c_str());
}

template<class T> 
void
itk_image_save_char (T image, const char* fname)
{
    CharImageType::Pointer char_img = cast_char (image);
    itk_image_save (char_img, fname);
}

template<class T> 
void
itk_image_save_uchar (T image, const char* fname)
{
    UCharImageType::Pointer uchar_img = cast_uchar (image);
    itk_image_save (uchar_img, fname);
}

template<class T> 
void
itk_image_save_short (T image, const char* fname)
{
    ShortImageType::Pointer short_img = cast_short (image);
    itk_image_save (short_img, fname);
}

template<class T> 
void
itk_image_save_ushort (T image, const char* fname)
{
    UShortImageType::Pointer ushort_img = cast_ushort (image);
    itk_image_save (ushort_img, fname);
}

template<class T> 
void
itk_image_save_short_dicom (
    T image, 
    const char* dir_name, 
    Rt_study_metadata *rsm
)
{
    ShortImageType::Pointer short_img = cast_short (image);
    itk_dicom_save (short_img, dir_name, rsm);
}


template<class T> 
void
itk_image_save_int32 (T image, const char* fname)
{
    Int32ImageType::Pointer int32_img = cast_int32 (image);
    itk_image_save (int32_img, fname);
}

template<class T> 
void
itk_image_save_uint32 (T image, const char* fname)
{
    UInt32ImageType::Pointer uint32_img = cast_uint32 (image);
    itk_image_save (uint32_img, fname);
}

template<class T> 
void
itk_image_save_float (T image, const char* fname)
{
    FloatImageType::Pointer float_img = cast_float (image);
    itk_image_save (float_img, fname);
}

template<class T> 
void
itk_image_save_double (T image, const char* fname)
{
    DoubleImageType::Pointer double_img = cast_double (image);
    itk_image_save (double_img, fname);
}

/* Explicit instantiations */
template PLMBASE_API void itk_image_save(CharImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save(UCharImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save(ShortImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save(UShortImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save(Int32ImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save(UInt32ImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save(FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save(DoubleImageType::Pointer, const char*);

template PLMBASE_API void itk_image_save(DeformationFieldType::Pointer, const char*);
template PLMBASE_API void itk_image_save(UCharImage4DType::Pointer, const char*);
template PLMBASE_API void itk_image_save(UCharVecImageType::Pointer, const char*);

template PLMBASE_API void itk_image_save(CharImageType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(UCharImageType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(ShortImageType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(UShortImageType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(Int32ImageType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(UInt32ImageType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(FloatImageType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(DoubleImageType::Pointer, const std::string&);

template PLMBASE_API void itk_image_save(DeformationFieldType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(UCharImage4DType::Pointer, const std::string&);
template PLMBASE_API void itk_image_save(UCharVecImageType::Pointer, const std::string&);

template PLMBASE_API void itk_image_save_char (FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save_uchar (FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save_short (FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save_ushort (FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save_int32 (FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save_uint32 (FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save_float (FloatImageType::Pointer, const char*);
template PLMBASE_API void itk_image_save_double (FloatImageType::Pointer, const char*);

template PLMBASE_API void itk_image_save_short_dicom (UCharImageType::Pointer, const char*, Rt_study_metadata *rsm);
template PLMBASE_API void itk_image_save_short_dicom (ShortImageType::Pointer, const char*, Rt_study_metadata *rsm);
template PLMBASE_API void itk_image_save_short_dicom (UShortImageType::Pointer, const char*, Rt_study_metadata *rsm);
template PLMBASE_API void itk_image_save_short_dicom (UInt32ImageType::Pointer, const char*, Rt_study_metadata *rsm);
template PLMBASE_API void itk_image_save_short_dicom (FloatImageType::Pointer, const char*, Rt_study_metadata *rsm);
