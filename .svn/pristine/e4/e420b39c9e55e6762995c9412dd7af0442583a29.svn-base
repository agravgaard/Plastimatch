/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_sro_h_
#define _dcmtk_sro_h_

#include "plmbase_config.h"
#include <string>
#include "rt_study_metadata.h"
#include "xform.h"

class Xform;

class PLMBASE_API Dcmtk_sro {
public:
    static void save (
        const Xform::Pointer& xf,
        const Rt_study_metadata::Pointer& rsm_src,   /* Fixed image */
        const Rt_study_metadata::Pointer& rsm_reg,   /* Moving image */
        const std::string& dicom_dir,
        bool filenames_with_uid);
};

#endif
