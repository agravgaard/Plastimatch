/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_series_h_
#define _plm_series_h_

#include "plmbase_config.h"

class Plm_series_private;

class Plm_series 
{
public:
    Plm_series ();
    ~Plm_series ();

public:
    Plm_series_private *d_ptr;

public:
    void debug (void) const;
};

#endif
