/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "logfile.h"
#include "plm_exception.h"
#include "print_and_exit.h"
#include "string_util.h"

#if (defined(_WIN32) || defined(WIN32))
	#define vsnprintf _vsnprintf
#endif

void
print_and_exit (const char* prompt_fmt, ...)
{
    if (prompt_fmt) {
        va_list argptr;
        va_start (argptr, prompt_fmt);
        std::string error_message = string_format_va (prompt_fmt, argptr);
        lprintf ("%s", error_message.c_str());
        Plm_exception pe = Plm_exception (error_message);
        va_end (argptr);
        throw pe;
    }
    throw Plm_exception ("Plastimatch: unknown error.");
}
