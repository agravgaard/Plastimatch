/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _print_and_exit_h_
#define _print_and_exit_h_

#include "plmsys_config.h"

PLMSYS_API void print_and_exit (const char* prompt_fmt, ...);
#define error_printf(fmt, ...) \
    fprintf (stderr, "\nplastimatch has encountered an issue.\n" \
             "file: %s (line:%i)\n" fmt, __FILE__, __LINE__,##__VA_ARGS__)


#endif
