/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "compiler_warnings.h"
#if PLM_DCM_USE_DCMTK
#include "dcmtk_loader.h"
#include "dcmtk_rt_study.h"
#endif
#include "logfile.h"
#include "plm_image.h"
#include "rt_study.h"
#include "rt_study_p.h"
#include "segmentation.h"

void
Rt_study::load_dcmtk (const char *dicom_path)
{
#if PLM_DCM_USE_DCMTK
    Dcmtk_rt_study drs;
    drs.set_rt_study_metadata (d_ptr->m_drs);
    drs.load (dicom_path);

    d_ptr->m_img = drs.get_image ();
    Rtss::Pointer rtss = drs.get_rtss ();
    if (rtss) {
        d_ptr->m_rtss = Segmentation::New ();
        d_ptr->m_rtss->set_structure_set (drs.get_rtss ());
    }
    d_ptr->m_dose = drs.get_dose ();
#endif
}

void
Rt_study::save_dcmtk (const char *dicom_dir, bool filenames_with_uid)
{
#if PLM_DCM_USE_DCMTK
    Dcmtk_rt_study drs;
    drs.set_rt_study_metadata (d_ptr->m_drs);
    drs.set_image (d_ptr->m_img);
    if (d_ptr->m_rtss) {
        /* GCS FIX. This call to prune_empty() is a hack. 
           It should be allowed to write empty structures, but 
           current plastimatch logic sets num_structures to max 
           when performing cxt_extract().  Segmentation class 
           logic should be improved to better keep track of 
           when structure names are valid to avoid this. */
        d_ptr->m_rtss->prune_empty ();

        drs.set_rtss (d_ptr->m_rtss->get_structure_set());
    }
    drs.set_dose (d_ptr->m_dose);

    drs.set_filenames_with_uid (filenames_with_uid);
    drs.save (dicom_dir);
#endif
}

void
Rt_study::save_dcmtk_dose (const char *dicom_dir)
{
#if PLM_DCM_USE_DCMTK
    Dcmtk_rt_study drs;
    drs.set_rt_study_metadata (d_ptr->m_drs);
    drs.set_dose (d_ptr->m_dose);
    drs.save (dicom_dir);
#endif
}
