/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_load.txx"

Int32ImageType::Pointer
itk_image_load_int32 (const char* fname, Plm_image_type* original_type)
{
    Int32ImageType::Pointer img;

    /* If it is directory, then must be dicom */
    if (is_directory(fname)) {
	img = load_dicom_int32 (fname);
    } else {
	img = itk_image_load_any (fname, original_type, static_cast<int32_t>(0));
    }
    return itk_image_load_postprocess (img);
}
