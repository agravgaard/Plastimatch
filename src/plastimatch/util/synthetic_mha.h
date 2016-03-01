/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _synthetic_mha_h_
#define _synthetic_mha_h_

#include "plmutil_config.h"
#include <string>
#include "direction_cosines.h"
#include "plm_image_type.h"

class Rt_study;

enum Pattern_type {
    PATTERN_DOSE,
    PATTERN_GAUSS,
    PATTERN_RECT,
    PATTERN_SPHERE,
    PATTERN_MULTI_SPHERE,
    PATTERN_DONUT,
    PATTERN_GRID,
    PATTERN_LUNG,
    PATTERN_XRAMP,
    PATTERN_YRAMP,
    PATTERN_ZRAMP,
    PATTERN_NOISE,
    PATTERN_CYLINDER,
    PATTERN_GABOR
};

class Synthetic_mha_parms_private;

class PLMUTIL_API Synthetic_mha_parms {
public:
    Synthetic_mha_parms_private *d_ptr;
public:
    enum Image_normalization {
        NORMALIZATION_NONE,
        NORMALIZATION_SUM_ONE,
        NORMALIZATION_SUM_SQR_ONE,
        NORMALIZATION_ZERO_MEAN_STD_ONE,
        NORMALIZATION_GABOR
    };
public:
    Plm_image_type output_type;
    Pattern_type pattern;
    std::string fixed_fn;
    std::string input_fn;
    int dim[3];
    float origin[3];
    float spacing[3];
    Direction_cosines dc;

    float background;
    float foreground;
    float background_alpha;
    float foreground_alpha;

    bool m_want_ss_img;
    bool m_want_dose_img;
    Image_normalization image_normalization;

    float gauss_center[3];
    float gauss_std[3];
    float penumbra;
    float dose_size[6];
    float dose_center[3];
    float rect_size[6];
    float sphere_center[3];
    float sphere_radius[3];
    float donut_center[3];
    float donut_radius[3];
    int donut_rings;
    int grid_spacing[3];
    float lung_tumor_pos[3];
    float noise_mean;
    float noise_std;
    float cylinder_radius[3];
    float cylinder_center[3];
    bool gabor_use_k_fib;
    int gabor_k_fib[2];
    float gabor_k[3];
    
    int num_multi_sphere;

public:
    Synthetic_mha_parms ();
    ~Synthetic_mha_parms ();
};

PLMUTIL_API void synthetic_mha (Rt_study *rtds, Synthetic_mha_parms *parms);

#endif
