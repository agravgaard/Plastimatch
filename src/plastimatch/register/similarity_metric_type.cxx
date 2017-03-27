/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <string.h>
#include "similarity_metric_type.h"

const char* 
similarity_metric_type_string (Similarity_metric_type type)
{
    switch (type) {
    case SIMILARITY_METRIC_NONE:
        return "none";
    case SIMILARITY_METRIC_GM:
        return "GM";
    case SIMILARITY_METRIC_MI_MATTES:
        return "MI";
    case SIMILARITY_METRIC_MI_VW:
        return "MIVW";
    case SIMILARITY_METRIC_MSE:
        return "MSE";
    case SIMILARITY_METRIC_NMI:
        return "NMI";
    default:
        return "(unkn)";
    }
}
