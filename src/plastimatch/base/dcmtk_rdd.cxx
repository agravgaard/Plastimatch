/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "dcmtk_rt_study.h"
#include "plm_math.h"
#include "plm_uid_prefix.h"
#include "rt_study_metadata.h"

void
dcmtk_load_rdd (
    Rt_study_metadata::Pointer rsm, 
    const char *dicom_dir
)
{
    if (!dicom_dir) {
	return;
    }

    Dcmtk_rt_study drs (dicom_dir);
    drs.set_rt_study_metadata (rsm);
    drs.parse_directory ();
}
