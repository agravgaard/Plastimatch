/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xform_convert_h_
#define _xform_convert_h_

#include "plmbase_config.h"
#include "volume_header.h"
#include "xform.h"

class Xform_convert_private;

class PLMBASE_API Xform_convert {
public:
    Xform_convert_private *d_ptr;
public:
    Xform_type m_xf_out_type;
    Volume_header m_volume_header;
    float m_grid_spac[3];
    int m_nobulk;
public:
    Xform_convert ();
    ~Xform_convert ();
public:
    void run ();
    void set_input_xform (const Xform::Pointer& xf_in);
    Xform::Pointer& get_output_xform ();
};

#endif
