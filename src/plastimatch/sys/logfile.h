/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _logfile_h_
#define _logfile_h_

#include "plmsys_config.h"

PLMSYS_API void logfile_open (const char* log_fn, const char* mode = "w");
PLMSYS_API void logfile_close (void);
PLMSYS_API void logfile_printf (const char* fmt, ...);
#define lprintf logfile_printf

#endif
