/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "logfile.h"
#include "plm_version.h"

#define LOGFILE_ECHO_ON 1

FILE* log_fp = 0;

void
logfile_open (const char* log_fn, const char* mode)
{
    if (!log_fn[0]) return;
    if (!(log_fp)) {
	log_fp = fopen (log_fn, mode);
	if (!log_fp) {
	    /* If failure (e.g. bad path), do nothing */	
	}
    } else {
	/* Already open? */
    }
    logfile_printf ("Plastimatch " 
        PLASTIMATCH_VERSION_STRING
        "\n");
}

void
logfile_close (void)
{
    if (log_fp) {
	fclose (log_fp);
	log_fp = 0;
    }
}

void
logfile_printf (const char* fmt, ...)
{
    /* Write to console */
    if (LOGFILE_ECHO_ON) {
        va_list argptr;
        va_start (argptr, fmt);
	vprintf (fmt, argptr);
	fflush (stdout);
        va_end (argptr);
    }

    if (log_fp) {
        va_list argptr;
        va_start (argptr, fmt);
        vfprintf (log_fp, fmt, argptr);
        fflush (log_fp);
	va_end (argptr);
    }
}
