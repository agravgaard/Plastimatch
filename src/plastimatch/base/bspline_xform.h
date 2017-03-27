/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_xform_h_
#define _bspline_xform_h_

#include "plmbase_config.h"
#include "direction_cosines.h"
#include "smart_pointer.h"
#include "plm_int.h"

//TODO: Change type of dc to Direction_cosines*

//class Direction_cosines;
class Volume;
class Volume_header;

/*! \brief 
 * The Bspline_xform class encapsulates the B-spline coefficients 
 * used by native registration and warping algorithms.
 */
class PLMBASE_API Bspline_xform {
public:
    SMART_POINTER_SUPPORT (Bspline_xform);
public:
    Bspline_xform ();
    ~Bspline_xform ();
public:
    enum Lut_type {
        LUT_ALIGNED,
        LUT_UNALIGNED
    };
public:
    float img_origin[3];         /* Image origin (in mm) */
    float img_spacing[3];        /* Image spacing (in mm) */
    plm_long img_dim[3];         /* Image size (in vox) */
    Direction_cosines dc;        /* Image direction cosines */
    plm_long roi_offset[3];	 /* Position of first vox in ROI (in vox) */
    plm_long roi_dim[3];	 /* Dimension of ROI (in vox) */
    plm_long vox_per_rgn[3];	 /* Knot spacing (in vox) */
    float grid_spac[3];          /* Knot spacing (in mm) */
    plm_long rdims[3];           /* # of regions in (x,y,z) */
    plm_long cdims[3];           /* # of knots in (x,y,z) */
    int num_knots;               /* Total number of knots (= product(cdims)) */
    int num_coeff;               /* Total number of coefficents (= product(cdims) * 3) */
    float* coeff;                /* Coefficients.  Vector directions interleaved. */

    Lut_type lut_type;           /* Which kind of LUT is used */

    /* Aligned grid (3D) LUTs */
    plm_long* cidx_lut;          /* Lookup volume for region number */
    plm_long* c_lut;             /* Lookup table for control point indices */
    plm_long* qidx_lut;          /* Lookup volume for region offset */
    float* q_lut;                /* Lookup table for influence multipliers */

    /* Aligned grid (1D) LUTs */
    float *bx_lut;               /* LUT for influence multiplier in x dir */
    float *by_lut;               /* LUT for influence multiplier in y dir */
    float *bz_lut;               /* LUT for influence multiplier in z dir */

    /* Unaligned grid (1D) LUTs */
    float *ux_lut;               /* LUT for influence multiplier in x dir */
    float *uy_lut;               /* LUT for influence multiplier in y dir */
    float *uz_lut;               /* LUT for influence multiplier in z dir */

public:
    void initialize (
        float img_origin[3],          /* Image origin (in mm) */
        float img_spacing[3],         /* Image spacing (in mm) */
        plm_long img_dim[3],          /* Image size (in vox) */
        plm_long roi_offset[3],       /* Position of first vox in ROI (in vox) */
        plm_long roi_dim[3],	      /* Dimension of ROI (in vox) */
        plm_long vox_per_rgn[3],      /* Knot spacing (in vox) */
        float direction_cosines[9]    /* Direction cosines */
    );
    void save (const char* filename);
    void fill_coefficients (float val);
    /*! \brief This function jitters the coefficients if they are all zero. 
     *  It is used to prevent local minima artifact when optimizing an MI cost 
     *  function for images with the same geometry.
     */
    void jitter_if_zero ();
    void get_volume_header (Volume_header *vh);
    void log_header ();
};

PLMBASE_C_API Bspline_xform* bspline_xform_load (const char* filename);

/* Debugging routines */
PLMBASE_C_API void bspline_xform_dump_coeff (Bspline_xform* bxf, const char* fn);
PLMBASE_C_API void bspline_xform_dump_luts (Bspline_xform* bxf);


#endif
