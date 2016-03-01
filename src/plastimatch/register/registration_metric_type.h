/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _registration_metric_type_h_
#define _registration_metric_type_h_

enum Registration_metric_type {
    REGISTRATION_METRIC_NONE,
    REGISTRATION_METRIC_GM,
    REGISTRATION_METRIC_MI_MATTES,
    REGISTRATION_METRIC_MI_VW,
    REGISTRATION_METRIC_MSE,
    REGISTRATION_METRIC_NMI
};

const char* registration_metric_type_string (Registration_metric_type);

#endif
