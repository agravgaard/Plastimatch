/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_module_patient.h"
#include "dcmtk_metadata.h"
#include "metadata.h"

void
Dcmtk_module_patient::set (DcmDataset *dataset, const Metadata::Pointer& meta)
{
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientName, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientID, "");
    dataset->putAndInsertString (DCM_PatientBirthDate, "");
    dcmtk_copy_from_metadata (dataset, meta, DCM_PatientSex, "O");
}
