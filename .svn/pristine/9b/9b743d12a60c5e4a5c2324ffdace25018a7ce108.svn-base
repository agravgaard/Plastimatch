/* -----------------------------------------------------------------------
 *    See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
 *       ----------------------------------------------------------------------- */
#ifndef _mabs_staple_h_
#define _mabs_staple_h_

#include "plmsegment_config.h"

#include <iostream>
#include <list>

#include "plm_image.h"

class PLMSEGMENT_API Mabs_staple {

public:
    Mabs_staple();
    ~Mabs_staple();
    void add_input_structure(Plm_image::Pointer&);
    void set_confidence_weight(float confidence_weight);
    void run();

public:
    std::list<Plm_image::Pointer> structures;
    
    int foreground_val;
    float confidence_weight;

    Plm_image::Pointer output_img;

};

#endif /* #ifndef _mabs_staple_h_ */
