/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include "viscous_cuda.h"
#include "viscous_main.h"

int
viscous_main (int argc, char *argv[])
{
    LOAD_LIBRARY_SAFE (libplmregistercuda);
    LOAD_SYMBOL (CUDA_viscous_main, libplmregistercuda);
    int rc = CUDA_viscous_main (argc, argv);
    UNLOAD_LIBRARY (libplmregistercuda);
    return rc;
}
