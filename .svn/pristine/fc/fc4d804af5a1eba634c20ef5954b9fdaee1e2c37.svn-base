/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _gdcm2_util_h_
#define _gdcm2_util_h_

#include "plmbase_config.h"

#if PLM_DCM_USE_GDCM2

#include <string>

class Metadata;
namespace gdcm { class File; };

void
gdcm2_get_date_time (
    std::string *date,
    std::string *time
);
void
set_metadata_from_gdcm_file (
    Metadata *meta, 
    /* const */ gdcm::File *gdcm_file, 
    unsigned short group,
    unsigned short elem
);
void
set_gdcm_file_from_metadata (
    gdcm::File *gdcm_file, 
    const Metadata *meta, 
    unsigned short group, 
    unsigned short elem
);
char*
gdcm_uid (char *uid, const char *uid_root);

#endif
#endif
