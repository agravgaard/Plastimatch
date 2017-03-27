/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _histogram_h_
#define _histogram_h_

#include "plmregister_config.h"
#include <string>
#include "plm_int.h"

/* Maximum # of bins for a vopt histogram */
#define VOPT_RES 1000

enum Mi_hist_type {
    HIST_EQSP,
    HIST_VOPT
};

class PLMREGISTER_API Histogram {
public:
    Histogram (
        Mi_hist_type type = HIST_EQSP, 
        plm_long bins = 32);
    ~Histogram ();
public:
    /* Used by all histogram types */
    enum Mi_hist_type type;   /* Type of histograms */
    plm_long bins;                    /* # of bins in histogram  */
    float offset;                     /* minimum voxel intensity */
    plm_long big_bin;                 /* fullest bin index       */
    float delta;                      /* bin OR key spacing   */

    /* For V-Optimal Histograms */
    plm_long keys;                    /* # of keys               */
    int* key_lut;                     /* bin keys lookup table   */
};

#endif
