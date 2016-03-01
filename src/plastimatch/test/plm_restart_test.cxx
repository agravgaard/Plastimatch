#include <stdio.h>

#include "plm_sleep.h"
#include "registration.h"
#include "rt_study.h"
#include "synthetic_mha.h"

int main ()
{
    Registration r;
    std::string s = 
        "[GLOBAL]\n"
#if defined (commentout)
        "fixed=c:/tmp/fixed.mha\n"
        "moving=c:/tmp/moving.mha\n"
        "fixed=/home/gcs6/tmp/fixed_1.mha\n"
        "moving=/home/gcs6/tmp/moving.mha\n"
#endif
        "fixed=/home/gcs6/tmp/fixed.mha\n"
        "moving=/home/gcs6/tmp/moving_1.mha\n"
        "[STAGE]\n"
        "xform=bspline\n"
        "max_its=1\n"
        "flavor=c\n"
#if defined (commentout)
        "max_its=100\n"
#endif

        "[STAGE]\n"
        ;
    r.set_command_string (s);

    Plm_image::Pointer fixed;
    float origin[3];

    for (int i = 0; i < 3; i++) origin[i] = -245.5;   /* Fast */
    for (int i = 0; i < 3; i++) origin[i] = 0;        /* Slow */

    {
    Rt_study rtds;
    Synthetic_mha_parms sm_parms;
    sm_parms.pattern = PATTERN_RECT;
    for (int i = 0; i < 3; i++) {sm_parms.origin[i] = origin[i];}
    synthetic_mha (&rtds, &sm_parms);
    fixed = rtds.get_image();
    }
    Plm_image::Pointer moving;
    {
    Rt_study rtds;
    Synthetic_mha_parms sm_parms;
    sm_parms.pattern = PATTERN_SPHERE;
    for (int i = 0; i < 3; i++) {sm_parms.origin[i] = origin[i];}
    synthetic_mha (&rtds, &sm_parms);
    moving = rtds.get_image();
    }
    
//    fixed->save_image ("/home/gcs6/tmp/fixed_1.mha");
//    moving->save_image ("/home/gcs6/tmp/moving_1.mha");

#if defined (commentout)
    r.load_global_inputs ();
#endif

    r.set_fixed_image (fixed);
    r.set_moving_image (moving);

    printf ("Calling start_registration\n");

#if defined (commentout)    
    r.start_registration ();
    plm_sleep (1000);
    printf (">>> PAUSE\n");
    r.pause_registration ();
    printf (">>> PAUSE RETURNED\n");
    plm_sleep (3000);
    printf (">>> PAUSE COMPLETE\n");
    r.resume_registration ();
    r.wait_for_complete ();
#endif

#if defined (commentout)    
    r.do_registration_pure_old ();
#endif

    return 0;
}
