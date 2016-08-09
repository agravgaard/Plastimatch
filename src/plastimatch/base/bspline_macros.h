/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_macros_h_
#define _bspline_macros_h_

#include "plmbase_config.h"
#include "bspline_xform.h"
#include "plm_int.h"
#include "volume_macros.h"

/***************************************************************
 * MACROS FOR VOXEL CENTRIC ALGORITHMS                         *
 ***************************************************************/

/* ITERATORS */

/* For looping through volume ROI.  Here: 
 *    roi_ijk - coordinates within ROI
 *    vol_ijk - volume coordinate of ROI point roi_ijk
 */
#define LOOP_THRU_ROI_X(roi_ijk, vol_ijk, bxf) \
    for (roi_ijk[0] = 0, vol_ijk[0] = bxf->roi_offset[0]; roi_ijk[0] < bxf->roi_dim[0]; roi_ijk[0]++, vol_ijk[0]++)

#define LOOP_THRU_ROI_Y(roi_ijk, vol_ijk, bxf) \
    for (roi_ijk[1] = 0, vol_ijk[1] = bxf->roi_offset[1]; roi_ijk[1] < bxf->roi_dim[1]; roi_ijk[1]++, vol_ijk[1]++)

#define LOOP_THRU_ROI_Z(roi_ijk, vol_ijk, bxf) \
    for (roi_ijk[2] = 0, vol_ijk[2] = bxf->roi_offset[2]; roi_ijk[2] < bxf->roi_dim[2]; roi_ijk[2]++, vol_ijk[2]++)


/* COORDINATE MANIPULATION MACROS */
#define REGION_INDEX_X(ijk, bxf) \
    (ijk[0] / bxf->vox_per_rgn[0])

#define REGION_INDEX_Y(ijk, bxf) \
    (ijk[1] / bxf->vox_per_rgn[1])

#define REGION_INDEX_Z(ijk, bxf) \
    (ijk[2] / bxf->vox_per_rgn[2])

static inline void
get_region_index_3 (int p[3], const int ijk[3], const Bspline_xform *bxf) {
    p[0] = ijk[0] / bxf->vox_per_rgn[0];
    p[1] = ijk[1] / bxf->vox_per_rgn[1];
    p[2] = ijk[2] / bxf->vox_per_rgn[2];
}

static inline void
get_region_index_3 (int p[3], int i, int j, int k, const Bspline_xform *bxf) {
    p[0] = i / bxf->vox_per_rgn[0];
    p[1] = j / bxf->vox_per_rgn[1];
    p[2] = k / bxf->vox_per_rgn[2];
}

static inline plm_long
get_region_index (const plm_long ijk[3], const Bspline_xform *bxf) {
    plm_long p[3];
    p[0] = ijk[0] / bxf->vox_per_rgn[0];
    p[1] = ijk[1] / bxf->vox_per_rgn[1];
    p[2] = ijk[2] / bxf->vox_per_rgn[2];
    return volume_index (bxf->rdims, p);
}

static inline plm_long
get_region_index (plm_long i, plm_long j, plm_long k, const Bspline_xform *bxf) {
    plm_long p[3];
    p[0] = i / bxf->vox_per_rgn[0];
    p[1] = j / bxf->vox_per_rgn[1];
    p[2] = k / bxf->vox_per_rgn[2];
    return volume_index (bxf->rdims, p);
}

#define REGION_OFFSET_X(ijk, bxf) \
    (ijk[0] % bxf->vox_per_rgn[0]) 

#define REGION_OFFSET_Y(ijk, bxf) \
    (ijk[1] % bxf->vox_per_rgn[1]) 

#define REGION_OFFSET_Z(ijk, bxf) \
    (ijk[2] % bxf->vox_per_rgn[2]) 

static inline void
get_region_offset (int q[3], const int ijk[3], const Bspline_xform *bxf) {
    q[0] = ijk[0] % bxf->vox_per_rgn[0];
    q[1] = ijk[1] % bxf->vox_per_rgn[1];
    q[2] = ijk[2] % bxf->vox_per_rgn[2];
}

static inline void
get_region_offset (int q[3], int i, int j, int k, const Bspline_xform *bxf) {
    q[0] = i % bxf->vox_per_rgn[0];
    q[1] = j % bxf->vox_per_rgn[1];
    q[2] = k % bxf->vox_per_rgn[2];
}

static inline plm_long
get_region_offset (const plm_long ijk[3], const Bspline_xform *bxf) {
    plm_long q[3];
    q[0] = ijk[0] % bxf->vox_per_rgn[0];
    q[1] = ijk[1] % bxf->vox_per_rgn[1];
    q[2] = ijk[2] % bxf->vox_per_rgn[2];
    return volume_index (bxf->vox_per_rgn, q);
}

