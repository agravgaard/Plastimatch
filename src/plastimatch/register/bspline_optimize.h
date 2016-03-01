/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_optimize_h_
#define _bspline_optimize_h_

#include "plmregister_config.h"
#include "bspline_state.h"
#include "smart_pointer.h"

class Bspline_optimize_private;
class Bspline_parms;
class Bspline_xform;

class PLMREGISTER_API Bspline_optimize {
public:
    SMART_POINTER_SUPPORT (Bspline_optimize);
    Bspline_optimize_private *d_ptr;
public:
    Bspline_optimize ();
    ~Bspline_optimize ();
public:
    Volume *fixed;
    Volume *moving;
    Volume *moving_grad;
public:
    void initialize (Bspline_xform *bxf, Bspline_parms *parms);

    /* Get raw objects, needed by CUDA code */
    Bspline_parms* get_bspline_parms ();
    Bspline_state* get_bspline_state ();
    Bspline_xform* get_bspline_xform ();
};

PLMREGISTER_C_API void bspline_optimize (
    Bspline_xform *bxf, 
    Bspline_parms *parms);

#endif
