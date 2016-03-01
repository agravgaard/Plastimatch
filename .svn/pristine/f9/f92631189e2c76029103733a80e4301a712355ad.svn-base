/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xio_patient_h
#define _xio_patient_h

#include "plmbase_config.h"
#include <list>
#include <string>

/* This class represents a toplevel patient directory */
class PLMBASE_API Xio_patient {
public:
    Xio_patient (const char* path);
    ~Xio_patient ();
public:
    std::string m_path;
    std::string m_demographic_fn;
    std::list< std::string > studyset_dirs;
    std::list< std::string > plan_dirs;
public:
    void add_studyset_dir (const std::string& studyset_path);
    void add_plan_dir (const std::string& plan_path);
    void analyze ();
};

#endif
