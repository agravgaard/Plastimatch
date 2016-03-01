/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _geometry_chooser_h_
#define _geometry_chooser_h_

#include "plmutil_config.h"
#include "itk_image_type.h"
#include "plm_int.h"

class Direction_cosines;
class Geometry_chooser_private;
class Plm_image_header;

/*! \brief 
 * The Geometry_chooser class provides a convenient method for 
 * choosing the right output geometry for a function.
 *
 * The output geometry is selected based on preferential selection 
 * from a set of possible inputs which might or might not be 
 * available.  In order of preference, the geometry is set according to:
 * 
 * - Manual setting of dim, origin, spacing, direction_cosines
 * - Copying the geometry from a specific image (called the "fixed image")
 * - Setting the geometry from an input image (called the "reference image")
 * 
 * In the last case, the geometry can be expanded to also include the 
 * union of the regions spanned by another image (called the "compare image")
 */
class PLMUTIL_API Geometry_chooser {
public:
    Geometry_chooser ();
    ~Geometry_chooser ();
public:
    Geometry_chooser_private *d_ptr;
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the reference image.  The geometry will be 
      from the specified filename. */
    void set_reference_image (const char* image_fn);
    /*! \brief Set the reference image as an ITK image. */
    template<class T> void set_reference_image (const T& image);
    /*! \brief Set the compare image.  The image will be loaded
      from the specified filename. */
    void set_compare_image (const char* image_fn);
    /*! \brief Set the compare image as an ITK image. */
    void set_compare_image (const UCharImageType::Pointer image);
    void set_compare_image (const FloatImageType::Pointer image);
    /*! \brief Set the fixed image.  The image will be loaded
      from the specified filename. */
    void set_fixed_image (const char* image_fn);
    void set_fixed_image (const std::string& image_fn);
    /*! \brief Set the fixed image as an ITK image. */
    template<class T> void set_fixed_image (const T& image);
    /*! \brief Set the image dimension (number of voxels) manually. */
    void set_dim (const plm_long dim[3]);
    /*! \brief Set the image origin manually. */
    void set_origin (const float origin[3]);
    /*! \brief Set the image spacing manually. */
    void set_spacing (const float spacing[3]);
    /*! \brief Set the image direction cosines manually. */
    void set_direction_cosines (const Direction_cosines& direction_cosines);
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the best fitting geometry */
    const Plm_image_header *get_geometry ();
    ///@}
};

#endif
