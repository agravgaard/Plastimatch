/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_load.txx"

FloatImageType::Pointer
itk_image_load_float (const char* fname, Plm_image_type* original_type)
{
    FloatImageType::Pointer img;

    /* If it is directory, then must be dicom */
    if (is_directory(fname)) {
	img = load_dicom_float (fname);
    } else {
	img = itk_image_load_any (fname, original_type, static_cast<float>(0));
    }
    return itk_image_load_postprocess (img);
}

FloatImageType::Pointer
itk_image_load_float (const std::string& fname, Plm_image_type* original_type)
{
    return itk_image_load_float (fname.c_str(), original_type);
}
