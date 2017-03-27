/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_state_h_
#define _bspline_state_h_

#include "plmregister_config.h"
#include <list>
#include <string>

#include "bspline_regularize.h"
#include "bspline_score.h"
#include "metric_state.h"
#include "plm_int.h"
#include "smart_pointer.h"

class Bspline_state_private;
class Bspline_parms;
class Joint_histogram;

class PLMREGISTER_API Bspline_state {
public:
    SMART_POINTER_SUPPORT (Bspline_state);
    Bspline_state_private *d_ptr;
public:
    Bspline_state ();
    ~Bspline_state ();
public:
    int sm;                         /* Current smetric */
    int it;                         /* Current iterations */
    int feval;                      /* Number of function evaluations */
    Bspline_score ssd;              /* Score and Gradient  */
    void* dev_ptrs;                 /* GPU Device Pointers */

    /* Similarity metric */
    std::list<Metric_state::Pointer> similarity_data;

    /*! \brief Current similarity images.  These are raw pointers 
     because they are passed to CUDA code.  */
    Volume *fixed;
    Volume *moving;
    Volume *moving_grad;
    Volume *fixed_roi;
    Volume *moving_roi;
    
    Bspline_regularize rst;

protected:
    /*! \brief Current joint histogram.  This is raw pointer 
      because it is passed to CUDA code.  */
    Joint_histogram *mi_hist;

public:
    void initialize (Bspline_xform *bxf, Bspline_parms *parms);
    void initialize_similarity_images ();
    void initialize_mi_histograms ();
    void set_metric_state (const Metric_state::Pointer& ms);
    Bspline_score* get_bspline_score () {
        return &ssd;
    }
    Joint_histogram* get_mi_hist () {
        return mi_hist;
    }
    bool has_metric_type (Similarity_metric_type metric_type);
    void log_metric ();
};

#endif
