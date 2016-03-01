/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_stage_h_
#define _bspline_stage_h_

#include "plmregister_config.h"
#include "xform.h"

class Bspline_stage_private;
class Registration_parms;
class Registration_data;
class Stage_parms;
class Volume;

class PLMREGISTER_API Bspline_stage {
public:
    Bspline_stage_private *d_ptr;
public:
    Bspline_stage (
        Registration_parms *regp, 
        Registration_data *regd, 
        const Stage_parms *stage, 
        Xform *xf_in);
    ~Bspline_stage ();

public:
    void run_stage ();
protected:
    void initialize ();
    void cleanup ();
};

Xform::Pointer
do_gpuit_bspline_stage (
    Registration_parms* regp, 
    Registration_data* regd, 
    const Xform::Pointer& xf_in, 
    const Stage_parms* stage);

#endif
