/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

#include <stdlib.h>

#if defined (_WIN32)
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#endif

#include <stdio.h>  /* defines FILENAME_MAX */

int
main (int argc, char* argv[])
{
    #if defined (_WIN32)
    
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return errno;
    }
    printf(cCurrentPath);
    printf("\n");

    ShellExecute(0, "open", "cmd", 0, (LPCSTR)cCurrentPath, SW_SHOW);
    #else
    printf ("Sorry, CMD prompt launcher is only for windows system.\n");
    #endif
   
    return 0;
} 
