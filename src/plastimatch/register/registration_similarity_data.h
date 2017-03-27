/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _registration_similarity_data_h_
#define _registration_similarity_data_h_

#include "plmregister_config.h"

/*! \brief 
 * The Registration_similarity_data class holds original or processed 
 * images used for similarity measure calculations over multiple stages.
 */
class PLMREGISTER_API Registration_similarity_data
{
public:
    SMART_POINTER_SUPPORT (Registration_similarity_data);
public:
    Plm_image::Pointer fixed;
    Plm_image::Pointer moving;
    Plm_image::Pointer fixed_roi;
    Plm_image::Pointer moving_roi;
};

#endif
