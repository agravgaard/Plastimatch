/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_load.txx"

CharImageType::Pointer
itk_image_load_char (const char* fname, Plm_image_type* original_type)
{
    CharImageType::Pointer img;

    /* If it is directory, then must be dicom */
    if (is_directory(fname)) {
	img = load_dicom_char (fname);
    } else {
	img = itk_image_load_any (fname, original_type, 
	    static_cast<char>(0));
    }
    return itk_image_load_postprocess (img);
}
