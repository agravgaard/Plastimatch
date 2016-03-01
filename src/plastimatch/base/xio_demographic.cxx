/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "print_and_exit.h"
#include "string_util.h"
#include "xio_demographic.h"

Xio_demographic::Xio_demographic (const char *filename)
{
    std::ifstream ifs (filename);
    if (ifs.fail()) {
        print_and_exit ("Error opening file %s for read\n", filename);
    }

    /* version string 
       00011017 - pxio version 4.2 */
    std::string version;
    getline (ifs, version);

    /* important stuff here */
    getline (ifs, m_import_date);
    m_import_date = string_trim (m_import_date);
    if (m_import_date.length() >= 8) {
        m_import_date = m_import_date.substr(0,8);
    } else {
        m_import_date = "";
    }
    getline (ifs, m_patient_name);
    m_patient_name = string_trim (m_patient_name);
    getline (ifs, m_patient_id);
    m_patient_id = string_trim (m_patient_id);
}

Xio_demographic::~Xio_demographic ()
{
}
