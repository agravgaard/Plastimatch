/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_load.txx"

UShortImageType::Pointer
itk_image_load_ushort (const char* fname, Plm_image_type* original_type)
{
    UShortImageType::Pointer img;

    /* If it is directory, then must be dicom */
    if (is_directory(fname)) {
	img = load_dicom_ushort (fname);
    } else {
	img = itk_image_load_any (fname, original_type, static_cast<unsigned short>(0));
    }
    return itk_image_load_postprocess (img);
}
