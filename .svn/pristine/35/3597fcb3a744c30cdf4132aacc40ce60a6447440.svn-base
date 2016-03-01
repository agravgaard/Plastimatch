/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "plm_image.h"
#include "plm_image_set.h"

class Plm_image_set_private {
public:
    std::list<Plm_image::Pointer> img_list;
};

Plm_image_set::Plm_image_set () {
    d_ptr = new Plm_image_set_private;
}

Plm_image_set::~Plm_image_set () {
    delete d_ptr;
}
