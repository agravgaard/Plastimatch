/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xio_dose_h_
#define _xio_dose_h_

#include "plmbase_config.h"

class Metadata;
class Plm_image;
class Xio_ct_transform;

PLMBASE_API void xio_dose_load (
    Plm_image *plm,
    Metadata::Pointer& meta,
    const char *filename
);
PLMBASE_API void xio_dose_save (
    const Plm_image::Pointer& plm,
    Metadata::Pointer& meta,
    Xio_ct_transform *transform,
    const char *filename,
    const char *filename_template
);
PLMBASE_API void xio_dose_apply_transform (
    Plm_image *plm,
    Xio_ct_transform *transform
);

#endif
