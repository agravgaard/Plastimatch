/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _mabs_vote_h_
#define _mabs_vote_h_

#include "plmsegment_config.h"
#include "itk_image_type.h"

class Mabs_vote_private;

class PLMSEGMENT_API Mabs_vote {
public:
    Mabs_vote ();
    ~Mabs_vote ();
public:
    Mabs_vote_private *d_ptr;

public:
    /*! \name Inputs */
    ///@{
    /*! \brief Set the reference image.  This is the image we want 
      to segment. */
    void set_fixed_image (
        FloatImageType::Pointer target
    );
    /*! \brief Set the tuning parameters.  Rho is the penalty 
      with respect to distance */
    void set_rho (
        float rho
    );
    /*! \brief Set the tuning parameters.  Sigma is the width of the 
      gaussian with respect to image difference */
    void set_sigma (
        float sigma
    );
    void set_minimum_similarity (
        float minimum_similarity
    );
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Vote for labels using the contribution for a single 
      registered atlas image */
    void vote (
        FloatImageType::Pointer atlas_image, 
        FloatImageType::Pointer dmap_image);
    /*! \brief After you done adding contributions, normalize the 
      votes. */
    void normalize_votes ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the label likelihoods as an ITK image. */
    FloatImageType::Pointer get_weight_image ();
    /*! \brief Return the weights as an ITK image. */
    UCharImageType::Pointer get_label_image ();
    ///@}
};

#endif /* #ifndef _mabs_vote_h_ */
