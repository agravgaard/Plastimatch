/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _mc_dose_h_
#define _mc_dose_h_

#include "plmbase_config.h"

class Plm_image;
class Xio_ct_transform;

PLMBASE_API void mc_dose_load (Plm_image *plm, const char *filename);
PLMBASE_API void mc_dose_apply_transform (Plm_image *plm, Xio_ct_transform *transform);

#endif
