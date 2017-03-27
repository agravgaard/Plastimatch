/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <string>
#include "logfile.h"
#include "metric_parms.h"
#include "string_util.h"

Metric_parms::Metric_parms ()
{
    metric_type = SIMILARITY_METRIC_MSE;
    metric_lambda = 1.0;
}

Plm_return_code
Metric_parms::set_metric_type (const std::string& val)
{
    if (val == "dm" || val == "dmap") {
        this->metric_type = SIMILARITY_METRIC_DMAP;
        return PLM_SUCCESS;
    }
    else if (val == "gm") {
        this->metric_type = SIMILARITY_METRIC_GM;
        return PLM_SUCCESS;
    }
    else if (val == "mattes") {
        this->metric_type = SIMILARITY_METRIC_MI_MATTES;
        return PLM_SUCCESS;
    }
    else if (val == "mse" || val == "MSE") {
        this->metric_type = SIMILARITY_METRIC_MSE;
        return PLM_SUCCESS;
    }
    else if (val == "mi" || val == "MI") {
#if PLM_CONFIG_LEGACY_MI_METRIC
        this->metric_type = SIMILARITY_METRIC_MI_VW;
#else
        this->metric_type = SIMILARITY_METRIC_MI_MATTES;
#endif
        return PLM_SUCCESS;
    }
    else if (val == "mi_vw" || val == "viola-wells") {
        this->metric_type = SIMILARITY_METRIC_MI_VW;
        return PLM_SUCCESS;
    }
    else if (val == "nmi" || val == "NMI") {
        this->metric_type = SIMILARITY_METRIC_NMI;
        return PLM_SUCCESS;
    }
    else {
        return PLM_ERROR;
    }
}
