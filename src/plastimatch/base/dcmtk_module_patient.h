/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dcmtk_module_patient_h_
#define _dcmtk_module_patient_h_

#include "plmbase_config.h"
#include <string>
#include "metadata.h"

class DcmDataset;

class PLMBASE_API Dcmtk_module_patient {
public:
    static void set (DcmDataset *dataset, const Metadata::Pointer& meta);
};

#endif
