/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itk_image_load.txx"

UCharVecImageType::Pointer
itk_image_load_uchar_vec (const char* fname)
{
    UCharVecImageType::Pointer img 
	= itk_image_load<UCharVecImageType> (fname);
    return itk_image_load_postprocess (img);
}

DeformationFieldType::Pointer
itk_image_load_float_field (const char* fname)
{
    DeformationFieldType::Pointer img 
	= itk_image_load<DeformationFieldType> (fname);
    return itk_image_load_postprocess (img);
}

DeformationFieldType::Pointer
itk_image_load_float_field (const std::string& fname)
{
    return itk_image_load_float_field (fname.c_str());
}
