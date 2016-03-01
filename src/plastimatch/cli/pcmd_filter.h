/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pcmd_filter_h_
#define _pcmd_filter_h_

#include "plmcli_config.h"

class Filter_parms {
public:
    enum Filter_type {
        FILTER_TYPE_UNDEFINED,
        FILTER_TYPE_GABOR,
        FILTER_TYPE_GAUSSIAN_COMBINED,
        FILTER_TYPE_GAUSSIAN_SEPARABLE,
        FILTER_TYPE_KERNEL
    };

public:
    std::string in_image_fn;
    std::string in_kernel_fn;
    std::string out_image_fn;
    std::string out_kernel_fn;

    Filter_type filter_type;
    float gauss_width;
    bool gabor_use_k_fib;
    int gabor_k_fib[2];

public:
    Filter_parms () {
        filter_type = FILTER_TYPE_UNDEFINED;
        gauss_width = 10.f;
        gabor_use_k_fib = false;
        gabor_k_fib[0] = 0;
        gabor_k_fib[1] = 1;
    }
};

void do_command_filter (int argc, char *argv[]);

#endif
