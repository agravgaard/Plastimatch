/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _segment_body_h_
#define _segment_body_h_

#include "plmsegment_config.h"

#include "itk_image_type.h"

class Plm_image;

class PLMSEGMENT_API Segment_body {
  public:
    Plm_image *img_in;
    Plm_image *img_out;

    bool m_bot_given;
    float m_bot;
    bool m_debug;
    bool m_fast;
    bool m_fill_holes;

    float m_lower_threshold;
    float m_upper_threshold;

    int m_fill_parms [6];

  public:
    Segment_body () {
        m_bot_given = false;
        m_debug = false;
        m_fast = false;
        m_fill_holes = false;
        m_lower_threshold = -300;

        m_fill_parms[0]= 7;
        m_fill_parms[1]= 5;
        m_fill_parms[2]= 2;
        m_fill_parms[3]= 2;
        m_fill_parms[4]= 10;
        m_fill_parms[5]= 50;
    }
    void do_segmentation ();
    void do_segmentation_air_cavity();
    FloatImageType::Pointer reduce_image_dim (FloatImageType::Pointer i1);
    UCharImageType::Pointer threshold_patient (FloatImageType::Pointer i1);
    int find_patient_bottom (FloatImageType::Pointer i1);
    UCharImageType::Pointer fill_holes (UCharImageType::Pointer mask, 
        int radius, int max_its);
};

#endif
