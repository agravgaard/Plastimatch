/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _gamma_dose_comparison_h_
#define _gamma_dose_comparison_h_

#include "plmutil_config.h"
#include "plm_image.h"
#include "plm_macros.h"
#include "itk_image_type.h"

class Gamma_dose_comparison_private;
class Plm_image;

/*! \brief 
 * The Gamma_dose_comparison class executes a comparison between 
 * two dose distributions based on the "gamma index" defined by 
 * Dan Low et al. in the following reference:
 * \n\n
 *   Low et al, 
 *   A technique for the quantitative evaluation of dose distributions,
 *   Med Phys. 1998 May;25(5):656-61.
 * \n\n
 * The comparison is based on searching a local neighborhood for the 
 * most similar dose.  The similarity is computed as the geometric mean 
 * of the dose difference and spatial distance.  The gamma value at 
 * a point is then the minimum of this similarity value over the 
 * the neighborhood.
 * Generally, the gamma value is normalized based on a spatial tolerance
 * and a dose difference tolerance such that gamma values of less than 
 * 1.0 are acceptable, and gamma values greater than 1.0 are unacceptable.
 */
class PLMUTIL_API Gamma_dose_comparison {
public:
    Gamma_dose_comparison ();
    ~Gamma_dose_comparison ();
public:
    Gamma_dose_comparison_private *d_ptr;
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the reference image.  The image will be loaded
      from the specified filename. */
    void set_reference_image (const char* image_fn);
    /*! \brief Set the reference image as a Plm image. */
    void set_reference_image (const Plm_image::Pointer& image);
    /*! \brief Set the reference image as an ITK image. */
    void set_reference_image (const FloatImageType::Pointer image);
    /*! \brief Set the compare image.  The image will be loaded
      from the specified filename. */
    void set_compare_image (const char* image_fn);
    /*! \brief Set the compare image as a Plm image. */
    void set_compare_image (const Plm_image::Pointer& image);
    /*! \brief Set the compare image as an ITK image. */
    void set_compare_image (const FloatImageType::Pointer image);

    /*! \brief Set the mask image.  The image will be loaded
      from the specified filename. */
    void set_mask_image (const char* image_fn);
    void set_mask_image (const std::string& image_fn);
    /*! \brief Set the mask image as a Plm image. */
    void set_mask_image (const Plm_image::Pointer& image);
    /*! \brief Set the mask image as an ITK image. */
    void set_mask_image (const UCharImageType::Pointer image);

    /*! \brief Get the distance to agreement (DTA) tolerance, in mm. */
    float get_spatial_tolerance ();
    /*! \brief Set the distance to agreement (DTA) tolerance, in mm. */
    void set_spatial_tolerance (float spatial_tol);
    /*! \brief Get the dose difference tolerance, in float (fraction of reference dose). */
    float get_dose_difference_tolerance ();
    /*! \brief Set the dose difference tolerance, in float type. 
      If a reference dose (prescription dose) is specified, 
      the dose difference tolerance is treated as a 
      fraction of the reference dose.  Otherwise it is treated as a
      fraction of the maximum dose in the reference volume.
      With local-gamma option, it is treated as a fraction of the local dose in the reference image
      To use a 3% dose tolerance, you would set this value to 0.03.  */
    void set_dose_difference_tolerance (float dose_tol);
    /*! \brief Set the reference dose (prescription dose).  
      The reference dose is used for dose comparison and analysis 
      threshold.  If the reference dose is not set, the maximum dose in the 
      reference dose volume is used as the reference dose. */
    void set_reference_dose (float dose);
    /*! \brief Unset the reference dose (prescription dose).  
      The reference dose is used for dose comparison and analysis 
      threshold.  If the reference dose is not set, the maximum dose in the 
      reference dose volume is used as the reference dose. */
    void unset_reference_dose ();

    /*! \brief Set a dose threshold for gamma analysis, as a 
      fraction of the reference dose.  This is used to ignore 
      voxels which have dose below a certain value.  
      For example, to consider only voxels which have dose greater 
      than 10% of the maximum dose, you would call
      set_analysis_threshold (0.1). 
      The threshold is applied to dose voxels in the reference dose volume.
    */
    void set_analysis_threshold (float thresh);
    /*! \brief Set the maximum gamma computed by the class.  This is 
      used to speed up computation.  A typical value is 2 or 3.  */
    /* */
    void set_gamma_max (float gamma_max);

    /*! \brief Set a callback routine for reporting progress 
     */
    void set_progress_callback (void (*progress_callback)(float));
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute gamma value at each location in the input image */
    void run ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the gamma image as a Plm_image.  */
    Plm_image::Pointer get_gamma_image ();
    /*! \brief Return the gamma image as an ITK image.  */
    FloatImageType::Pointer get_gamma_image_itk ();
    /*! \brief Return a binary image of passing voxels as a Plm image. */
    Plm_image* get_pass_image ();
    /*! \brief Return a binary image of passing voxels as an ITK image. */
    UCharImageType::Pointer get_pass_image_itk ();
    /*! \brief Return a binary image of failing voxels as a Plm image. */
    Plm_image* get_fail_image ();
    /*! \brief Return a binary image of failing voxels as an ITK image. */
    UCharImageType::Pointer get_fail_image_itk ();

    /*! \brief Return ref. image used for gamma evaluation */
    Plm_image* get_ref_image();
    /*! \brief Return comp. image used for gamma evaluation */
    Plm_image* get_comp_image();



    /*! \brief Return fraction of passing points, subject to reference dose 
      being greater than analysis threshold */
    float get_pass_fraction ();

    int get_analysis_num_vox();
    int get_passed_num_vox();
    float get_reference_dose();

    ///@}
    std::string get_report_string();	
    void set_report_string(std::string& report_str);
    bool is_local_gamma();
    void set_local_gamma(bool bLocalGamma);
    bool is_compute_full_region();
    void set_compute_full_region(bool b_compute_full_region);
    float get_inherent_resample_mm();	
    void set_inherent_resample_mm(float inherent_spacing_mm);
    bool is_resample_nn();
    void set_resample_nn(bool b_resample_nn);
    bool is_interp_search();
    void set_interp_search(bool b_interp_search);

    bool is_ref_only_threshold();
    /*! \brief By default, when analysis is limited to areas with 
     dose higher than a percent threshold, this threshold is based 
     on the dose in either reference or compare image.  
     If set_ref_only_threshold() is called with a value of true, 
     the analysis will only consider dose higher than the threshold 
     in the compare image */
    void set_ref_only_threshold(bool b_ref_only_threshold);

protected:
    /*! \brief Resample ref image with fixed spacing */
    void resample_image_with_fixed_spacing (Plm_image::Pointer& input_img, float spacing[3]);
    /*! \brief Resample image_moving to image_reference */
    void resample_image_to_reference (const Plm_image::Pointer& image_reference, Plm_image::Pointer& image_moving);
};

#endif
