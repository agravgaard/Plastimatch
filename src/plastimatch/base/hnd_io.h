/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _hnd_io_h_
#define _hnd_io_h_

#include "plmbase_config.h"

class Proj_image;

PLMBASE_C_API void hnd_load (
        Proj_image *proj,
        const char *fn,
        const double xy_offset[2]
);


#endif
