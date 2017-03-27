/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __xvi_archive_h__
#define __xvi_archive_h__

#include <string>

class Xvi_archive_parms
{
public:
    Xvi_archive_parms () {
        write_debug_files = false;
    }
public:
    std::string patient_dir;
    std::string patient_id_override;
    bool write_debug_files;
};

#endif
