/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dvh_h_
#define _dvh_h_

#include "plmutil_config.h"
#include <string>
#include "itk_image_type.h"

class Dvh_private;
class Plm_image;
class Segmentation;

class PLMUTIL_API Dvh {
public:
    Dvh ();
    ~Dvh ();
public:
    Dvh_private *d_ptr;

public:
    enum Dvh_units {
        DVH_UNITS_GY,
        DVH_UNITS_CGY,
    };

    enum Dvh_normalization {
        DVH_NORMALIZATION_PCT,
        DVH_NORMALIZATION_VOX,
    };

    enum Histogram_type {
        DVH_CUMULATIVE_HISTOGRAM,
        DVH_DIFFERENTIAL_HISTOGRAM
    };

public:

    /*! \name Defaults */
    ///@{
    /*! \brief Return the default value for dose units */
    static Dvh::Dvh_units default_dose_units () {
        return Dvh::DVH_UNITS_GY;
    }
    /*! \brief Return the default value for DVH normalization */
    static Dvh::Dvh_normalization default_normalization () {
        return Dvh::DVH_NORMALIZATION_PCT;
    }
    /*! \brief Return the default value for histogram type */
    static Dvh::Histogram_type default_histogram_type () {
        return Dvh::DVH_CUMULATIVE_HISTOGRAM;
    }
    /*! \brief Return the default number of bins in the histogram */
    static int default_histogram_num_bins () {
        return 256;
    }
    /*! \brief Return the default bin width (in Gy) in the histogram */
    static float default_histogram_bin_width () {
        return 0.5;
    }
    ///@}

    /*! \name Inputs */
    ///@{
    /*! \brief Set the structure set image.  The image will be loaded
      from the specified filename, and an optional file containing the 
      image list will be loaded. */
    void set_structure_set_image (const char* ss_image_fn, 
        const char *ss_list_fn);
    /*! \brief Set the structure set image as an Segmentation */
    void set_structure_set_image (Segmentation* image);
    /*! \brief Set the dose image.  The image will be loaded
      from the specified filename. */
    void set_dose_image (const char* image_fn);
    /*! \brief Set the dose image as a Plm image. */
    void set_dose_image (Plm_image* image);
    /*! \brief Set the dose image as an ITK image. */
    void set_dose_image (const FloatImageType::Pointer image);

    /*! \brief Set the units for dose image. */
    void set_dose_units (enum Dvh_units units);
    /*! \brief Set the units for dvh computation.  Normalization in 
      either percent or voxels, choice of cumulative or differential 
      histogram, number of bins, and bin width. */
    void set_dvh_parameters (enum Dvh_normalization normalization,
        enum Histogram_type histogram_type, int num_bins, float bin_width);

    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute dvh */
    void run ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Save the DVH as a csv file */
    void save_csv (const char* csv_fn);
    ///@}

};

#endif
