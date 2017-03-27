/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_parms_h_
#define _rt_parms_h_

#include "plmdose_config.h"
#include <string>
#include "plm_return_code.h"
#include "rt_plan.h"
#include "smart_pointer.h"
#include "threading.h"

class Plm_image;
class Rt_parms_private;
class Rt_plan;

class PLMDOSE_API Rt_parms
{
public:
    SMART_POINTER_SUPPORT (Rt_parms);
    Rt_parms_private *d_ptr;
public:
    Rt_parms ();
    Rt_parms (Rt_plan* rt_plan);
    ~Rt_parms ();

public:
    void set_rt_plan (Rt_plan *rt_plan);
    Plm_return_code set_command_file (const char *command_file);
    Plm_return_code parse_args (int argc, char** argv);
    Plm_return_code set_key_value (
        const std::string& section,
        const std::string& key, 
        // const std::string& index, 
        const std::string& val);

    void append_beam ();
    void append_peak ();


protected:
    void parse_config (const char* config_fn);
};

#endif
