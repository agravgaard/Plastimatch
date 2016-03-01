/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

class Plm_timer_private {
public:
    Plm_timer_private ();
public:
    bool running;
    double start_time;
    double acc_time;
#ifdef _WIN32
    LARGE_INTEGER clock_freq;
#endif
public:
    double get_time ();
    double elapsed_time ();
};
