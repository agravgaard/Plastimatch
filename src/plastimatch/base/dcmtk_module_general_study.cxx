/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_module_general_study.h"
#include "dcmtk_metadata.h"
#include "metadata.h"

void
Dcmtk_module_general_study::set (
    DcmDataset *dataset, 
    const Rt_study_metadata::Pointer& rsm)
{
    /* These are stored in Dcmtk_rt_study class */
    dataset->putAndInsertString (DCM_StudyInstanceUID, 
        rsm->get_study_uid());
    dataset->putAndInsertOFStringArray (DCM_StudyDate, 
        rsm->get_study_date());
    dataset->putAndInsertOFStringArray (DCM_StudyTime, 
        rsm->get_study_time());
    dataset->putAndInsertString (DCM_ReferringPhysicianName, "");
    dcmtk_copy_from_metadata (dataset, rsm->get_study_metadata(), 
        DCM_StudyID, "");
    dataset->putAndInsertOFStringArray (DCM_AccessionNumber, "");
}
