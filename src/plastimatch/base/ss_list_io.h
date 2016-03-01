/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _ss_list_io_h_
#define _ss_list_io_h_

#include "plmbase_config.h"

class Rtss;

PLMBASE_API Rtss* ss_list_load (
        Rtss* cxt,
        const char* ss_list_fn
);
PLMBASE_API void ss_list_save (
        Rtss* cxt,
        const char* cxt_fn
);
PLMBASE_API void ss_list_save_colormap (
        Rtss* cxt,
        const char* colormap_fn
);

#endif
