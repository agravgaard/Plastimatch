/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _cxt_extract_h_
#define _cxt_extract_h_

#include "plmbase_config.h"

class Rtss;

template<class T> void cxt_extract (
        Rtss *cxt, 
        T image,
        int num_structs,
        bool check_cxt_bits
);

#endif
