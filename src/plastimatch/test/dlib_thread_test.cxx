#include <stdio.h>
#include "dlib_threads.h"

#if _WIN32
#include <windows.h>
#define plm_sleep(x) Sleep(x)
#else
#include <unistd.h>
#define plm_sleep(x) usleep(1000*x)
#endif


bool time_to_die = false;

void thread_func (void* param)
{
    Dlib_master_slave *s = (Dlib_master_slave *) param;
    
    while (1) {
        s->slave_grab_resource ();
        plm_sleep (300);
        printf ("Child execute\n");
        if (time_to_die) {
            break;
        }
        s->slave_release_resource ();
    }
}

int main ()
{
    Dlib_master_slave s;

    Dlib_thread_function tf (thread_func, &s);

    /* Parent and child execute simultaneously */
    for (int i = 0; i < 3; i++) {
        plm_sleep (770);
        printf ("Parent execute\n");
    }

    /* Only parent executes */
    printf ("Parent tries to grab...\n");
    s.master_grab_resource ();
    printf (">>> Parent only\n");
    for (int i = 0; i < 15; i++) {
        plm_sleep (70);
        printf ("Parent execute\n");
    }
    printf (">>> End parent only\n");
    s.master_release_resource ();

    /* Parent and child execute simultaneously */
    for (int i = 0; i < 3; i++) {
        plm_sleep (770);
        printf ("Parent execute\n");
    }

    time_to_die = true;

    return 0;
}
