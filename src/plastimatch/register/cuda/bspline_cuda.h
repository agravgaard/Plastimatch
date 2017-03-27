/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_cuda_h_
#define _bspline_cuda_h_

#include "plmregister_config.h"

#if (CUDA_FOUND)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <cuda.h>

#include "cuda_mem.h"
#include "delayload.h"
#include "plm_int.h"

/* B-Spline CUDA MI Switches */
//#define MI_HISTS_CPU
//#define MI_GRAD_CPU
#define MI_SCORE_CPU

/* Uncomment to profile CUDA MSE */
//#define PROFILE_MSE

/* HARDWARE IMPOSED CONSTANTS */
#define GPU_MAX_BINS 32

class Joint_histogram;
class Bspline_optimize;
class Bspline_parms;
class Bspline_state;
class Bspline_xform;
class Volume;

typedef struct dev_pointers_bspline Dev_Pointers_Bspline;
struct dev_pointers_bspline
{
    // IMPORTANT!
    // Each member of this struct is a POINTER TO
    // AN ADDRESS RESIDING IN THE GPU'S GLOBAL
    // MEMORY!  Care must be taken when referencing
    // and dereferencing members of this structure!

    float* fixed_image;     // Fixed Image Voxels
    float* moving_image;    // Moving Image Voxels
    float* moving_grad;     // dc_dp (Gradient) Volume

    float* coeff;           // B-Spline coefficients (p)
    float* score;           // The "Score"

    float* dc_dv_x;         // dc_dv (De-Interleaved)
    float* dc_dv_y;         // dc_dv (De-Interleaved)
    float* dc_dv_z;         // dc_dv (De-Interleaved)

    float* cond_x;          // dc_dv_x (Condensed)
    float* cond_y;          // dc_dv_y (Condensed)
    float* cond_z;          // dc_dv_z (Condensed)

    float* grad;            // dc_dp

    float* f_hist_seg;      // "Segmented" fixed histogram
    float* m_hist_seg;      // "Segmented" moving histogram
    float* j_hist_seg;      // "Segmented" joint histogram

    float* f_hist;          // fixed image histogram
    float* m_hist;          // moving image histogram
    float* j_hist;          // joint histogram

    int* LUT_Knot;          // Control Point LUT
    int* LUT_Offsets;       // Tile Offset LUT
    float* LUT_Bspline_x;   // Pre-computed Bspline evaluations
    float* LUT_Bspline_y;   // ------------ '' ----------------
    float* LUT_Bspline_z;   // ------------ '' ----------------

    // # of voxels that do not have a correspondence
    float* skipped;                 // Legacy (for GPU w/o atomics)
    unsigned int* skipped_atomic;   // New (for GPU w/ Global atomics)

    // Head of linked list tracking pinned CPU memory
    // NOTE: This is the only pointer in this struct containing
    //       a pointer from the CPU memory map.
    Vmem_Entry* vmem_list;

    // Sizes of allocations for above pointers.
    plm_long fixed_image_size;
    plm_long moving_image_size;
    plm_long moving_grad_size;

    plm_long coeff_size;
    plm_long score_size;

    plm_long dc_dv_x_size;
    plm_long dc_dv_y_size;
    plm_long dc_dv_z_size;

    plm_long cond_x_size;
    plm_long cond_y_size;
    plm_long cond_z_size;

    plm_long grad_size;

    plm_long f_hist_seg_size;
    plm_long m_hist_seg_size;
    plm_long j_hist_seg_size;

    plm_long f_hist_size;
    plm_long m_hist_size;
    plm_long j_hist_size;

    plm_long LUT_Knot_size;
    plm_long LUT_Offsets_size;
    plm_long LUT_Bspline_x_size;
    plm_long LUT_Bspline_y_size;
    plm_long LUT_Bspline_z_size;
    plm_long skipped_size;
};

