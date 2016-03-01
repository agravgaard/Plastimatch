/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _option_range_h_
#define _option_range_h_

#include "plmsys_config.h"
#include <list>
#include <string>

class Option_range_private;

class PLMSYS_API Option_range {

public:
    Option_range ();
    ~Option_range ();
public:
    void set_log_range (const std::string& range);
    void set_linear_range (const std::string& range);
    void set_range (const std::string& range);
    void set_range (float range);
    const std::list<float>& get_range ();

public:
    Option_range_private *d_ptr;
};

#endif
