/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _metric_state_h_
#define _metric_state_h_

#include "plmregister_config.h"
#include "similarity_metric_type.h"
#include "volume.h"

class Joint_histogram;

class PLMREGISTER_API Metric_state
{
public:
    SMART_POINTER_SUPPORT (Metric_state);
public:
    Metric_state ();
    ~Metric_state ();
public:
    Volume::Pointer fixed_ss;
    Volume::Pointer moving_ss;
    Volume::Pointer fixed_grad;
    Volume::Pointer moving_grad;
    Volume::Pointer fixed_roi;
    Volume::Pointer moving_roi;

    Similarity_metric_type metric_type;
    float metric_lambda;

    Joint_histogram *mi_hist;

public:
    const char *metric_string () {
        return similarity_metric_type_string (metric_type);
    }
};

#endif
