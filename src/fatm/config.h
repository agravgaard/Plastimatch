/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef CONFIG_H
#define CONFIG_H

/* Disable MSVC8 warnings for using std C library */
#if defined (_MSC_VER)
#if ( _MSC_VER >= 1400 )
#pragma warning(disable : 4996)
#endif

/* Disable MSVC6 warnings about unknown pragmas (at least until I figure out how to guard the GCC pragma) */
#pragma warning(disable : 4068)

#endif

/* Disable gcc warnings when passing static (const) strings to fopen() and the like */
#pragma GCC diagnostic ignored "-Wwrite-strings"

#endif /* CONFIG_H */