#if defined __cplusplus
extern "C" {
#endif

    // -------------------------------------------------------------------
    // Prototypes: bspline_cuda.cpp 

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    void CUDA_bspline_mi_a,
        Bspline_parms *parms,
        Bspline_state *bst,
        Bspline_xform *bxf
    );

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    void CUDA_bspline_mse_j,
        Bspline_parms *parms,
        Bspline_state *bst,
        Bspline_xform *bxf
    );

    //
    // -------------------------------------------------------------------




    // -------------------------------------------------------------------
    // Prototypes: bspline_cuda.cu

    void
    CUDA_bspline_mse_pt1 (
        Volume* fixed,
        Volume* moving,
        Volume* moving_grad,
        Bspline_xform* bxf,
        Bspline_parms* parms,
        Dev_Pointers_Bspline* dev_ptrs
    );

    void
    CUDA_bspline_mse_pt2 (
        Bspline_parms* parms,
        Bspline_xform* bxf,
        Volume* fixed,
        plm_long* vox_per_rgn,
        plm_long* volume_dim,
        float* host_score,
        float* host_grad,
        float* host_grad_mean,
        float* host_grad_norm,
        Dev_Pointers_Bspline* dev_ptrs,
        plm_long* num_vox
    );

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    void CUDA_bspline_mse_init_j,
        Dev_Pointers_Bspline* dev_ptrs,
        Volume* fixed,
        Volume* moving,
        Volume* moving_grad,
        Bspline_xform* bxf,
        Bspline_parms* parms
    );

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    void CUDA_bspline_mse_cleanup_j,
        Dev_Pointers_Bspline* dev_ptrs,
        Volume* fixed,
        Volume* moving,
        Volume* moving_grad
    );

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    void CUDA_bspline_mi_cleanup_a,
        Dev_Pointers_Bspline* dev_ptrs,
        Volume* fixed,
        Volume* moving,
        Volume* moving_grad
    );

    void
    CUDA_bspline_push_coeff (
        Dev_Pointers_Bspline* dev_ptrs,
        Bspline_xform* bxf
    );
    
    void
    CUDA_bspline_zero_score (
        Dev_Pointers_Bspline* dev_ptrs
    );
    
    void
    CUDA_bspline_zero_grad (
        Dev_Pointers_Bspline* dev_ptrs
    );

    void
    CUDA_bspline_mse_score_dc_dv (
        Dev_Pointers_Bspline* dev_ptrs,
        Bspline_xform* bxf,
        Volume* fixed,
        Volume* moving
    );

    void
    CUDA_bspline_condense (
        Dev_Pointers_Bspline* dev_ptrs,
        plm_long* vox_per_rgn,
        int num_tiles
    );

    void
    CUDA_bspline_reduce (
        Dev_Pointers_Bspline* dev_ptrs,
        int num_knots
    );

    float
    CPU_obtain_bspline_basis_function (
        int t_idx,
        int vox_idx,
        int vox_per_rgn
    ); 
    
    void
    CPU_find_knots (
        int* knots,
        int tile_num,
        plm_long* rdims,
        plm_long* cdims
    );

    int*
    CPU_calc_offsets (
        plm_long* tile_dims,
        plm_long* cdims
    );

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    void CUDA_bspline_mi_init_a,
        Bspline_xform* bxf,
        Bspline_state* bst,
        Dev_Pointers_Bspline* dev_ptrs,
        Volume* fixed,
        Volume* moving,
        Volume* moving_grad
    );

    int
    CUDA_bspline_mi_hist (
        Dev_Pointers_Bspline *dev_ptrs,
        Joint_histogram* mi_hist,
        Volume* fixed,
        Volume* moving,
        Bspline_xform *bxf
    );

    void
    CUDA_bspline_mi_hist_fix (
        Dev_Pointers_Bspline *dev_ptrs,
        Joint_histogram* mi_hist,
        Volume* fixed,
        Volume* moving,
        Bspline_xform *bxf
    );
    
    void
    CUDA_bspline_mi_hist_mov (
        Dev_Pointers_Bspline *dev_ptrs,
        Joint_histogram* mi_hist,
        Volume* fixed,
        Volume* moving,
        Bspline_xform *bxf
    );
    
    int
    CUDA_bspline_mi_hist_jnt (
        Dev_Pointers_Bspline *dev_ptrs,
        Joint_histogram* mi_hist,
        Volume* fixed,
        Volume* moving,
        Bspline_xform *bxf
    );

    void
    CUDA_bspline_mi_grad (
        Bspline_state *bst,
        Bspline_xform *bxf,
        Volume* fixed,
        Volume* moving,
        float num_vox_f,
        float score,
        Dev_Pointers_Bspline *dev_ptrs
    );

    PLMREGISTERCUDA_API
    DELAYLOAD_WRAP (
    void CUDA_bspline_interpolate_vf,
        Volume* interp,
        Bspline_xform* bxf
    );
    //
    // -------------------------------------------------------------------

#if defined __cplusplus
}
#endif

#endif /* CUDA_FOUND */

#endif
