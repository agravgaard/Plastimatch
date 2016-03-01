/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rt_study_p_h_
#define _rt_study_p_h_

#include "plmbase_config.h"
#include "metadata.h"
#include "rt_study_metadata.h"
#include "segmentation.h"
#include "xio_ct_transform.h"

class PLMBASE_API Rt_study_private {
public:
    Rt_study_metadata::Pointer m_drs;  /* UIDs, etc -- used by dcmtk */
    std::string m_xio_dose_filename;   /* XiO dose file to use as template 
                                          for saving in XiO format */
    Xio_ct_transform *m_xio_transform; /* Transformation from XiO to DICOM
                                          coordinates */
    Plm_image::Pointer m_img;          /* CT image */
    Plm_image::Pointer m_dose;         /* RT dose */
    Segmentation::Pointer m_rtss;      /* RT structure set */

public:
    Rt_study_private () {
        m_drs = Rt_study_metadata::New ();
        m_xio_transform = new Xio_ct_transform ();
    }
    ~Rt_study_private () {
        delete m_xio_transform;
    }
};

#endif
