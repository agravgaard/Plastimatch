/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xio_ct_transform_h_
#define _xio_ct_transform_h_

#include "plmbase_config.h"

class Metadata;
class Plm_image;
class Rt_study_metadata;

class PLMBASE_API Xio_ct_transform {
public:
    float direction_cosines[9];
    float x_offset;
    float y_offset;
public:
    Xio_ct_transform ();
    Xio_ct_transform (const Metadata::Pointer& meta);
public:
    void set (const Metadata::Pointer& meta);
    void set (const char* ppos);
    void set_from_rdd (
        Plm_image *pli,
        Rt_study_metadata *rsm);
};

#endif
