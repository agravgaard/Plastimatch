/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "plm_sleep.h"

void 
plm_sleep (int milliseconds)
{
#if (_WIN32)
    Sleep (milliseconds);
#else
    usleep (1000 * milliseconds);
#endif
}
