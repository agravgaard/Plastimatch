/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"

#include "joint_histogram.h"
#include "metric_state.h"

Metric_state::Metric_state ()
{
    metric_type = SIMILARITY_METRIC_MSE;
    metric_lambda = 1.f;
    mi_hist = 0;
}

Metric_state::~Metric_state ()
{
    delete mi_hist;
}
