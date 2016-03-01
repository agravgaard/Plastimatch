/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _registration_h_
#define _registration_h_

#include "plmregister_config.h"
#include "registration_data.h"
#include "registration_parms.h"
#include "xform.h"

class Registration_private;

class PLMREGISTER_API Registration {
public:
    SMART_POINTER_SUPPORT (Registration);
    Registration_private *d_ptr;
public:
    Registration ();
    ~Registration ();
public:
    int set_command_file (const std::string& command_file);
    int set_command_string (const std::string& command_string);
    void set_fixed_image (Plm_image::Pointer& fixed);
    void set_moving_image (Plm_image::Pointer& moving);
    void set_fixed_roi (Plm_image::Pointer& fixed_roi);
    void set_moving_roi (Plm_image::Pointer& moving_roi);

    Registration_data::Pointer get_registration_data ();
    Registration_parms::Pointer get_registration_parms ();

    /* New API */
    void load_global_inputs ();
    void start_registration ();
    void pause_registration ();
    void resume_registration ();
    void wait_for_complete ();

    /* Wrapper around new API, to emulate old API */
    void do_registration ();
    Xform::Pointer do_registration_pure ();

    Xform::Pointer get_current_xform ();
    void save_global_outputs ();

    /* This is called by worker thread */
    void run_main_thread ();

protected:
    Xform::Pointer do_registration_stage (Stage_parms* stage);
};

#endif
