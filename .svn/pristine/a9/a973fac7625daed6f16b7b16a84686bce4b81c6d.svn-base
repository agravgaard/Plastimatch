/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_volume_header_h_
#define _itk_volume_header_h_

#include "plmbase_config.h"
#include "itk_image_type.h"

class Bspline_xform;

class PLMBASE_API Itk_volume_header {
public:
    RegionType m_region;
    OriginType m_origin;
    SpacingType m_spacing;
    DirectionType m_direction;

public:
    Itk_volume_header () {}
    Itk_volume_header (
        float origin[3], float spacing[3], plm_long dim[3])
    {
        this->set_from_gpuit (origin, spacing, dim, 0);
    }
    Itk_volume_header (
        float origin[3], float spacing[3],
        plm_long dim[3], float direction_cosines[9])
    {
        this->set_from_gpuit (origin, spacing, dim, direction_cosines);
    }

public:
    int Size (int d) const { return m_region.GetSize()[d]; }

public:
    /* Return 1 if the two headers are the same */
    static int compare (Itk_volume_header *pli1, Itk_volume_header *pli2);

public:
    void set_origin (float origin[3]);
    void set_spacing (float spacing[3]);
    void set_dim (plm_long dim[3]);
    void 
    set_from_gpuit (float gpuit_origin[3],
                    float gpuit_spacing[3],
                    plm_long gpuit_dim[3],
                    float gpuit_direction_cosines[9]);
    void 
    set_from_gpuit_bspline (Bspline_xform *bxf);
    template<class T> 
    void set_from_itk_image (const T image) {
        m_origin = image->GetOrigin ();
        m_spacing = image->GetSpacing ();
        m_region = image->GetLargestPossibleRegion ();
        m_direction = image->GetDirection ();
    }
    static void clone (Itk_volume_header *dest, Itk_volume_header *src) {
        dest->m_origin = src->m_origin;
        dest->m_spacing = src->m_spacing;
        dest->m_region = src->m_region;
        dest->m_direction = src->m_direction;
    }

    void get_origin (float origin[3]);
    void get_spacing (float spacing[3]);
    void get_dim (plm_long dim[3]);
    void get_direction_cosines (
        float direction_cosines[9]);

    void print (void) const;
    void get_image_center (float center[3]);
};

/* -----------------------------------------------------------------------
   Global functions
   ----------------------------------------------------------------------- */
void
direction_cosines_from_itk (
    float direction_cosines[9],
    DirectionType* itk_direction
);

#endif
