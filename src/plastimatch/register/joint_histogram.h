/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _joint_histogram_h_
#define _joint_histogram_h_

#include "plmregister_config.h"
#include <string>
#include "histogram.h"
#include "plm_int.h"

class Volume;

class PLMREGISTER_API Joint_histogram {
public:
    Joint_histogram ();
    Joint_histogram (
        Mi_hist_type type,
        plm_long fixed_bins,
        plm_long moving_bins);
    ~Joint_histogram ();
public:
    void initialize (Volume *fixed, Volume *moving);
    void reset_histograms ();
    void dump_hist (int it, const std::string& prefix);

    void add_pvi_8 (
        const Volume *fixed, 
        const Volume *moving, 
        int fidx, 
        int mvf, 
        float li_1[3],      /* Fraction of interpolant in lower index */
        float li_2[3]);     /* Fraction of interpolant in upper index */

    float compute_score (int num_vox);

public:
    Histogram moving;
    Histogram fixed;
    Histogram joint;
    double* m_hist;
    double* f_hist;
    double* j_hist;
protected:
    void allocate ();
};

void dump_xpm_hist (Joint_histogram* mi_hist, char* file_base, int iter);

#endif
