/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_image_p_h_
#define _plm_image_p_h_

#include "plmbase_config.h"
#include "volume.h"

class Plm_image_private {
public:
    Metadata::Pointer m_meta;
    Volume::Pointer m_vol;
};

#endif
