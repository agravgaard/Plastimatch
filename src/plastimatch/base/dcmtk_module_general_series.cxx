/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_module_general_series.h"
#include "dcmtk_metadata.h"
#include "dicom_util.h"
#include "metadata.h"
#include "plm_uid_prefix.h"

void
Dcmtk_module_general_series::set_sro (
    DcmDataset *dataset, 
    const Rt_study_metadata::Pointer& rsm)
{
    dataset->putAndInsertOFStringArray (DCM_Modality, "REG");
    dataset->putAndInsertString (DCM_SeriesInstanceUID, 
        dicom_uid(PLM_UID_PREFIX).c_str());
    dataset->putAndInsertString (DCM_SeriesNumber, "");
}
