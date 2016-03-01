/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pcmd_threshold_h_
#define _pcmd_threshold_h_

#include "plmcli_config.h"
#include <string>
#include <string.h>
#include <stdlib.h>
#include "plm_image_type.h"

class Pcmd_threshold {
public:
    std::string img_in_fn;
    std::string img_out_fn;

    /* threshold string */
    std::string range_string;

    bool output_dicom;
    Plm_image_type output_type;
public:
    Pcmd_threshold () {
	output_dicom = false;
	output_type = PLM_IMG_TYPE_UNDEFINED;
    }
};

void do_command_threshold (int argc, char *argv[]);

#endif
