/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _demons_state_h_
#define _demons_state_h_

#include "plmregister_config.h"

class Volume;
class Demons_parms;

class PLMREGISTER_API Demons_state {
  public:
    Volume *vf_smooth;
    Volume *vf_est;
  public:
    Demons_state (void);
    ~Demons_state (void);
    void init (
	Volume* fixed, 
	Volume* moving, 
	Volume* moving_grad, 
	Volume* vf_init, 
	Demons_parms* parms);
};

#endif
