/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _thumbnail_h_
#define _thumbnail_h_

#include "plmbase_config.h"
#include "itk_image_type.h"
#include "plm_image.h"

class PLMBASE_API Thumbnail {
public:
    Plm_image::Pointer pli;
    float origin[3];
    float center[3];
    float spacing[3];
    plm_long dim[3];

    int axis;
    int thumbnail_dim;
    float thumbnail_spacing;
    float slice_loc;
    bool slice_loc_was_set;

public:
    Thumbnail ();
    void set_input_image (const Plm_image::Pointer& pli);
    void set_axis (int axis);
    void set_slice_loc (float slice_loc);
    void set_thumbnail_dim (int thumb_dim);
    void set_thumbnail_spacing (float thumb_spacing);
    FloatImageType::Pointer make_thumbnail ();
private:
    void set_internal_geometry (void);

};

#endif
