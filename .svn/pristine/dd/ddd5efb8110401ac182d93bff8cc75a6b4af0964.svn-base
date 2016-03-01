/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _mabs_seg_weights_h_
#define _mabs_seg_weights_h_

#include "plmsegment_config.h"
#include <list>
#include <string>
#include "logfile.h"

class PLMSEGMENT_API Mabs_seg_weights {
public:
    Mabs_seg_weights () {
        this->factory_reset ();
    }
public:
    std::string structure;
    float rho;
    float sigma;
    float minsim;
    /* This is a string because the thresholding function can efficiently 
       perform multiple thresholds during training */
    std::string thresh;
    float confidence_weight;
public:
    void factory_reset () {
        rho = 0.5;
        sigma = 1.5;
        minsim = 0.25;
        thresh = "0.4";
        confidence_weight = 1e-8;
    }        
    void print () const;
};


class PLMSEGMENT_API Mabs_seg_weights_list {
public:
    Mabs_seg_weights default_weights;
    std::list<Mabs_seg_weights> weights_list;
public:
    void push_back (const Mabs_seg_weights& new_weights) {
        lprintf ("MSW: pushing new entry\n");
        new_weights.print();
        this->weights_list.push_back (new_weights);
    }
    Mabs_seg_weights& front () {
        return this->weights_list.front ();
    }
    const Mabs_seg_weights*
        find (const std::string& structure) const;
};


#endif
