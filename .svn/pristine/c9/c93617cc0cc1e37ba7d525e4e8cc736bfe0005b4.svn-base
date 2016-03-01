/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _image_center_h_
#define _image_center_h_

#include "plmutil_config.h"
#include "itk_image_type.h"
#include "plm_image.h"

class Image_center_private;

/*! \brief 
 * The Image_center class computes the center of mass of a binary image.
 */
class PLMUTIL_API Image_center {
public:
    Image_center ();
    ~Image_center ();
public:
    Image_center_private *d_ptr;
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the reference image as a Plm image. */
    void set_image (const UCharImageType::Pointer& image);
    /*! \brief Set the reference image as an ITK image. */
    void set_image (const Plm_image::Pointer& image);
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute dice statistics */
    void run ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the center of mass */
    DoubleVector3DType get_image_center_of_mass ();
    ///@}
};

#endif
