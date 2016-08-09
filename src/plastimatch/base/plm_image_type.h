/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_image_type_h_
#define _plm_image_type_h_

#include "plmbase_config.h"
#include <string>

/* We only deal with these kinds of images. */
enum Plm_image_type {
    PLM_IMG_TYPE_UNDEFINED, 
    PLM_IMG_TYPE_ITK_UCHAR, 
    PLM_IMG_TYPE_ITK_CHAR, 
    PLM_IMG_TYPE_ITK_USHORT, 
    PLM_IMG_TYPE_ITK_SHORT, 
    PLM_IMG_TYPE_ITK_ULONG, 
    PLM_IMG_TYPE_ITK_LONG, 
    PLM_IMG_TYPE_ITK_FLOAT, 
    PLM_IMG_TYPE_ITK_DOUBLE, 
    PLM_IMG_TYPE_ITK_FLOAT_FIELD, 
    PLM_IMG_TYPE_ITK_UCHAR_VEC,
    PLM_IMG_TYPE_GPUIT_UCHAR, 
    PLM_IMG_TYPE_GPUIT_UINT16, 
    PLM_IMG_TYPE_GPUIT_SHORT, 
    PLM_IMG_TYPE_GPUIT_UINT32,
    PLM_IMG_TYPE_GPUIT_INT32,
    PLM_IMG_TYPE_GPUIT_FLOAT, 
    PLM_IMG_TYPE_GPUIT_FLOAT_FIELD, 
    PLM_IMG_TYPE_GPUIT_LIST,
    PLM_IMG_TYPE_GPUIT_UCHAR_VEC
};

PLMBASE_API Plm_image_type plm_image_type_parse (const std::string& string);
PLMBASE_API Plm_image_type plm_image_type_parse (const char* string);
PLMBASE_API char* plm_image_type_string (Plm_image_type type);
PLMBASE_API char* plm_image_type_string_simple (Plm_image_type type);

#endif
