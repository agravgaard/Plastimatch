/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _metric_parms_h_
#define _metric_parms_h_

#include "plmregister_config.h"
#include <string>
#include <vector>
#include "plm_return_code.h"
#include "similarity_metric_type.h"

class PLMREGISTER_API Metric_parms {
public:
    Metric_parms ();
public:
    Similarity_metric_type metric_type;
    float metric_lambda;

    std::string fixed_fn;
    std::string moving_fn;
    std::string fixed_roi_fn;
    std::string moving_roi_fn;
public:
    Plm_return_code set_metric_type (const std::string& val);
};

#endif
