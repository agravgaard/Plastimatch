/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _file_util_h_
#define _file_util_h_

#include "plmsys_config.h"
#include <string>
#include <stdio.h>
#include "plm_int.h"

PLMSYS_API int file_exists (const char *filename);
PLMSYS_API int file_exists (const std::string& filename);
PLMSYS_API uint64_t file_size (const char *filename);
PLMSYS_API int is_directory (const char *dir);
PLMSYS_API int is_directory (const std::string& dir);
PLMSYS_API void touch_file (const std::string& filename);
PLMSYS_API void copy_file (const std::string& dst_fn, 
    const std::string& src_fn);
PLMSYS_API void make_directory (const char *dirname);
PLMSYS_API void make_directory (const std::string& dirname);
PLMSYS_API void make_parent_directories (const char *filename);
PLMSYS_API void make_parent_directories (const std::string& filename);
PLMSYS_API void make_directory_recursive (const std::string& dirname);
PLMSYS_API FILE* make_tempfile (void);
PLMSYS_API char* plm_getcwd (char* s, int len);
PLMSYS_API int plm_chdir (char* s);
PLMSYS_API FILE* plm_fopen (const char *path, const char *mode);
PLMSYS_API FILE* plm_fopen (const std::string& path, const char *mode);

#endif
