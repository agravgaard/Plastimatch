/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _distance_map_h_
#define _distance_map_h_

#include "plmutil_config.h"
#include "itk_image_type.h"

class Distance_map_private;
class Plm_image;

class PLMUTIL_API Distance_map {
public:
    Distance_map ();
    ~Distance_map ();
public:
    Distance_map_private *d_ptr;

public:
    /*! \brief Different distance map algorithms. */
    enum Algorithm {
        DANIELSSON,
        ITK_DANIELSSON,
        ITK_MAURER,
    };

public:
    /*! \name Inputs */
    ///@{
    /*! \brief Set the input image.  The image will be loaded from 
      the specified filename as a binary mask (unsigned char) image. */
    void set_input_image (const std::string& image_fn);
    void set_input_image (const char* image_fn);
    /*! \brief Set the input image as an ITK image. */
    void set_input_image (const UCharImageType::Pointer image);
    /*! \brief Choose whether the output image is distance or squared 
      distance. The default is not squared distance. */
    void set_use_squared_distance (bool use_squared_distance);
    /*! \brief Choose whether the inside is positive or negative.  
      The default is inside negative */
    void set_inside_is_positive (bool inside_is_positive);
    /*! \brief Choose which algorithm to use */
    void set_algorithm (const std::string& algorithm);
    /*! \brief Set maximum distance */
    void set_maximum_distance (float max_distance);
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute gamma value at each location in the input image */
    void run ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the gamma image as an ITK image.  */
    FloatImageType::Pointer get_output_image ();
    ///@}
};

#endif
