/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>

#include "opencl_probe.h"
#include "delayload.h"


int
main (int argc, char* argv[])
{
    int opencl_works;

    LOAD_LIBRARY_SAFE (libplmopencl);
    LOAD_SYMBOL (opencl_probe, libplmopencl);

    opencl_works = opencl_probe ();

    if (opencl_works) {
        printf ("Opencl works ok.\n");
    } else {
        printf ("Opencl does not work.\n");
    }

    UNLOAD_LIBRARY (libplmopencl);

    return 0;
}
