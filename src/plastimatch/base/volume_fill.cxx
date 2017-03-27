/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "string.h"
#include "volume_fill.h"

template<class T> 
void
volume_fill (
    Volume* vol,
    T val
)
{
    T* img = vol->get_raw<T> ();

    for (plm_long i = 0; i < vol->npix; i++) {
        img[i] = val;
    }
}

/* Explicit instantiations */
template PLMBASE_API void volume_fill (Volume* vol, float val);
