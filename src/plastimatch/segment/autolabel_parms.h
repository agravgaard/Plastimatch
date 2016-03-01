/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _autolabel_parms_h_
#define _autolabel_parms_h_

#include "plmsegment_config.h"
#include <list>
#include <map>
#include <string>
#include "plm_return_code.h"
#include "smart_pointer.h"

class Autolabel_feature;
class Autolabel_parms_private;

class PLMSEGMENT_API Autolabel_parms {
public:
    SMART_POINTER_SUPPORT (Autolabel_parms);
    Autolabel_parms_private *d_ptr;
public:
    Autolabel_parms ();
    ~Autolabel_parms ();

public:
    void parse_command_file ();
    Plm_return_code set_key_value (
        const std::string& section,
        const std::string& key, 
        const std::string& val);

public:
    std::string cmd_file_fn;
    std::string input_fn;
    std::string network_dir;
    std::string output_csv_fn;
    std::string output_fcsv_fn;
    std::string task;
    bool enforce_anatomic_constraints;
};

#endif
