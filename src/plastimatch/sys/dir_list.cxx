/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if (defined(_WIN32) || defined(WIN32))
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#endif

#include "dir_list.h"
#include "path_util.h"

class Dir_list_private
{
public:
    std::string dir;
};

Dir_list::Dir_list ()
{
    d_ptr = new Dir_list_private;
    this->init ();
}

Dir_list::Dir_list (const char* dir)
{
    d_ptr = new Dir_list_private;
    this->init ();
    this->load (dir);
}

Dir_list::Dir_list (const std::string& dir)
{
    d_ptr = new Dir_list_private;
    this->init ();
    this->load (dir.c_str());
}

Dir_list::~Dir_list ()
{
    int i;
    if (this->entries) {
	for (i = 0; i < this->num_entries; i++) {
	    free (this->entries[i]);
	}
	free (this->entries);
    }
    delete d_ptr;
}

void
Dir_list::init ()
{
    this->num_entries = 0;
    this->entries = 0;
}

void 
Dir_list::load (const char* dir)
{
#if (_WIN32)
    intptr_t srch;
    struct _finddata_t d;
    char* buf;
    
    buf = (char*) malloc (strlen (dir) + 3);
    sprintf (buf, "%s/*", dir);

    srch = _findfirst (buf, &d);
    free (buf);

    if (srch == -1) return;

    do {
	this->num_entries ++;
	this->entries = (char**) realloc (
	    this->entries, 
	    this->num_entries * sizeof (char*));
	this->entries[this->num_entries-1] = strdup (d.name);
    } while (_findnext (srch, &d) != -1);
    _findclose (srch);

#else
    DIR* dp;
    struct dirent* d;

    dp = opendir (dir);
    if (!dp) return;

    for (d = readdir(dp); d; d = readdir(dp)) {
	this->num_entries ++;
	this->entries = (char**) realloc (
	    this->entries, 
	    this->num_entries * sizeof (char*));
	this->entries[this->num_entries-1] = strdup (d->d_name);
    }
    closedir (dp);
#endif

    d_ptr->dir = dir;
}

std::string
Dir_list::entry (int idx)
{
    if (idx < 0 || idx > num_entries) {
        return "";
    }
    return compose_filename (d_ptr->dir, entries[idx]);
}
