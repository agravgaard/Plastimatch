/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _registration_parms_h_
#define _registration_parms_h_

#include "plmregister_config.h"
#include <list>
#include <string>
#include <ctype.h>
#include <stdlib.h>
#include "bspline.h"    /* for enums */
#include "plm_image_type.h"
#include "plm_return_code.h"
#include "smart_pointer.h"
#include "threading.h"

class Plm_image;
class Registration_parms_private;
class Shared_parms;
class Stage_parms;

class PLMREGISTER_API Registration_parms {
public:
    SMART_POINTER_SUPPORT (Registration_parms);
    Registration_parms_private *d_ptr;
public:
    int num_stages;
    int img_out_fmt;
    Plm_image_type img_out_type;
    std::string img_out_fn;
    std::string xf_in_fn;
    bool xf_out_itk;
    std::list<std::string> xf_out_fn;
    std::string vf_out_fn;
    std::string log_fn;
    float default_value;           /* Replacement when out-of-view */
    int init_type;
    double init[12];

    /* for 4D and atlas */
    std::string moving_dir;
    std::string fixed_dir;
    std::string img_out_dir;
    std::string vf_out_dir;
    int job_idx;
    int num_jobs;

public:
    Registration_parms();
    ~Registration_parms();
public:
    int set_command_string (const std::string& command_string);
    Plm_return_code set_key_value (
        const std::string& section,
        const std::string& key, 
        const std::string& val);
    int parse_command_file (const char* options_fn);
    void set_job_paths (void);
public:
    const std::string& get_fixed_fn ();
    const std::string& get_moving_fn ();
    Shared_parms* get_shared_parms ();
    void delete_all_stages ();
    std::list<Stage_parms*>& get_stages ();
    Stage_parms* append_stage ();
    Stage_parms* append_process_stage ();
};

#endif
