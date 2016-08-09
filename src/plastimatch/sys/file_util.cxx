/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if (defined(_WIN32) || defined(WIN32))
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>    /* sleep() */
#include <dirent.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "file_util.h"
#include "path_util.h"

int
is_directory (const char *dir)
{
#if (defined(_WIN32) || defined(WIN32))
    char *pwd;
    if ((pwd = _getcwd (NULL, 0)) == NULL) {
        return 0;
    }
    if (_chdir (dir) == -1) {
        free (pwd);
        return 0;
    }
    _chdir (pwd);
    free (pwd);
    return 1;
#else /* UNIX */
    DIR *dp;
    if ((dp = opendir (dir)) == NULL) {
        return 0;
    }
    closedir (dp);
    return 1;
#endif
}

int
is_directory (const std::string& dir)
{
    return is_directory (dir.c_str());
}

int
file_exists (const char *filename)
{
    FILE *f = fopen (filename, "r");
    if (f) {
	fclose (f);
	return 1;
    }
    return 0;
}

int
file_exists (const std::string& filename)
{
    return file_exists (filename.c_str());
}

uint64_t
file_size (const char *filename)
{
    struct stat fs;
    if (stat (filename, &fs) != 0) return 0;

    return (uint64_t) fs.st_size;
}

void 
touch_file (const std::string& filename)
{
    make_parent_directories (filename);
    FILE *fp = fopen (filename.c_str(), "w");
    fclose (fp);
}

/* N.b. there might be a faster way for MSVC debug mode:
   http://bytes.com/topic/c/answers/62145-filecopy-std-copy
*/
void
copy_file (const std::string& dst_fn, const std::string& src_fn)
{
    std::ifstream src (src_fn.c_str(), std::ios::binary);
    std::ofstream dst (dst_fn.c_str(), std::ios::binary);

    dst << src.rdbuf();
}

void
make_directory (const char *dirname)
{
    int retries = 4;

#if (_WIN32)
    mkdir (dirname);
#else
    mkdir (dirname, 0777);
#endif

    /* On various samba mounts, there is a delay in creating the directory. 
       Here, we work around that problem by waiting until the directory 
       is created */
    while (--retries > 0 && !is_directory (dirname)) {
#if (_WIN32)
	Sleep (1000);
#else
	sleep (1);
#endif
    }
}

void
make_directory (const std::string& dirname)
{
    make_directory (dirname.c_str());
}

void
make_parent_directories (const char *filename)
{
    char *p, *tmp;

    if (!filename) return;

    tmp = strdup (filename);
    p = tmp;
    while (*p) {
	if (ISSLASH (*p) && p != tmp) {
	    *p = 0;
	    make_directory (tmp);
	    *p = '/';
	}
	p++;
    }
    free (tmp);
}

void
make_parent_directories (const std::string& filename)
{
    make_parent_directories (filename.c_str());
}

void
make_directory_recursive (const std::string& dirname)
{
    make_parent_directories (dirname);
    make_directory (dirname.c_str());
}

FILE*
plm_fopen (const char *path, const char *mode)
{
    if (mode && (mode[0] == 'w' || mode[0] == 'a')) {
        make_parent_directories (path);
    }
    return fopen (path, mode);
}

FILE*
plm_fopen (const std::string& path, const char *mode)
{
    return plm_fopen (path.c_str(), mode);
}

FILE*
make_tempfile (void)
{
# if defined (_WIN32)
    /* tmpfile is broken on windows.  It tries to create the 
	temorary files in the root directory where it doesn't 
	have permissions. 
	http://msdn.microsoft.com/en-us/library/x8x7sakw(VS.80).aspx */

    /* GCS FIX: The temporary file doesn't get deleted on windows */
    char *parms_fn = _tempnam (0, "plastimatch_");
    FILE *fp = fopen (parms_fn, "wb+");
# else
    FILE *fp = tmpfile ();
# endif
    return fp;
}

/* cross platform getcwd */
char*
plm_getcwd (char* s, int len)
{
#if (UNIX)
    return getcwd (s, len);
#elif (WIN32)
    /* Microsoft declares POSIX as "deprecated" */
    return _getcwd (s, len);
#endif
}

/* cross platform chdir */
int
plm_chdir (char* s)
{
#if (UNIX)
    return chdir (s);
#elif (WIN32)
    /* Microsoft declares POSIX as "deprecated" */
    return _chdir (s);
#endif
}
