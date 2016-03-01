/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <string.h>
#include "registration_metric_type.h"

const char* 
registration_metric_type_string (Registration_metric_type type)
{
    switch (type) {
    case REGISTRATION_METRIC_NONE:
        return "none";
    case REGISTRATION_METRIC_GM:
        return "GM";
    case REGISTRATION_METRIC_MI_MATTES:
        return "MI";
    case REGISTRATION_METRIC_MI_VW:
        return "MIVW";
    case REGISTRATION_METRIC_MSE:
        return "MSE";
    case REGISTRATION_METRIC_NMI:
        return "NMI";
    default:
        return "(unkn)";
    }
}
