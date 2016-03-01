/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _synthetic_vf_h_
#define _synthetic_vf_h_

#include "plmutil_config.h"
#include "plm_image_header.h"

// TODO: change type of pih to Plm_image_header*

//class Plm_image_header;

class Synthetic_vf_parms {
public:
    enum Pattern {
	PATTERN_ZERO,
	PATTERN_TRANSLATION,
	PATTERN_RADIAL,
	PATTERN_GAUSSIAN,
	PATTERN_UNKNOWN
    };

public:
    Pattern pattern;
    Plm_image_header pih;
    float gaussian_center[3];
    float gaussian_mag[3];
    float gaussian_std[3];
    float radial_center[3];
    float radial_mag[3];
    float translation[3];

public:
    Synthetic_vf_parms () {
	pattern = PATTERN_UNKNOWN;
	for (int i = 0; i < 3; i++) {
            gaussian_center[i] = 0.0f;
            gaussian_mag[i] = 0.0f;
            gaussian_std[i] = 0.0f;
	    radial_center[i] = 0.0f;
	    radial_mag[i] = 0.0f;
	    translation[i] = 0.0f;
	}
    }
};

PLMUTIL_API DeformationFieldType::Pointer synthetic_vf (Synthetic_vf_parms* parms);

#endif
