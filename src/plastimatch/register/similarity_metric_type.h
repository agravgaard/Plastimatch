/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _similarity_metric_type_h_
#define _similarity_metric_type_h_

enum Similarity_metric_type {
    SIMILARITY_METRIC_NONE,
    SIMILARITY_METRIC_DMAP,
    SIMILARITY_METRIC_GM,
    SIMILARITY_METRIC_MI_MATTES,
    SIMILARITY_METRIC_MI_VW,
    SIMILARITY_METRIC_MSE,
    SIMILARITY_METRIC_NMI
};

const char* similarity_metric_type_string (Similarity_metric_type);

#endif
