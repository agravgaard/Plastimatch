/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   Apparently timing things is not easy.  See these pages:
   http://www.virtualdub.org/blog/pivot/entry.php?id=106
   http://support.microsoft.com/kb/274323
   http://support.microsoft.com/kb/895980
   ----------------------------------------------------------------------- */
#if defined _WIN32
#include <windows.h>
#endif
#include "timer.h"

#if defined _WIN32
LARGE_INTEGER clock_freq;
double timestamp_ref;
#endif

void
static_timer_reset (void)
{
#if defined _WIN32
    LARGE_INTEGER clock_count;
    QueryPerformanceFrequency (&clock_freq);
    QueryPerformanceCounter (&clock_count);
    timestamp_ref = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
#endif
}

double
static_timer_get_time (void)
{
#if defined _WIN32
    LARGE_INTEGER clock_count;
    double timestamp_2;

    QueryPerformanceCounter (&clock_count);
    timestamp_2 = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
    return timestamp_2 - timestamp_ref;
#else
    return 0.0;
#endif
}
