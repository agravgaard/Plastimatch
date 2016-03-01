/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "file_util.h"
#include "path_util.h"

int
extension_is (const char* fname, const char* ext)
{
    return (strlen (fname) > strlen(ext)) 
	&& !strcmp (&fname[strlen(fname)-strlen(ext)], ext);
}

int
extension_is (const std::string& fname, const char* ext)
{
    return extension_is (fname.c_str(), ext);
}

void
strip_extension (char* filename)
{
    char *p;

    p = strrchr (filename, '.');
    if (p) {
	*p = 0;
    }
}

std::string
strip_extension (const std::string& filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot); 
}

void
trim_trailing_slashes (char *pathname)
{
    char *p = pathname + strlen (pathname) - 1;
    while (p >= pathname && ISSLASH(*p)) {
	*p = 0;
    }
}

std::string
trim_trailing_slashes (const std::string& pathname)
{
    size_t s = pathname.find_last_not_of ("/");
    return pathname.substr(0, s+1);
}

/* Caller must free memory */
char*
file_util_parent (const char *filename)
{
    char *tmp = 0;
    char *p = 0, *q = 0;

    if (!filename) return tmp;

    p = tmp = strdup (filename);
    trim_trailing_slashes (p);
    while (*p) {
	if (ISSLASH (*p)) {
	    q = p;
	}
	p ++;
    }
    if (q) {
	*q = 0;
	return tmp;
    } else {
	/* No directory separators -- return "." */
	free (tmp);
	return strdup (".");
    }
}

std::string
file_util_parent (const std::string& filename)
{
    char *tmp = file_util_parent (filename.c_str());
    std::string parent (tmp);
    free (tmp);
    return parent;
}

/* Caller must free memory */
char*
file_util_dirname (const char *filename)
{
    if (!filename) return 0;

    if (is_directory (filename)) {
	return strdup (filename);
    }

    return file_util_parent (filename);
}

std::string
file_util_dirname_string (const char *filename)
{
    std::string dirname = "";

    char *c_dirname = file_util_dirname (filename);
    if (c_dirname) {
        dirname = c_dirname;
        free (c_dirname);
    }
    return dirname;
}

std::string
strip_leading_dir (const std::string& fn)
{
    size_t s = fn.find_first_of ("/");
    if (s == fn.npos) {
        return fn;
    }
    return fn.substr(s+1);
}

std::string
basename (const std::string& fn)
{
    std::string tmp = trim_trailing_slashes (fn);
    size_t s = tmp.find_last_of ("/");
    if (s == tmp.npos) {
        return tmp;
    }
    return tmp.substr(s+1);
}

std::string
dirname (const std::string& fn)
{
    std::string tmp = trim_trailing_slashes (fn);
    size_t s = tmp.find_last_of ("/");
    if (s == tmp.npos) {
        return tmp;
    }
    tmp = tmp.substr(0, s+1);
    return trim_trailing_slashes (tmp);
}

std::string
compose_filename (const char *a, const char *b)
{
    std::string output_fn;

    char *tmp = strdup (a);
    trim_trailing_slashes (tmp);
    output_fn = tmp;
    free (tmp);
    output_fn.append ("/");
    output_fn.append (b);
    return output_fn;
}

std::string
compose_filename (const std::string& a, const std::string& b)
{
    return compose_filename (a.c_str(), b.c_str());
}

std::string 
make_windows_slashes (const std::string& s)
{
#if (_WIN32)
    std::string t = s;
    std::replace(t.begin(), t.end(), '/', '\\');
    return t;
#else
    return s;
#endif
}

