/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "gdcm1_file.h"
#include "gdcm1_rdd.h"
#include "gdcm1_series.h"
#include "gdcm1_util.h"
#include "metadata.h"
#include "plm_image_header.h"
#include "plm_math.h"
#include "plm_uid_prefix.h"
#include "rt_study_metadata.h"

void
gdcm1_load_rdd (
    Rt_study_metadata *rdd,
    const char *dicom_dir
)
{
    Gdcm_series gs;
    std::string tmp;

    if (!dicom_dir) {
	return;
    }

    gs.load (dicom_dir);
    gs.digest_files ();
    if (!gs.m_have_ct) {
	return;
    }
    gdcm::File* file = gs.get_ct_slice ();

    /* Add geometry */
    int d;
    float offset[3], spacing[3];
    /* Convert double to float */
    for (d = 0; d < 3; d++) {
	offset[d] = gs.m_origin[d];
	spacing[d] = gs.m_spacing[d];
    }
    rdd->set_image_header (Plm_image_header (gs.m_dim, offset, spacing, 0));

    /* Store metadata into here */
    Metadata::Pointer& meta = rdd->get_study_metadata ();

    /* PatientName */
    set_metadata_from_gdcm_file (meta, file, 0x0010, 0x0010);

    /* PatientID */
    set_metadata_from_gdcm_file (meta, file, 0x0010, 0x0020);

    /* PatientSex */
    set_metadata_from_gdcm_file (meta, file, 0x0010, 0x0040);

    /* PatientPosition */
    set_metadata_from_gdcm_file (meta, file, 0x0018, 0x5100);

    /* StudyID */
    tmp = gdcm_file_GetEntryValue (file, 0x0020, 0x0010);
    if (tmp != gdcm_file_GDCM_UNFOUND()) {
        meta->set_metadata (0x0020, 0x0010, tmp.c_str());
    }

    /* StudyInstanceUID */
    tmp = gdcm_file_GetEntryValue (file, 0x0020, 0x000d);
    rdd->set_study_uid (tmp.c_str());

    /* SeriesInstanceUID */
    tmp = gdcm_file_GetEntryValue (file, 0x0020, 0x000e);
    rdd->set_ct_series_uid (tmp.c_str());
	
    /* FrameOfReferenceUID */
    tmp = gdcm_file_GetEntryValue (file, 0x0020, 0x0052);
    rdd->set_frame_of_reference_uid (tmp.c_str());

    /* Slice uids */
    gs.get_slice_uids (rdd);

    /* Done */
    rdd->set_slice_list_complete ();
}
