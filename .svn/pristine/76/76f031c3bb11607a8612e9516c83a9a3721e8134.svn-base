/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dcmtk_config.h"
#include "dcmtk/dcmdata/dctk.h"
#include "print_and_exit.h"

#include "dcmtk_util.h"

void
dcmtk_get_date_time (
    std::string *current_date,
    std::string *current_time
)
{
    OFString date_string;
    OFString time_string;
    DcmDate::getCurrentDate (date_string);
    DcmTime::getCurrentTime (time_string);
    *current_date = date_string.c_str();
    *current_time = time_string.c_str();
    
    //*date = "20110101";
    //*time = "120000";
            
}

