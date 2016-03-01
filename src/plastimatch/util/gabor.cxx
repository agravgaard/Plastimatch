/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include "fftw3.h"

#include "gabor.h"
#include "direction_cosines.h"
#include "plm_image_header.h"

class Gabor_private
{
public:
    Plm_image_header pih;
public:
    Gabor_private () {
        plm_long dim[3];
        float origin[3];
        float spacing[3];
        Direction_cosines dc;
        for (int d = 0; d < 3; d++) {
            dim[d] = 10;
            origin[d] = 0.f;
            spacing[d] = 1.f;
        }
        this->pih.set (dim, origin, spacing, dc);
    }
};
    
Gabor::Gabor ()
{
    d_ptr = new Gabor_private;
}

Gabor::~Gabor ()
{
    delete d_ptr;
}

Plm_image::Pointer
Gabor::get_filter ()
{
    Plm_image::Pointer f = Plm_image::New (
        PLM_IMG_TYPE_GPUIT_FLOAT, d_ptr->pih);
    float *img = f->get_volume_float()->get_raw<float> ();

    UNUSED_VARIABLE (img);

    return f;
}
