/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rtss_roi_h_
#define _rtss_roi_h_

#include "plmbase_config.h"
#include <string>

class Rtss_contour;

class PLMBASE_API Rtss_roi {
public:
    std::string name;
    std::string color;
    int id;                    /* Used for import/export (must be >= 1) */
    int bit;                   /* Used for ss-img (-1 for no bit) */
    size_t num_contours;
    Rtss_contour** pslist;
public:
    Rtss_roi ();
    ~Rtss_roi ();

    void clear ();
    Rtss_contour* add_polyline ();
    Rtss_contour* add_polyline (size_t num_vertices);
    void set_color (const char* color_string);
    std::string get_dcm_color_string () const;
    void get_rgb (int *r, int *g, int *b) const;

    static std::string adjust_name (const std::string& name_in);
};


#endif
