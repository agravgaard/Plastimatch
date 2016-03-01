/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _sift_h_
#define _sift_h_

#include "plmutil_config.h"
#include <string>
#include <itkScaleInvariantFeatureImageFilter.h>
#include "plm_macros.h"
#include "itk_image_type.h"

class Sift_private;
class Plm_image;

/*! \brief 
 * The Sift class implements a SIFT feature detector.
 */
class PLMUTIL_API Sift {
public:
    Sift ();
    ~Sift ();
public:
    Sift_private *d_ptr;
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the input image.  The image will be loaded
      from the specified filename. */
    void set_image (const char* image_fn);
    /*! \brief Set the input image.  The image will be loaded
      from the specified filename. */
    void set_image (const std::string& image_fn);
    /*! \brief Set the input image as a Plm image. */
    void set_image (Plm_image* image);
    /*! \brief Set the input image as an ITK image. */
    void set_image (const FloatImageType::Pointer image);

    /*! \brief Set the contrast threshold */
    void set_contrast_threshold (float contrast_threshold);
    /*! \brief Set the curvature threshold */
    void set_curvature_threshold (float curvature_threshold);
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute SIFT feature locations in the input image. */
    void run ();
    /*! \brief Do feature matching, and write output to file.
      This should be a separate class. */
    static void match_features (Sift& sift1, Sift& sift2,
        const char* filename1, const char* filename2, 
        float match_ratio);
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the detected keypoints. Unfortunately, the 
      SIFT keypoints are not (yet) compatible with ordinary 
      plastimatch (native or itk) pointsets. */
    itk::ScaleInvariantFeatureImageFilter<FloatImageType,3>::PointSetTypePointer
        get_keypoints ();
    /*! \brief Save the sift-detected points to a fcsv file.  */
    void save_pointset (const char* filename);
    ///@}
};

#endif
