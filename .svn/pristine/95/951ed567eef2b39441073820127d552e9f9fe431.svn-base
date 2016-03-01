/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _path_util_h_
#define _path_util_h_

#include "plmsys_config.h"
#include <string>
#include <stdio.h>

#if (_WIN32)
#define ISSLASH(c) (((c) == '/') || ((c) == '\\'))
#else
#define ISSLASH(c) ((c) == '/')
#endif

PLMSYS_API std::string basename (const std::string& fn);
PLMSYS_API std::string dirname (const std::string& fn);
PLMSYS_API int extension_is (const char* fname, const char* ext);
PLMSYS_API int extension_is (const std::string& fname, const char* ext);
PLMSYS_API void strip_extension (char* filename);
PLMSYS_API std::string strip_extension (const std::string& filename);
PLMSYS_API void trim_trailing_slashes (char *pathname);
PLMSYS_API std::string trim_trailing_slashes (const std::string& pathname);
PLMSYS_API char* file_util_parent (const char *filename);
PLMSYS_API std::string file_util_parent (const std::string& filename);
PLMSYS_API char* file_util_dirname (const char *filename);
PLMSYS_API std::string file_util_dirname_string (const char *filename);
PLMSYS_API std::string strip_leading_dir (const std::string& fn);
PLMSYS_API std::string compose_filename (const std::string& a, 
    const std::string& b);
PLMSYS_API std::string compose_filename (const char *a, const char *b);
PLMSYS_API std::string make_windows_slashes (const std::string& s);

#endif
