/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rtss_contour_h_
#define _rtss_contour_h_

#include "plmbase_config.h"
#include <string>

class PLMBASE_API Rtss_contour {
public:
    int slice_no;           /* Can be "-1" */
    std::string ct_slice_uid;
    size_t num_vertices;
    float* x;
    float* y;
    float* z;
public:
    Rtss_contour ();
    ~Rtss_contour ();
public:
    void find_direction_cosines ();
};

#endif