static inline plm_long
get_region_offset (plm_long i, plm_long j, plm_long k, const Bspline_xform *bxf) {
    plm_long q[3];
    q[0] = i % bxf->vox_per_rgn[0];
    q[1] = j % bxf->vox_per_rgn[1];
    q[2] = k % bxf->vox_per_rgn[2];
    return volume_index (bxf->vox_per_rgn, q);
}

#define GET_WORLD_COORD_X_NO_DCOS(ijk_vol, bxf)                 \
    (bxf->img_origin[0] + bxf->img_spacing[0] * ijk_vol[0])

#define GET_WORLD_COORD_Y_NO_DCOS(ijk_vol, bxf)                 \
    (bxf->img_origin[1] + bxf->img_spacing[1] * ijk_vol[1])

#define GET_WORLD_COORD_Z_NO_DCOS(ijk_vol, bxf)                 \
    (bxf->img_origin[2] + bxf->img_spacing[2] * ijk_vol[2])

#define GET_WORLD_COORD_X(ijk_vol, vol, bxf)                            \
    (bxf->img_origin[0]                                                 \
        + ijk_vol[0]*vol->step[0*3+0]                                   \
        + ijk_vol[1]*vol->step[0*3+1]                                   \
        + ijk_vol[2]*vol->step[0*3+2])

#define GET_WORLD_COORD_Y(ijk_vol, vol, bxf)                            \
    (bxf->img_origin[1]                                                 \
        + ijk_vol[0]*vol->step[1*3+0]                                   \
        + ijk_vol[1]*vol->step[1*3+1]                                   \
        + ijk_vol[2]*vol->step[1*3+2])

#define GET_WORLD_COORD_Z(ijk_vol, vol, bxf)                            \
    (bxf->img_origin[2]                                                 \
        + ijk_vol[0]*vol->step[2*3+0]                                   \
        + ijk_vol[1]*vol->step[2*3+1]                                   \
        + ijk_vol[2]*vol->step[2*3+2])

/***************************************************************
 * MACROS FOR THE "PARALLEL FRIENDLY" TILE-CENTRIC ALGORITHMS  *
 ***************************************************************/

/* ITERATORS */

/* For linearlly cycling through regions/tiles in the volume */
#define LOOP_THRU_VOL_TILES(idx_tile, bxf)                              \
    for (idx_tile = 0; idx_tile < (bxf->rdims[0] * bxf->rdims[1] * bxf->rdims[2]); idx_tile++)

/* For cycling through local coordinates within a tile */
#define LOOP_THRU_TILE_X(ijk_local, bxf)                                     \
    for (ijk_local[0]=0; ijk_local[0] < bxf->vox_per_rgn[0]; ijk_local[0]++)  

#define LOOP_THRU_TILE_Y(ijk_local, bxf)                                     \
    for (ijk_local[1]=0; ijk_local[1] < bxf->vox_per_rgn[1]; ijk_local[1]++)  

#define LOOP_THRU_TILE_Z(ijk_local, bxf)                                     \
    for (ijk_local[2]=0; ijk_local[2] < bxf->vox_per_rgn[2]; ijk_local[2]++)  



/* COORDINATE MANIPULATION MACROS */

/* Get volume coordinates given tile coordinates and local coordinates */
#define GET_VOL_COORDS(ijk_vol, ijk_tile, ijk_local, bxf)                                \
    do {                                                                                 \
    ijk_vol[0] = bxf->roi_offset[0] + bxf->vox_per_rgn[0] * ijk_tile[0] + ijk_local[0];  \
    ijk_vol[1] = bxf->roi_offset[1] + bxf->vox_per_rgn[1] * ijk_tile[1] + ijk_local[1];  \
    ijk_vol[2] = bxf->roi_offset[2] + bxf->vox_per_rgn[2] * ijk_tile[2] + ijk_local[2];  \
    } while (0);


/* Get real-space coordinates from a set of volume indices */
#define GET_REAL_SPACE_COORDS(xyz_vol, ijk_vol, bxf)			\
    do {								\
	xyz_vol[0] = bxf->img_origin[0] + bxf->img_spacing[0] * ijk_vol[0]; \
	xyz_vol[1] = bxf->img_origin[1] + bxf->img_spacing[1] * ijk_vol[1]; \
	xyz_vol[2] = bxf->img_origin[2] + bxf->img_spacing[2] * ijk_vol[2]; \
    } while (0);

/* Get real-space coordinates from a set of volume indices */
#if defined (commentout)
#define GET_WORLD_COORDS(xyz_vol, ijk_vol, vol, bxf)                    \
    VOXEL_COORDS (xyz_vol, ijk_vol, bxf->img_origin, vol->step)
#endif

#endif /* _bspline_macros_h_ */
