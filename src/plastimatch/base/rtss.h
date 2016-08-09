/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rtss_h_
#define _rtss_h_

#include "plmbase_config.h"
#include <list>
#include <vector>

#include "direction_cosines.h"
#include "plm_int.h"
#include "rt_study_metadata.h"
#include "smart_pointer.h"

class Plm_image;
class Plm_image_header;
class Rtss_roi;
class Slice_list;

/*! \brief 
 * The Rtss class represents a set of segmentations in polyline format, 
 * analogous to the DICOM-RT RTSTRUCT object.
 */
class PLMBASE_API Rtss {
public:
    SMART_POINTER_SUPPORT (Rtss);
public:
    /* Output geometry */
    int have_geometry;
    plm_long m_dim[3];
    float m_spacing[3];
    float m_offset[3];
    /* Rasterization geometry */
    plm_long rast_dim[3];
    float rast_spacing[3];
    float rast_offset[3];
    Direction_cosines rast_dc;
    /* Structures */
    size_t num_structures;
    Rtss_roi **slist;
public:
    Rtss ();
    ~Rtss ();
    void init (void);
    void clear (void);
    Rtss_roi* add_structure (
        const std::string& structure_name, 
        const std::string& color, 
	int structure_id,
        int bit = -1);
    void delete_structure (int index);
    Rtss_roi* find_structure_by_id (int structure_id);
    std::string get_structure_name (size_t index);
    void set_structure_name (size_t index, const std::string& name);
    void debug (void);
    void adjust_structure_names (void);
    void prune_empty (void);
    static Rtss* clone_empty (Rtss* cxt_out, 
        Rtss* cxt_in);
    void find_rasterization_geometry (float offset[3], 
	float spacing[3], plm_long dims[3], Direction_cosines& dc);
    void find_rasterization_geometry (Plm_image_header *pih);
    std::string find_unused_structure_name (void);
    void fix_polyline_slice_numbers (void);
    /*! \brief Copy slice UIDs from referenced image into the Rtss object. */
    void apply_slice_list (const Rt_study_metadata::Pointer& rsm);
    /*! \brief Copy slice UIDs from referenced image into the Rtss object. */
    void apply_slice_list (const Slice_list *slice_list);
    void free_all_polylines (void);
    void keyholize (void);
    void set_rasterization_geometry (void);
    void set_geometry (const Plm_image_header *pih);
    void set_geometry (const Plm_image::Pointer& pli);
};

#endif
