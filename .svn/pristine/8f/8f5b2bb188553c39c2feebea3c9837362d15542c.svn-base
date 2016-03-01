/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _cxt_io_h_
#define _cxt_io_h_

#include "plmbase_config.h"
#include "rt_study_metadata.h"

class Metadata;
class Rtss;

PLMBASE_API Rtss* cxt_load_ss_list (
    Rtss* cxt,
    const char* xorlist_fn
);
PLMBASE_API void cxt_load (
    Rtss *cxt,                  /* Output: load into this object */
    Rt_study_metadata *rsm,     /* Output: load into this object */
    const char *cxt_fn          /* Input: file to load from */
);
PLMBASE_API void cxt_save (
    Rtss *cxt,                             /* In: save this object */
    const Rt_study_metadata::Pointer& rsm, /* In: save this object */
    const char* cxt_fn,                    /* In: File to save to */
    bool prune_empty                       /* In: Prune empty structures? */
);

#endif
