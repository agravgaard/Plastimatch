/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdarg.h>
#include <stdio.h>
#include <QString>
#include <QTime>


/* Just a printf, but also flushes the stream */
int 
aqprintf (const char * format, ...)
{   
    va_list argptr;
    int rc;   

    va_start (argptr, format);
    rc = vprintf (format, argptr);

    va_end (argptr);
    fflush (stdout);
    return rc;
}