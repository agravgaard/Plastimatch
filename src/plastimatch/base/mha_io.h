/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _mha_io_h_
#define _mha_io_h_

#include "plmbase_config.h"

class Volume;

PLMBASE_C_API Volume* read_mha (const char* filename);
PLMBASE_C_API void write_mha (const char* filename, const Volume* vol);

#endif
