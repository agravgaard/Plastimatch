/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_opts_h_
#define _bspline_opts_h_

#include "bspline.h"
#include "bspline_parms.h"

class Bspline_options
{
public:
    char* fixed_fn;
    char* moving_fn;
    char* input_xf_fn;
    char* output_warped_fn;
    char* output_xf_fn;
    char* output_vf_fn;
    char* fixed_landmarks;
    char* moving_landmarks;
    char* warped_landmarks;
    char* method;
    float landmark_stiffness;
    float young_modulus;
    plm_long vox_per_rgn[3];
    Bspline_parms parms;
public:
    Bspline_options () {
	fixed_fn = 0;
	moving_fn = 0;
	input_xf_fn = 0;
	output_warped_fn = 0;
	output_xf_fn = 0;
	output_vf_fn = 0;
	fixed_landmarks = 0;
	moving_landmarks = 0;
	warped_landmarks = 0;
	method = 0;
	landmark_stiffness = 0;
	young_modulus = 0;
	for (int d = 0; d < 3; d++) {
	    vox_per_rgn[d] = 15;
	}
    }
};

void bspline_opts_parse_args (
    Bspline_options* options, 
    int argc,char* argv[]
);

#endif
