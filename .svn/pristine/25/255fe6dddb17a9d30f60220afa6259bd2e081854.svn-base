/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _slice_list_h_
#define _slice_list_h_

#include "plmbase_config.h"

class Plm_image_header;
class Slice_list_private;

class PLMBASE_API Slice_list {
public:
    Slice_list_private *d_ptr;
    
public:
    Slice_list ();
    ~Slice_list ();
    const Plm_image_header* get_image_header () const;
    void set_image_header (const Plm_image_header& pih);
    void set_image_header (ShortImageType::Pointer img);

    void reset_slice_uids ();
    const char* get_slice_uid (int index) const;
    void set_slice_uid (int index, const char* slice_uid);
    bool slice_list_complete () const;
    void set_slice_list_complete ();

    int num_slices ();
    int get_slice_index (float z) const;
};

#endif
