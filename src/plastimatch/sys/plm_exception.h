/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plm_exception_h_
#define _plm_exception_h_

#include "plmsys_config.h"
#include <exception>
#include <string>

class Plm_exception : public std::exception
{
public:
    Plm_exception (const std::string& a) : info(a) { }
    virtual ~Plm_exception () throw() {}

    const char* what () const throw () {
        return info.c_str();
    }

public:
    /* Error message */
    const std::string info;

private:
    const Plm_exception& operator=(const Plm_exception&);
};

#endif
