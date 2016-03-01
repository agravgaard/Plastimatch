/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _volume_header_h_
#define _volume_header_h_

#include "plmbase_config.h"
#include "direction_cosines.h"
#include "volume.h"

class Bspline_xform;
class Plm_image_header;
class Volume;
class Volume_header_private;

class PLMBASE_API Volume_header {
public:
    Volume_header_private *d_ptr;

public:
    Volume_header ();
    Volume_header (plm_long dim[3], float origin[3], float spacing[3]);
    Volume_header (plm_long dim[3], float origin[3], float spacing[3],
        float direction_cosines[9]);
    Volume_header (const Volume::Pointer& vol);
    Volume_header (const Plm_image_header *pih);
    ~Volume_header ();

public:
    void set_dim (const plm_long dim[3]);
    plm_long* get_dim ();
    const plm_long* get_dim () const;

    void set_origin (const float origin[3]);
    float* get_origin ();
    const float* get_origin () const;

    void set_spacing (const float spacing[3]);
    float* get_spacing ();
    const float* get_spacing () const;

    void set_direction_cosines (const float direction_cosines[9]);
    void set_direction_cosines (const Direction_cosines& dc);
    void set_direction_cosines_identity ();
    Direction_cosines& get_direction_cosines ();
    const Direction_cosines& get_direction_cosines () const;

    void set (const plm_long dim[3], const float origin[3], 
        const float spacing[3], const float dc[9]);
    void set (const plm_long dim[3], const float origin[3], 
        const float spacing[3], const Direction_cosines& dc);
    void set_from_bxf (Bspline_xform *bxf);

public:
    void clone (const Volume_header *src);
    static void clone (Volume_header *dest, Volume_header *src);

public:
    void get_image_center (float center[3]);
    void print (void) const;

public:
    /* Return 1 if the two headers are the same */
    static int compare (Volume_header *pli1, Volume_header *pli2);
};

#endif
