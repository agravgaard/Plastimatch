/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pcmd_gamma_h_
#define _pcmd_gamma_h_

#include "plmcli_config.h"
#include <string.h>
#include <stdlib.h>
#include "plm_image_type.h"

class Gamma_parms {
public:
    std::string ref_image_fn;
    std::string cmp_image_fn;
    std::string mask_image_fn;
    std::string out_image_fn;
    std::string out_failmap_fn;

    /* Gamma options */
    float dose_tolerance;
    float dta_tolerance;
    bool have_reference_dose;
    float reference_dose;
    float gamma_max;

    /* Extended Gamma options by YK*/	
    std::string out_report_fn; //YK: text file name
    bool b_local_gamma; // If true, local dose difference will be used. 
    bool b_compute_full_region; // If true, gamma will not be calculated for points below the threshold dose e.g. <10%
    float f_inherent_resample_mm; //if -1.0, no resample will be carried out.
    float f_analysis_threshold; //if -1.0, no threshold will be applied. typical value = 0.1 (10%)
    bool b_resample_nn; //with this on, nearest resample will be used for comp-to-ref image resampling (as well as inherent resampling for ref image)

    bool b_interp_search;
    /*With this option, analysis threshold is applied only to ref dose, regardless of the corresponding dose of compare image.*/
    bool b_ref_only_threshold;

public:
    Gamma_parms () {
        dose_tolerance = .03f;
        dta_tolerance = 3.f;
        have_reference_dose = false;
        reference_dose = 0.f;
        gamma_max = 2.0f;		
        b_local_gamma = false;
        b_compute_full_region = false;
        f_inherent_resample_mm = -1.0;
        f_analysis_threshold = 0.1;
        b_resample_nn = false;
        b_interp_search = false;
        b_ref_only_threshold = false;
    }
};

void do_command_gamma (int argc, char *argv[]);

#endif
