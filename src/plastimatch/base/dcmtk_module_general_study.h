/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_module_general_study_h_
#define _dcmtk_module_general_study_h_

#include "plmbase_config.h"
#include <string>
#include "rt_study_metadata.h"

class DcmDataset;

class PLMBASE_API Dcmtk_module_general_study {
public:
    static void set (DcmDataset *dataset, 
        const Rt_study_metadata::Pointer& rsm);
};

#endif
