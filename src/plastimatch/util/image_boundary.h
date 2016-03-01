/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _image_boundary_h_
#define _image_boundary_h_

#include "plmutil_config.h"
#include "itk_image.h"

class Plm_image;
class Image_boundary_private;

/*! \brief 
 * The Image_boundary class takes an input image (binary) and computes
 * an output image.  Voxels of the output image will be one 
 * if they are (1) non-zero in the input image, and (2) have a zero 
 * voxel in their six-neighborhood.  Other output image voxels will have 
 * value zero.
 */
class PLMUTIL_API Image_boundary {
public:
    Image_boundary ();
    ~Image_boundary ();
public:
    Image_boundary_private *d_ptr;

public:
    /*! \brief This enum is used to control the algorithm behavior 
      for voxels at the edge of the volume.  If ZERO_PADDING is 
      specified, all non-zero voxels at the edge of the volume 
      will be treated as boundary voxels.  If EDGE_PADDING is 
      specified, non-zero voxels at the edge of the volume are 
      only treated as boundary voxels if they neighbor 
      a zero voxel. */
    enum Volume_boundary_behavior {
        ZERO_PADDING,
        EDGE_PADDING
    };
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the input image.  The image will be loaded
      from the specified filename. */
    void set_input_image (const char* image_fn);
    /*! \brief Set the input image as an ITK image. */
    void set_input_image (const UCharImageType::Pointer image);
    /*! \brief Set the volume boundary behavior, either 
      ZERO_PADDING or EDGE_PADDING */
    void set_volume_boundary_behavior (Volume_boundary_behavior vbb);
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute image boundary */
    void run ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the boundary image as an ITK image.  */
    UCharImageType::Pointer get_output_image ();
    ///@}
};

PLMUTIL_API
UCharImageType::Pointer do_image_boundary (UCharImageType::Pointer image);

#endif
