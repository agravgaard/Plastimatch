/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dicom_sro_save_h_
#define _dicom_sro_save_h_

#include "plmutil_config.h"
#include "plm_image.h"
#include "xform.h"

class Dicom_sro_save_private;

/*! \brief 
 * The Dicom_sro_save is a utility class for saving DICOM Spatial 
 * Registration objects from different input sources.
 * The input images may either already exist as DICOM, in which 
 * case the SRO will reference the existing series, or they may 
 * be written by this class.
 */
class PLMUTIL_API Dicom_sro_save {
public:
    Dicom_sro_save ();
    ~Dicom_sro_save ();
public:
    Dicom_sro_save_private *d_ptr;
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the reference image.  If it is a non-DICOM volume, 
      the volume will be loaded and exported as DICOM.
      If it is a DICOM volume, it will be loaded and referenced. */
    void set_fixed_image (const char* path);
    /*! \brief Set the reference image.  The volume will be 
      exported as DICOM. */
    void set_fixed_image (const Plm_image::Pointer& fixed_image);
    /*! \brief Set the moving image.  If it is a non-DICOM volume, 
      the volume will be loaded and exported as DICOM.
      If it is a DICOM volume, it will be loaded and referenced */
    void set_moving_image (const char* path);
    /*! \brief Set the moving image.  The volume will be 
      exported as DICOM. */
    void set_moving_image (const Plm_image::Pointer& moving_image);
    /*! \brief Set the transform, which will be exported as DICOM. */
    void set_xform (const Xform::Pointer& xform);
    /*! \brief Set the path to the output directory where the DICOM
      SRO and images will be saved */
    void set_output_dir (const std::string& output_dir);
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Save the SRO and maybe some images too. */
    void run ();
    ///@}
};

#endif
