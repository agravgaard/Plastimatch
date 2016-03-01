/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dir_list_h_
#define _dir_list_h_

#include "plmsys_config.h"
#include <string>

class Dir_list_private;

class PLMSYS_API Dir_list
{
public:
    Dir_list_private *d_ptr;
public:
    int num_entries;
    char** entries;
public:
    Dir_list ();
    Dir_list (const char* dir);
    Dir_list (const std::string& dir);
    ~Dir_list ();

    void init ();
    void load (const char* dir);
    std::string entry (int idx);
};

#endif
