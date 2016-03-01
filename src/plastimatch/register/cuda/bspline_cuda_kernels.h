/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_cuda_kernels_h_
#define _bspline_cuda_kernels_h_

// NOTE: Cannot be included in C or C++ files due to
//       special CUDA types such as int4, dim3, etc.
//       Can only be included in CUDA files.


typedef struct gpu_bspline_data GPU_Bspline_Data;
struct gpu_bspline_data
{
    // bxf items
    int3 rdims;         
    int3 cdims;
    float3 img_origin;      
    float3 img_spacing;
    int3 roi_dim;           
    int3 roi_offset;        
    int3 vox_per_rgn;       

    // fixed volume items
    int3 fix_dim;

    // moving volume items
    int3 mov_dim;       
    float3 mov_origin;
    float3 mov_spacing;
};


    /* Function prototypes of kernels */
    __global__ void
    kernel_bspline_condense (
        float* cond_x,          // Return: condensed dc_dv_x values
        float* cond_y,          // Return: condensed dc_dv_y values
        float* cond_z,          // Return: condensed dc_dv_z values
        float* dc_dv_x,         // Input : dc_dv_x values
        float* dc_dv_y,         // Input : dc_dv_y values
        float* dc_dv_z,         // Input : dc_dv_z values
        int* LUT_Tile_Offsets,  // Input : tile offsets
        int* LUT_Knot,          // Input : linear knot indicies
        int pad,                // Input : amount of tile padding
        int4 tile_dim,          // Input : dims of tiles
        float one_over_six      // Input : Precomputed (GPU division is slow)
    );

    __global__ void
    kernel_bspline_reduce (
        float* grad,            // Return: interleaved dc_dp values
        float* cond_x,          // Input : condensed dc_dv_x values
        float* cond_y,          // Input : condensed dc_dv_y values
        float* cond_z           // Input : condensed dc_dv_z values
    );

    __global__ void
    kernel_bspline_grad_normalize (
        float *grad,
        int num_vox,
        int num_elems
    );


    __global__ void
    kernel_bspline_interpolate_vf (
        float* vf,
        int3 fdim,
        int3 rdim,
        int3 cdim,
        int3 vpr
    );

    __global__ void
    kernel_sum_reduction_pt1 (
        float *idata, 
        float *odata, 
        int num_elems
    );

    __global__ void
    kernel_sum_reduction_pt2 (
        float *idata,
        float *odata,
        int num_elems
    );

    __global__ void
    kernel_bspline_mi_hist_fix (
        float* f_hist_seg,  // partial histogram (fixed image)
        float* f_img,       // moving image voxels
        float offset,       // histogram offset
        float delta,        // histogram delta
        long bins,          // # histogram bins
        int3 vpr,           // voxels per region
        int3 fdim,          // fixed  image dimensions
        int3 mdim,          // moving image dimensions
        int3 rdim,          //       region dimensions
        int3 cdim,          // # control points in x,y,z
        float3 img_origin,  // image origin
        float3 img_spacing, // image spacing
        float3 mov_origin,  // moving image offset
        float3 mov_ps       // moving image pixel spacing
    );

    __global__ void
    kernel_bspline_mi_hist_mov (
        float* m_hist_seg,  // partial histogram (moving image)
        float* m_img,       // moving image voxels
        float offset,       // histogram offset
        float delta,        // histogram delta
        long bins,          // # histogram bins
        int3 vpr,           // voxels per region
        int3 fdim,          // fixed  image dimensions
        int3 mdim,          // moving image dimensions
        int3 rdim,          //       region dimensions
        int3 cdim,          // # control points in x,y,z
        float3 img_origin,  // image origin
        float3 img_spacing, // image spacing
        float3 mov_origin,  // moving image offset
        float3 mov_ps       // moving image pixel spacing
    );

    __global__ void
    kernel_bspline_mi_hist_jnt (
        unsigned int* skipped,  // OUTPUT:   # of skipped voxels
        float* j_hist,          // OUTPUT:  joint histogram
        float* f_img,           // INPUT:  fixed image voxels
        float* m_img,           // INPUT: moving image voxels
        float f_offset,         // INPUT:  fixed histogram offset 
        float m_offset,         // INPUT: moving histogram offset
        float f_delta,          // INPUT:  fixed histogram delta
        float m_delta,          // INPUT: moving histogram delta
        long f_bins,            // INPUT: #  fixed histogram bins
        long m_bins,            // INPUT: # moving histogram bins
        int3 vpr,               // INPUT: voxels per region
        int3 fdim,              // INPUT:  fixed image dimensions
        int3 mdim,              // INPUT: moving image dimensions
        int3 rdim,              // INPUT: region dimensions
        int3 cdim,              // INPUT: # control points in x,y,z
        float3 img_origin,      // INPUT: image origin
        float3 img_spacing,     // INPUT: image spacing
        float3 mov_origin,      // INPUT: moving image offset
        float3 mov_ps,          // INPUT: moving image pixel spacing
        int3 roi_dim,           // INPUT: ROI dimensions
        int3 roi_offset         // INPUT: ROI Offset
    );

    __global__ void
    kernel_bspline_mi_hist_merge (
        float *f_hist,
        float *f_hist_seg,
        long num_seg_hist
    );

    __global__ void kernel_bspline_mi_dc_dv (
        float* dc_dv_x,     // OUTPUT: dC / dv (x-component)
        float* dc_dv_y,     // OUTPUT: dC / dv (y-component)
        float* dc_dv_z,     // OUTPUT: dC / dv (z-component)
        float* f_hist,      // INPUT:  fixed histogram
        float* m_hist,      // INPUT: moving histogram
        float* j_hist,      // INPUT:  joint histogram
        float* f_img,       // INPUT:  fixed image voxels
        float* m_img,       // INPUT: moving image voxels
        float f_offset,     // INPUT:  fixed histogram offset 
        float m_offset,     // INPUT: moving histogram offset
        float f_delta,      // INPUT:  fixed histogram delta
        float m_delta,      // INPUT: moving histogram delta
        long f_bins,        // INPUT: #  fixed histogram bins
        long m_bins,        // INPUT: # moving histogram bins
        int3 vpr,           // INPUT: voxels per region
        int3 fdim,          // INPUT:  fixed image dimensions
        int3 mdim,          // INPUT: moving image dimensions
        int3 rdim,          // INPUT: region dimensions
        int3 cdim,          // INPUT: # control points in x,y,z
        float3 img_origin,  // INPUT: image origin
        float3 img_spacing, // INPUT: image spacing
        float3 mov_origin,  // INPUT: moving image offset
        float3 mov_ps,      // INPUT: moving image pixel spacing
        int3 roi_dim,       // INPUT: ROI dimensions
        int3 roi_offset,    // INPUT: ROI Offset
        float num_vox_f,    // INPUT: # of voxels
        float score,        // INPUT: evaluated MI cost function
        int pad             // INPUT: Tile Paddign
    );

    __global__ void
    kernel_bspline_mse_score_dc_dv (
        float* score,       // OUTPUT
        float* skipped,     // OUTPUT
        float* dc_dv_x,     // OUTPUT
        float* dc_dv_y,     // OUTPUT
        float* dc_dv_z,     // OUTPUT
        float* f_img,       // fixed image voxels
        float* m_img,       // moving image voxels
        float* m_grad,      // moving image gradient
        int3 fdim,          // fixed  image dimensions
        int3 mdim,          // moving image dimensions
        int3 rdim,          //       region dimensions
        int3 cdim,          // # control points in x,y,z
        int3 vpr,           // voxels per region
        float3 img_origin,  // image origin
        float3 img_spacing, // image spacing
        float3 mov_origin,  // moving image offset
        float3 mov_ps,      // moving image pixel spacing
        int pad);           // tile padding

    __device__ inline void
    clamp_linear_interpolate_3d (
        float3* n,
        int3* n_f,
        int3* n_r,
        float3* li_1,
        float3* li_2,
        int3 mdim
    );

    __device__ inline int
    find_correspondence (
       float3 *d,
       float3 *m,
       float3 *n,
       float3 f,
       float3 mov_origin,
       float3 mov_ps,
       int3 mdim,
       int3 cdim,
       int3 vpr,
       int4 p,
       int4 q
    );

    __device__ inline float
    get_moving_value (
        int3 n_f,
        int3 mdim,
        float3 li_1,
        float3 li_2
    );

    __device__ inline void
    setup_indices (
        int4 *p,
        int4 *q,
        float3 *f,
        int fv,
        int3 fdim,
        int3 vpr,
        int3 rdim,
        float3 img_origin,
        float3 img_spacing
    );

    __device__ inline void
    write_dc_dv (
        float* dc_dv_x,
        float* dc_dv_y,
        float* dc_dv_z,
        float* m_grad,
        float diff,
        int3 n_r,
        int3 mdim,
        int3 vpr,
        int pad,
        int4 p,
        int4 q
    );

    __device__ inline void
    get_nearest_neighbors (
        int* nn,
        int3 n_f,
        int3 mdim
    );

    __device__ inline void
    get_weights (
        float* w,
        float3 li_1,
        float3 li_2
    );

    __device__ inline void
    get_weight_derivatives (
        float3* dw,
        float3 li_1,
        float3 li_2
    );

    __device__ inline void
    bspline_interpolate (
        float3* d,
        int3 cdim,
        int3 vpr,
        int4 p,
        int4 q
    );


#endif
