/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#if defined (_WIN32)
#include <windows.h>
#endif

#include "bspline.h"
#include "bspline_cuda.h"
#include "bspline_cuda_kernels.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "cuda_util.h"
#include "cuda_mem.h"
#include "cuda_kernel_util.h"
#include "joint_histogram.h"
#include "plm_int.h"
#include "volume.h"

// For CUDA Toolkits < 4.0
#ifndef cudaTextureType1D
    #define cudaTextureType1D 0x01
#endif

// Define file-scope textures
texture<float, cudaTextureType1D, cudaReadModeElementType> tex_moving_image;
texture<float, cudaTextureType1D, cudaReadModeElementType> tex_coeff;
texture<float, cudaTextureType1D, cudaReadModeElementType> tex_LUT_Bspline_x;
texture<float, cudaTextureType1D, cudaReadModeElementType> tex_LUT_Bspline_y;
texture<float, cudaTextureType1D, cudaReadModeElementType> tex_LUT_Bspline_z;
////////////////////////////////////////////////////////////


// Constructs the GPU Bspline Data structure
void
build_gbd (
    GPU_Bspline_Data* gbd,
    Bspline_xform* bxf,
    Volume* fixed,
    Volume* moving)
{
    if (bxf != NULL) {
        // populate bxf entries
        CUDA_array2vec_int3 (&gbd->rdims, bxf->rdims);
        CUDA_array2vec_int3 (&gbd->cdims, bxf->cdims);
        CUDA_array2vec_float3 (&gbd->img_origin, bxf->img_origin);
        CUDA_array2vec_float3 (&gbd->img_spacing, bxf->img_spacing);
        CUDA_array2vec_int3 (&gbd->roi_dim, bxf->roi_dim);
        CUDA_array2vec_int3 (&gbd->roi_offset, bxf->roi_offset);
        CUDA_array2vec_int3 (&gbd->vox_per_rgn, bxf->vox_per_rgn);
    }

    if (fixed != NULL) {
        // populate fixed volume entries
        CUDA_array2vec_int3 (&gbd->fix_dim, fixed->dim);
    }

    if (moving != NULL) {
        // populate moving volume entries
        CUDA_array2vec_int3 (&gbd->mov_dim, moving->dim);
        CUDA_array2vec_float3 (&gbd->mov_origin, moving->origin);
        CUDA_array2vec_float3 (&gbd->mov_spacing, moving->spacing);
    }
    
}

void
CUDA_bspline_mi_init_a (
    Bspline_xform* bxf,
    Bspline_state* bst,
    Dev_Pointers_Bspline* dev_ptrs,
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad
)
{
    Joint_histogram* mi_hist = bst->get_mi_hist();

    // Keep track of how much memory we allocated in the GPU global memory.
    long unsigned GPU_Memory_Bytes = 0;

    printf ("Allocating GPU Memory\n");

    // Fixed Image
    // ----------------------------------------------------------
    dev_ptrs->fixed_image_size = fixed->npix * fixed->pix_size;
    CUDA_alloc_copy ((void **)&dev_ptrs->fixed_image,
                     (void **)&fixed->img,
                     dev_ptrs->fixed_image_size);
    GPU_Memory_Bytes += dev_ptrs->fixed_image_size;
    printf(".");
    // ----------------------------------------------------------

    // Moving Image
    // ----------------------------------------------------------
    dev_ptrs->moving_image_size = moving->npix * moving->pix_size;
    CUDA_alloc_copy ((void **)&dev_ptrs->moving_image,
                     (void **)&moving->img,
                     dev_ptrs->moving_image_size);

    GPU_Memory_Bytes += dev_ptrs->moving_image_size;
    printf(".");
    // ----------------------------------------------------------


    // Skipped Voxels
    // ----------------------------------------------------------
    dev_ptrs->skipped_size = sizeof(unsigned int);
    CUDA_alloc_zero ((void**)&dev_ptrs->skipped_atomic,
                    dev_ptrs->skipped_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->skipped_size;
    printf(".");
    // ----------------------------------------------------------

    // Histograms
    // ----------------------------------------------------------
    dev_ptrs->f_hist_size = mi_hist->fixed.bins * sizeof(float);
    dev_ptrs->m_hist_size = mi_hist->moving.bins * sizeof(float);
    dev_ptrs->j_hist_size = mi_hist->fixed.bins * mi_hist->moving.bins * sizeof(float);
    cudaMalloc ((void**)&dev_ptrs->f_hist, dev_ptrs->f_hist_size);
    cudaMalloc ((void**)&dev_ptrs->m_hist, dev_ptrs->m_hist_size);
    cudaMalloc ((void**)&dev_ptrs->j_hist, dev_ptrs->j_hist_size);

    GPU_Memory_Bytes += dev_ptrs->f_hist_size;
    GPU_Memory_Bytes += dev_ptrs->m_hist_size;
    GPU_Memory_Bytes += dev_ptrs->j_hist_size;
    printf("...");
    // ----------------------------------------------------------


    // Coefficient LUT
    // ----------------------------------------------------------
    dev_ptrs->coeff_size = sizeof(float) * bxf->num_coeff;
    CUDA_alloc_zero ((void **)&dev_ptrs->coeff,
                    dev_ptrs->coeff_size,
                    cudaAllocStern);

    cudaBindTexture(0, tex_coeff,
                    dev_ptrs->coeff,
                    dev_ptrs->coeff_size);

    CUDA_check_error("Failed to bind dev_ptrs->coeff to texture reference!");
    GPU_Memory_Bytes += dev_ptrs->coeff_size;
    printf(".");
    // ----------------------------------------------------------


    // Score
    // ----------------------------------------------------------
    dev_ptrs->score_size = sizeof(float) * fixed->npix;
    CUDA_alloc_zero ((void **)&dev_ptrs->score,
                    dev_ptrs->score_size,
                    cudaAllocStern);

    GPU_Memory_Bytes += dev_ptrs->score_size;
    printf(".");
    // ----------------------------------------------------------
    

    // Gradient (dC_cP)
    // ----------------------------------------------------------
    dev_ptrs->grad_size = sizeof(float) * bxf->num_coeff;
    CUDA_alloc_zero ((void **)&dev_ptrs->grad,
                    dev_ptrs->grad_size,
                    cudaAllocStern);

    GPU_Memory_Bytes += dev_ptrs->grad_size;
    printf(".");
    // ----------------------------------------------------------



    // dc_dv_x,  dc_dv_y,  and  dc_dv_z
    // ----------------------------------------------------------
    int3 vol_dim;
    vol_dim.x = fixed->dim[0];
    vol_dim.y = fixed->dim[1];
    vol_dim.z = fixed->dim[2];

    int3 tile_dim;
    tile_dim.x = bxf->vox_per_rgn[0];
    tile_dim.y = bxf->vox_per_rgn[1];
    tile_dim.z = bxf->vox_per_rgn[2];

    int4 num_tile;
    num_tile.x = (vol_dim.x+tile_dim.x-1) / tile_dim.x;
    num_tile.y = (vol_dim.y+tile_dim.y-1) / tile_dim.y;
    num_tile.z = (vol_dim.z+tile_dim.z-1) / tile_dim.z;
    num_tile.w = num_tile.x * num_tile.y * num_tile.z;

    int tile_padding = 64 - ((tile_dim.x * tile_dim.y * tile_dim.z) % 64);
    int tile_bytes = (tile_dim.x * tile_dim.y * tile_dim.z);

    dev_ptrs->dc_dv_x_size = ((tile_bytes + tile_padding) * num_tile.w) * sizeof(float);
    dev_ptrs->dc_dv_y_size = dev_ptrs->dc_dv_x_size;
    dev_ptrs->dc_dv_z_size = dev_ptrs->dc_dv_x_size;

    CUDA_alloc_zero ((void **)&dev_ptrs->dc_dv_x,
                    dev_ptrs->dc_dv_x_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->dc_dv_x_size;
    printf(".");

    CUDA_alloc_zero ((void **)&dev_ptrs->dc_dv_y,
                    dev_ptrs->dc_dv_y_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->dc_dv_y_size;
    printf(".");

    CUDA_alloc_zero ((void **)&dev_ptrs->dc_dv_z,
                    dev_ptrs->dc_dv_z_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->dc_dv_z_size;
    printf(".");
    // ----------------------------------------------------------


    // Condensed dc_dv vectors
    // ----------------------------------------------------------
    dev_ptrs->cond_x_size = 64*bxf->num_knots*sizeof(float);
    CUDA_alloc_zero ((void **)&dev_ptrs->cond_x,
                    dev_ptrs->cond_x_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->cond_x_size;
    printf(".");

    dev_ptrs->cond_y_size = 64*bxf->num_knots*sizeof(float);
    CUDA_alloc_zero ((void **)&dev_ptrs->cond_y,
                    dev_ptrs->cond_y_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->cond_y_size;
    printf(".");

    dev_ptrs->cond_z_size = 64*bxf->num_knots*sizeof(float);
    CUDA_alloc_zero ((void **)&dev_ptrs->cond_z,
                    dev_ptrs->cond_z_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->cond_z_size;
    printf(".");
    // ----------------------------------------------------------


    // Tile Offset LUT
    // ----------------------------------------------------------
    int* offsets = CPU_calc_offsets (bxf->vox_per_rgn, bxf->cdims);
    int num_tiles = (bxf->cdims[0]-3) * (bxf->cdims[1]-3) * (bxf->cdims[2]-3);

    dev_ptrs->LUT_Offsets_size = num_tiles*sizeof(int);

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Offsets,
                    (void **)&offsets,
                    dev_ptrs->LUT_Offsets_size);

    GPU_Memory_Bytes += dev_ptrs->LUT_Offsets_size;
    printf(".");

    free (offsets);
    // ----------------------------------------------------------


    // Control Point (Knot) LUT
    // ----------------------------------------------------------
    dev_ptrs->LUT_Knot_size = 64*num_tiles*sizeof(int);

    int* local_set_of_64 = (int*)malloc(64*sizeof(int));
    int* LUT_Knot = (int*)malloc(dev_ptrs->LUT_Knot_size);

    int i,j;
    for (i = 0; i < num_tiles; i++)
    {
        CPU_find_knots(local_set_of_64, i, bxf->rdims, bxf->cdims);
        for (j = 0; j < 64; j++) {
            LUT_Knot[64*i + j] = local_set_of_64[j];
        }
    }

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Knot,
                    (void **)&LUT_Knot,
                    dev_ptrs->LUT_Knot_size);

    free (local_set_of_64);
    free (LUT_Knot);

    GPU_Memory_Bytes += dev_ptrs->LUT_Knot_size;
    printf (".");
    // ----------------------------------------------------------



    // B-spline LUT
    // ----------------------------------------------------------
    dev_ptrs->LUT_Bspline_x_size = 4*bxf->vox_per_rgn[0]* sizeof(float);
    dev_ptrs->LUT_Bspline_y_size = 4*bxf->vox_per_rgn[1]* sizeof(float);
    dev_ptrs->LUT_Bspline_z_size = 4*bxf->vox_per_rgn[2]* sizeof(float);
    float* LUT_Bspline_x = (float*)malloc(dev_ptrs->LUT_Bspline_x_size);
    float* LUT_Bspline_y = (float*)malloc(dev_ptrs->LUT_Bspline_y_size);
    float* LUT_Bspline_z = (float*)malloc(dev_ptrs->LUT_Bspline_z_size);

    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < bxf->vox_per_rgn[0]; i++) {
            LUT_Bspline_x[j*bxf->vox_per_rgn[0] + i] = CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[0]);
        }

        for (i = 0; i < bxf->vox_per_rgn[1]; i++) {
            LUT_Bspline_y[j*bxf->vox_per_rgn[1] + i] = CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[1]);
        }

        for (i = 0; i < bxf->vox_per_rgn[2]; i++) {
            LUT_Bspline_z[j*bxf->vox_per_rgn[2] + i] = CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[2]);
        }
    }

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Bspline_x,
                     (void **)&LUT_Bspline_x,
                     dev_ptrs->LUT_Bspline_x_size);

    cudaBindTexture(0, tex_LUT_Bspline_x, dev_ptrs->LUT_Bspline_x, dev_ptrs->LUT_Bspline_x_size);
    GPU_Memory_Bytes += dev_ptrs->LUT_Bspline_x_size;
    printf(".");


    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Bspline_y,
                     (void **)&LUT_Bspline_y,
                     dev_ptrs->LUT_Bspline_y_size);

    cudaBindTexture(0, tex_LUT_Bspline_y, dev_ptrs->LUT_Bspline_y, dev_ptrs->LUT_Bspline_y_size);
    GPU_Memory_Bytes += dev_ptrs->LUT_Bspline_y_size;
    printf(".");

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Bspline_z,
                     (void **)&LUT_Bspline_z,
                     dev_ptrs->LUT_Bspline_z_size);

    cudaBindTexture(0, tex_LUT_Bspline_z, dev_ptrs->LUT_Bspline_z, dev_ptrs->LUT_Bspline_z_size);
    GPU_Memory_Bytes += dev_ptrs->LUT_Bspline_z_size;
    printf(".");


    free (LUT_Bspline_x);
    free (LUT_Bspline_y);
    free (LUT_Bspline_z);
    // ----------------------------------------------------------

    // Inform user we are finished.
    printf (" done.\n");

    // Report global memory allocation.
    printf("             GPU Memory: %ld MB\n", GPU_Memory_Bytes / 1048576);

#if defined (commentout)
    printf ("---------------------------\n");
    printf ("Skipped Voxels: %i MB\n", dev_ptrs->skipped_size / 1048576);
    printf ("         Score: %i MB\n", dev_ptrs->score_size / 1048576);
    printf ("       dc_dv_x: %i MB\n", dev_ptrs->dc_dv_x_size / 1048576);
    printf ("       dc_dv_y: %i MB\n", dev_ptrs->dc_dv_y_size / 1048576);
    printf ("       dc_dv_z: %i MB\n", dev_ptrs->dc_dv_z_size / 1048576);
    printf ("        cond_x: %i MB\n", dev_ptrs->cond_x_size / 1048576);
    printf ("        cond_y: %i MB\n", dev_ptrs->cond_y_size / 1048576);
    printf ("        cond_z: %i MB\n", dev_ptrs->cond_z_size / 1048576);
    printf ("    Fixed Hist: %i KB\n", dev_ptrs->f_hist_size / 1024);
    printf ("   Moving Hist: %i KB\n", dev_ptrs->m_hist_size / 1024);
    printf ("    Joint Hist: %i KB\n", dev_ptrs->j_hist_size / 1024);
    printf ("         q-lut: %i KB\n", dev_ptrs->q_lut_size / 1024);
    printf ("         c-lut: %i KB\n", dev_ptrs->c_lut_size / 1024);
    printf ("     coeff-lut: %i KB\n", dev_ptrs->coeff_size / 1024);
    printf ("      Gradient: %i KB\n", dev_ptrs->grad_size / 1024);
    printf ("  Tile Offsets: %i KB\n", dev_ptrs->LUT_Offsets_size / 1024);
    printf ("      Knot LUT: %i KB\n", dev_ptrs->LUT_Knot_size / 1024);
    printf ("B-spline LUT-x: %i KB\n", dev_ptrs->LUT_Bspline_x_size / 1024);
    printf ("B-spline LUT-y: %i KB\n", dev_ptrs->LUT_Bspline_y_size / 1024);
    printf ("B-spline LUT-z: %i KB\n", dev_ptrs->LUT_Bspline_z_size / 1024);
    printf ("---------------------------\n");
#endif
}


// Initialize the GPU to execute CUDA MSE flavor 'j'
// Updated to use zero copy when enabled and available
//
// AUTHOR: James Shackleford
// DATE  : October 26, 2010
void
CUDA_bspline_mse_init_j (
    Dev_Pointers_Bspline* dev_ptrs,
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad,
    Bspline_xform* bxf,
    Bspline_parms* parms
)
{
    // Keep track of how much memory we allocated
    // in the GPU global memory.
    long unsigned GPU_Memory_Bytes = 0;

    printf ("Allocating GPU Memory");
    fflush (stdout);
    
    // Fixed Image (zero copy if possible)
    // ----------------------------------------------------------
    dev_ptrs->fixed_image_size = fixed->npix * fixed->pix_size;
    CUDA_alloc_copy ((void **)&dev_ptrs->fixed_image,
                     (void **)&fixed->img,
                     dev_ptrs->fixed_image_size);
    GPU_Memory_Bytes += dev_ptrs->fixed_image_size;
    printf(".");
    // ----------------------------------------------------------


    // Moving Image (must be global)
    // ----------------------------------------------------------
    dev_ptrs->moving_image_size = moving->npix * moving->pix_size;
    CUDA_alloc_copy ((void **)&dev_ptrs->moving_image,
                     (void **)&moving->img,
                     dev_ptrs->moving_image_size);

    cudaBindTexture(0, tex_moving_image,
                    dev_ptrs->moving_image,
                    dev_ptrs->moving_image_size);

    CUDA_check_error("Failed to bind dev_ptrs->moving_image to texture reference!");
    GPU_Memory_Bytes += dev_ptrs->moving_image_size;
    printf(".");
    // ----------------------------------------------------------


    // Moving Image Gradient
    // ----------------------------------------------------------
    dev_ptrs->moving_grad_size = moving_grad->npix * moving_grad->pix_size;
    CUDA_alloc_copy ((void **)&dev_ptrs->moving_grad,
                     (void **)&moving_grad->img,
                     dev_ptrs->moving_grad_size);
    GPU_Memory_Bytes += dev_ptrs->moving_grad_size;
    printf(".");
    // ----------------------------------------------------------


    // Coefficient LUT
    // ----------------------------------------------------------
    dev_ptrs->coeff_size = sizeof(float) * bxf->num_coeff;
    CUDA_alloc_zero ((void **)&dev_ptrs->coeff,
                     dev_ptrs->coeff_size,
                     cudaAllocStern);

    cudaBindTexture(0, tex_coeff,
                    dev_ptrs->coeff,
                    dev_ptrs->coeff_size);

    CUDA_check_error("Failed to bind dev_ptrs->coeff to texture reference!");
    GPU_Memory_Bytes += dev_ptrs->coeff_size;
    printf(".");
    // ----------------------------------------------------------



    // Score
    // ----------------------------------------------------------
    dev_ptrs->score_size = sizeof(float) * fixed->npix;
    CUDA_alloc_zero ((void **)&dev_ptrs->score,
                     dev_ptrs->score_size,
                     cudaAllocStern);

    GPU_Memory_Bytes += dev_ptrs->score_size;
    printf(".");
    // ----------------------------------------------------------



    // Skipped Voxels
    // ----------------------------------------------------------
    dev_ptrs->skipped_size = sizeof(float) * fixed->npix;
    CUDA_alloc_zero ((void **)&dev_ptrs->skipped,
                     dev_ptrs->skipped_size,
                     cudaAllocStern);

    GPU_Memory_Bytes += dev_ptrs->skipped_size;
    printf(".");
    // ----------------------------------------------------------



    // Gradient (dC_cP)
    // ----------------------------------------------------------
    dev_ptrs->grad_size = sizeof(float) * bxf->num_coeff;
    CUDA_alloc_zero ((void **)&dev_ptrs->grad,
                     dev_ptrs->grad_size,
                     cudaAllocStern);

    CUDA_check_error("Failed to bind dev_ptrs->grad to texture reference!");
    GPU_Memory_Bytes += dev_ptrs->grad_size;
    printf(".");
    // ----------------------------------------------------------


    // dc_dv_x,  dc_dv_y,  and  dc_dv_z
    // ----------------------------------------------------------
    int3 vol_dim;
    vol_dim.x = fixed->dim[0];
    vol_dim.y = fixed->dim[1];
    vol_dim.z = fixed->dim[2];

    int3 tile_dim;
    tile_dim.x = bxf->vox_per_rgn[0];
    tile_dim.y = bxf->vox_per_rgn[1];
    tile_dim.z = bxf->vox_per_rgn[2];

    int4 num_tile;
    num_tile.x = (vol_dim.x+tile_dim.x-1) / tile_dim.x;
    num_tile.y = (vol_dim.y+tile_dim.y-1) / tile_dim.y;
    num_tile.z = (vol_dim.z+tile_dim.z-1) / tile_dim.z;
    num_tile.w = num_tile.x * num_tile.y * num_tile.z;

    int tile_padding = 64 - ((tile_dim.x * tile_dim.y * tile_dim.z) % 64);
    int tile_bytes = (tile_dim.x * tile_dim.y * tile_dim.z);

    dev_ptrs->dc_dv_x_size = ((tile_bytes + tile_padding) * num_tile.w) * sizeof(float);
    dev_ptrs->dc_dv_y_size = dev_ptrs->dc_dv_x_size;
    dev_ptrs->dc_dv_z_size = dev_ptrs->dc_dv_x_size;


    CUDA_alloc_zero ((void **)&dev_ptrs->dc_dv_x,
                    dev_ptrs->dc_dv_x_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->dc_dv_x_size;
    printf(".");

    CUDA_alloc_zero ((void **)&dev_ptrs->dc_dv_y,
                    dev_ptrs->dc_dv_y_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->dc_dv_y_size;
    printf(".");

    CUDA_alloc_zero ((void **)&dev_ptrs->dc_dv_z,
                    dev_ptrs->dc_dv_z_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->dc_dv_z_size;
    printf(".");
    // ----------------------------------------------------------


    // Tile Offset LUT
    // ----------------------------------------------------------
    int* offsets = CPU_calc_offsets(bxf->vox_per_rgn, bxf->cdims);
    int num_tiles = (bxf->cdims[0]-3) * (bxf->cdims[1]-3) * (bxf->cdims[2]-3);

    dev_ptrs->LUT_Offsets_size = num_tiles*sizeof(int);

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Offsets,
                    (void **)&offsets,
                    dev_ptrs->LUT_Offsets_size);
    GPU_Memory_Bytes += dev_ptrs->LUT_Offsets_size;
    printf(".");

    free (offsets);
    // ----------------------------------------------------------


    // Control Point (Knot) LUT
    // ----------------------------------------------------------
    dev_ptrs->LUT_Knot_size = 64*num_tiles*sizeof(int);

    int* local_set_of_64 = (int*)malloc(64*sizeof(int));
    int* LUT_Knot = (int*)malloc(dev_ptrs->LUT_Knot_size);

    int i,j;
    for (i = 0; i < num_tiles; i++)
    {
        CPU_find_knots(local_set_of_64, i, bxf->rdims, bxf->cdims);
        for (j = 0; j < 64; j++) {
            LUT_Knot[64*i + j] = local_set_of_64[j];
        }
    }

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Knot,
                    (void **)&LUT_Knot,
                    dev_ptrs->LUT_Knot_size);

    free (local_set_of_64);
    free (LUT_Knot);

    GPU_Memory_Bytes += dev_ptrs->LUT_Knot_size;
    printf (".");
    // ----------------------------------------------------------



    // Condensed dc_dv vectors
    // ----------------------------------------------------------
    dev_ptrs->cond_x_size = 64*bxf->num_knots*sizeof(float);
    CUDA_alloc_zero ((void **)&dev_ptrs->cond_x,
                    dev_ptrs->cond_x_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->cond_x_size;
    printf(".");

    dev_ptrs->cond_y_size = 64*bxf->num_knots*sizeof(float);
    CUDA_alloc_zero ((void **)&dev_ptrs->cond_y,
                    dev_ptrs->cond_y_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->cond_y_size;
    printf(".");

    dev_ptrs->cond_z_size = 64*bxf->num_knots*sizeof(float);
    CUDA_alloc_zero ((void **)&dev_ptrs->cond_z,
                    dev_ptrs->cond_z_size,
                    cudaAllocStern);
    GPU_Memory_Bytes += dev_ptrs->cond_z_size;
    printf(".");
    // ----------------------------------------------------------


    // B-spline LUT
    // ----------------------------------------------------------
    dev_ptrs->LUT_Bspline_x_size = 4*bxf->vox_per_rgn[0]* sizeof(float);
    dev_ptrs->LUT_Bspline_y_size = 4*bxf->vox_per_rgn[1]* sizeof(float);
    dev_ptrs->LUT_Bspline_z_size = 4*bxf->vox_per_rgn[2]* sizeof(float);
    float* LUT_Bspline_x = (float*)malloc(dev_ptrs->LUT_Bspline_x_size);
    float* LUT_Bspline_y = (float*)malloc(dev_ptrs->LUT_Bspline_y_size);
    float* LUT_Bspline_z = (float*)malloc(dev_ptrs->LUT_Bspline_z_size);

    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < bxf->vox_per_rgn[0]; i++) {
            LUT_Bspline_x[j*bxf->vox_per_rgn[0] + i] = CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[0]);
        }

        for (i = 0; i < bxf->vox_per_rgn[1]; i++) {
            LUT_Bspline_y[j*bxf->vox_per_rgn[1] + i] = CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[1]);
        }

        for (i = 0; i < bxf->vox_per_rgn[2]; i++) {
            LUT_Bspline_z[j*bxf->vox_per_rgn[2] + i] = CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[2]);
        }
    }

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Bspline_x,
                    (void **)&LUT_Bspline_x,
                    dev_ptrs->LUT_Bspline_x_size);

    cudaBindTexture(0, tex_LUT_Bspline_x, dev_ptrs->LUT_Bspline_x, dev_ptrs->LUT_Bspline_x_size);
    GPU_Memory_Bytes += dev_ptrs->LUT_Bspline_x_size;
    printf(".");


    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Bspline_y,
                    (void **)&LUT_Bspline_y,
                    dev_ptrs->LUT_Bspline_y_size);

    cudaBindTexture(0, tex_LUT_Bspline_y, dev_ptrs->LUT_Bspline_y, dev_ptrs->LUT_Bspline_y_size);
    GPU_Memory_Bytes += dev_ptrs->LUT_Bspline_y_size;
    printf(".");

    CUDA_alloc_copy ((void **)&dev_ptrs->LUT_Bspline_z,
                    (void **)&LUT_Bspline_z,
                    dev_ptrs->LUT_Bspline_z_size);

    cudaBindTexture(0, tex_LUT_Bspline_z, dev_ptrs->LUT_Bspline_z, dev_ptrs->LUT_Bspline_z_size);
    GPU_Memory_Bytes += dev_ptrs->LUT_Bspline_z_size;
    printf(".");


    free (LUT_Bspline_x);
    free (LUT_Bspline_y);
    free (LUT_Bspline_z);
    // ----------------------------------------------------------

    // Inform user we are finished.
    printf("done.\n");

    // Report global memory allocation.
    printf("             GPU Memory: %ld MB\n", GPU_Memory_Bytes / 1048576);

}


// AUTHOR: James Shackleford
// DATE  : September 11th, 2009
void
CUDA_bspline_mse_cleanup_j (
    Dev_Pointers_Bspline* dev_ptrs,
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad
)
{
    // Textures
    cudaUnbindTexture(tex_moving_image);
    cudaUnbindTexture(tex_coeff);
    cudaUnbindTexture(tex_LUT_Bspline_x);
    cudaUnbindTexture(tex_LUT_Bspline_y);
    cudaUnbindTexture(tex_LUT_Bspline_z);

    // Global Memory
    cudaFree(dev_ptrs->fixed_image);
    cudaFree(dev_ptrs->moving_image);
    cudaFree(dev_ptrs->moving_grad);
    cudaFree(dev_ptrs->coeff);
    cudaFree(dev_ptrs->score);
    cudaFree(dev_ptrs->grad);
    cudaFree(dev_ptrs->dc_dv_x);
    cudaFree(dev_ptrs->dc_dv_y);
    cudaFree(dev_ptrs->dc_dv_z);
    cudaFree(dev_ptrs->LUT_Offsets);
    cudaFree(dev_ptrs->LUT_Knot);
    cudaFree(dev_ptrs->cond_x);
    cudaFree(dev_ptrs->cond_y);
    cudaFree(dev_ptrs->cond_z);
    cudaFree(dev_ptrs->LUT_Bspline_x);
    cudaFree(dev_ptrs->LUT_Bspline_y);
    cudaFree(dev_ptrs->LUT_Bspline_z);
    cudaFree(dev_ptrs->skipped);

}


// AUTHOR: James Shackleford
// DATE  : October 29th, 2010
void
CUDA_bspline_mi_cleanup_a (
    Dev_Pointers_Bspline* dev_ptrs,
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad
)
{
    // Textures
    cudaUnbindTexture(tex_coeff);
    cudaUnbindTexture(tex_LUT_Bspline_x);
    cudaUnbindTexture(tex_LUT_Bspline_y);
    cudaUnbindTexture(tex_LUT_Bspline_z);

    // Global Memory
    cudaFree(dev_ptrs->fixed_image);
    cudaFree(dev_ptrs->moving_image);
    cudaFree(dev_ptrs->skipped_atomic);
    cudaFree(dev_ptrs->f_hist);
    cudaFree(dev_ptrs->m_hist);
    cudaFree(dev_ptrs->j_hist);
    cudaFree(dev_ptrs->coeff);
    cudaFree(dev_ptrs->score);
    cudaFree(dev_ptrs->grad);
    cudaFree(dev_ptrs->dc_dv_x);
    cudaFree(dev_ptrs->dc_dv_y);
    cudaFree(dev_ptrs->dc_dv_z);
    cudaFree(dev_ptrs->cond_x);
    cudaFree(dev_ptrs->cond_y);
    cudaFree(dev_ptrs->cond_z);
    cudaFree(dev_ptrs->LUT_Offsets);
    cudaFree(dev_ptrs->LUT_Knot);
    cudaFree(dev_ptrs->LUT_Bspline_x);
    cudaFree(dev_ptrs->LUT_Bspline_y);
    cudaFree(dev_ptrs->LUT_Bspline_z);
}


int
CUDA_bspline_mi_hist (
    Dev_Pointers_Bspline *dev_ptrs,
    Joint_histogram* mi_hist,
    Volume* fixed,
    Volume* moving,
    Bspline_xform* bxf)
{
    cudaMemset(dev_ptrs->skipped_atomic, 0, dev_ptrs->skipped_size);

    // Generate the fixed histogram (48 ms)
    CUDA_bspline_mi_hist_fix (dev_ptrs, mi_hist, fixed, moving, bxf);

    // Generate the moving histogram (150 ms)
    CUDA_bspline_mi_hist_mov (dev_ptrs, mi_hist, fixed, moving, bxf);

    // Generate the joint histogram (~600 ms)
    return CUDA_bspline_mi_hist_jnt (dev_ptrs, mi_hist, fixed, moving, bxf);
}

void
CUDA_bspline_mi_hist_fix (
    Dev_Pointers_Bspline *dev_ptrs,
    Joint_histogram* mi_hist,
    Volume* fixed,
    Volume* moving,
    Bspline_xform *bxf)
{
    dim3 dimGrid;
    dim3 dimBlock;
    int num_blocks;

    GPU_Bspline_Data gbd; 
    build_gbd (&gbd, bxf, fixed, moving);

    // Initialize histogram memory on GPU
    cudaMemset(dev_ptrs->f_hist, 0, dev_ptrs->f_hist_size);
    CUDA_check_error ("Failed to initialize memory for f_hist");

    num_blocks = CUDA_exec_conf_1tpe (
        &dimGrid,          // OUTPUT: Grid  dimensions
        &dimBlock,         // OUTPUT: Block dimensions
        fixed->npix,       // INPUT: Total # of threads
        32,                // INPUT: Threads per block
        false              // INPUT: Is threads per block negotiable?
    );

    int smemSize = (int) (dimBlock.x * mi_hist->fixed.bins * sizeof(float));

    dev_ptrs->f_hist_seg_size = mi_hist->fixed.bins * num_blocks * sizeof(float);
    cudaMalloc ((void**)&dev_ptrs->f_hist_seg, dev_ptrs->f_hist_seg_size);
    CUDA_check_error ("Failed to allocate memory for f_hist_seg");
    cudaMemset(dev_ptrs->f_hist_seg, 0, dev_ptrs->f_hist_seg_size);
    CUDA_check_error ("Failed to initialize memory for f_hist_seg");


    // Launch kernel with one thread per voxel
    kernel_bspline_mi_hist_fix <<<dimGrid, dimBlock, smemSize>>> (
        dev_ptrs->f_hist_seg,       // partial histogram (moving image)
        dev_ptrs->fixed_image,      // moving image voxels
        mi_hist->fixed.offset,      // histogram origin
        1.0f/mi_hist->fixed.delta,  // histogram delta
        mi_hist->fixed.bins,        // # histogram bins
        gbd.vox_per_rgn,            // voxels per region
        gbd.fix_dim,                // fixed  image dimensions
        gbd.mov_dim,                // moving image dimensions
        gbd.rdims,                  //       region dimensions
        gbd.cdims,                  // # control points in x,y,z
        gbd.img_origin,             // image origin
        gbd.img_spacing,            // image spacing
        gbd.mov_origin,             // moving image origin
        gbd.mov_spacing             // moving image pixel spacing
    );

    cudaThreadSynchronize();
    CUDA_check_error ("kernel_bspline_mi_hist_fix");

    int num_sub_hists = num_blocks;

    // Merge sub-histograms
    dim3 dimGrid2 (mi_hist->fixed.bins, 1, 1);
    dim3 dimBlock2 (512, 1, 1);
    smemSize = 512 * sizeof(float);
    
    // this kernel can be ran with any thread-block size
    kernel_bspline_mi_hist_merge <<<dimGrid2 , dimBlock2, smemSize>>> (
        dev_ptrs->f_hist,
        dev_ptrs->f_hist_seg,
        num_sub_hists
    );

    cudaThreadSynchronize();
    CUDA_check_error ("kernel hist_fix_merge");

    /* copy result back to host
     *   -- Note CPU uses doubles whereas the GPU uses floats
     *      due to lack of double precision floats.  This is okay
     *      since the GPU's ability to add small numbers to large
     *      using single precision is more accurate than the CPU.
     *   
     *   -- However, this does result in the little bit of nastiness
     *      found below.  We copy these back to the CPU for the score
     *      computation, which the CPU completes very quickly.
     */
    float* f_hist_f = (float*)malloc(dev_ptrs->f_hist_size);

    cudaMemcpy (f_hist_f, dev_ptrs->f_hist, dev_ptrs->f_hist_size, cudaMemcpyDeviceToHost);
    CUDA_check_error ("Unable to copy fixed histograms from GPU to CPU!\n");

    /* type cast to CPU friendly double */
    for (int i=0; i< mi_hist->fixed.bins; i++) {
        mi_hist->f_hist[i] = (double)f_hist_f[i];
    }

    free (f_hist_f);

    cudaFree (dev_ptrs->f_hist_seg);
    CUDA_check_error ("Error freeing sub-histograms from GPU memory!\n");

}


void
CUDA_bspline_mi_hist_mov (
    Dev_Pointers_Bspline *dev_ptrs,
    Joint_histogram* mi_hist,
    Volume* fixed,
    Volume* moving,
    Bspline_xform *bxf)
{
    dim3 dimGrid;
    dim3 dimBlock;
    int num_blocks;

    GPU_Bspline_Data gbd;
    build_gbd (&gbd, bxf, fixed, moving);

    // Initialize histogram memory on GPU
    cudaMemset(dev_ptrs->m_hist, 0, dev_ptrs->m_hist_size);
    CUDA_check_error ("Failed to initialize memory for m_hist");
    
    num_blocks = 
	CUDA_exec_conf_1tpe (
	    &dimGrid,          // OUTPUT: Grid  dimensions
	    &dimBlock,         // OUTPUT: Block dimensions
	    fixed->npix,       // INPUT: Total # of threads
	    32,                // INPUT: Threads per block
	    false);            // INPUT: Is threads per block negotiable?

    int smemSize = (int) (dimBlock.x * mi_hist->moving.bins * sizeof(float));

    dev_ptrs->m_hist_seg_size = mi_hist->moving.bins * num_blocks * sizeof(float);
    cudaMalloc ((void**)&dev_ptrs->m_hist_seg, dev_ptrs->m_hist_seg_size);
    CUDA_check_error ("Failed to allocate memory for m_hist_seg");
    cudaMemset(dev_ptrs->m_hist_seg, 0, dev_ptrs->m_hist_seg_size);
    CUDA_check_error ("Failed to initialize memory for m_hist_seg");


    // Launch kernel with one thread per voxel
    kernel_bspline_mi_hist_mov <<<dimGrid, dimBlock, smemSize>>> (
        dev_ptrs->m_hist_seg,       // partial histogram (moving image)
        dev_ptrs->moving_image,     // moving image voxels
        mi_hist->moving.offset,     // histogram origin
        1.0f/mi_hist->moving.delta, // histogram delta
        mi_hist->moving.bins,       // # histogram bins
        gbd.vox_per_rgn,            // voxels per region
        gbd.fix_dim,                // fixed  image dimensions
        gbd.mov_dim,                // moving image dimensions
        gbd.rdims,                  //       region dimensions
        gbd.cdims,                  // # control points in x,y,z
        gbd.img_origin,             // image origin
        gbd.img_spacing,            // image spacing
        gbd.mov_origin,             // moving image origin
        gbd.mov_spacing             // moving image pixel spacing
    );

    cudaThreadSynchronize();
    CUDA_check_error ("kernel hist_mov");

    int num_sub_hists = num_blocks;


    // Merge sub-histograms
    dim3 dimGrid2 (mi_hist->moving.bins, 1, 1);
    dim3 dimBlock2 (512, 1, 1);
    smemSize = 512 * sizeof(float);
    
    // this kernel can be ran with any thread-block size
    kernel_bspline_mi_hist_merge <<<dimGrid2 , dimBlock2, smemSize>>> (
        dev_ptrs->m_hist,
        dev_ptrs->m_hist_seg,
        num_sub_hists
    );

    cudaThreadSynchronize();
    CUDA_check_error ("kernel hist_merge");

    /* copy result back to host
     *   -- Note CPU uses doubles whereas the GPU uses floats
     *      due to lack of double precision floats.  This is okay
     *      since the GPU's ability to add small numbers to large
     *      using single precision is more accurate than the CPU.
     *   
     *   -- However, this does result in the little bit of nastiness
     *      found below.  We copy these back to the CPU for the score
     *      computation, which the CPU completes very quickly.
     */
    float* m_hist_f = (float*)malloc(dev_ptrs->m_hist_size);

    cudaMemcpy (m_hist_f, dev_ptrs->m_hist, dev_ptrs->m_hist_size, cudaMemcpyDeviceToHost);
    CUDA_check_error ("Unable to copy moving histograms from GPU to CPU!\n");

    /* type cast to CPU friendly double */
    for (int i=0; i< mi_hist->moving.bins; i++) {
        mi_hist->m_hist[i] = (double)m_hist_f[i];
    }

    free (m_hist_f);

    cudaFree (dev_ptrs->m_hist_seg);
    CUDA_check_error ("Error freeing sub-histograms from GPU memory!\n");

}


int
CUDA_bspline_mi_hist_jnt (
    Dev_Pointers_Bspline *dev_ptrs,
    Joint_histogram* mi_hist,
    Volume* fixed,
    Volume* moving,
    Bspline_xform *bxf)
{
    GPU_Bspline_Data gbd;
    build_gbd (&gbd, bxf, fixed, moving);

    // Initialize histogram memory on GPU
    cudaMemset(dev_ptrs->j_hist, 0, dev_ptrs->j_hist_size);


    int num_bins = (int)mi_hist->fixed.bins * (int)mi_hist->moving.bins;


    // ----------------------
    // --- INITIALIZE GRID ---
    // ----------------------
    int i;
    int Grid_x = 0;
    int Grid_y = 0;
    int threads_per_block = 128;
    int num_threads = fixed->npix;
    int sqrt_num_blocks;
    int num_blocks;
    int smemSize;
    int found_flag = 0;

    // Search for a valid execution configuration
    // for the required # of blocks.
    for (threads_per_block = 192; threads_per_block > 32; threads_per_block -= 32) {
        num_blocks = (num_threads + threads_per_block - 1) / threads_per_block;
        sqrt_num_blocks = (int)sqrt((float)num_blocks);

        for (i = sqrt_num_blocks; i < 65535; i++) {
            if (num_blocks % i == 0) {
                Grid_x = i;
                Grid_y = num_blocks / Grid_x;
                found_flag = 1;
                break;
            }
        }

        if (found_flag == 1) {
            break;
        }
    }

    // Were we able to find a valid exec config?
    if (Grid_x == 0) {
        printf("\n[ERROR] Unable to find suitable kernel_bspline_mi_hist_jnt() configuration!\n");
        exit(0);
    } else {
//        printf ("Grid [%i,%i], %d threads_per_block.\n", Grid_x, Grid_y, threads_per_block);
    }

    dim3 dimGrid1(Grid_x, Grid_y, 1);
    dim3 dimBlock1(threads_per_block, 1, 1);
    // ----------------------
    // ----------------------
    // ----------------------

    dev_ptrs->j_hist_seg_size = dev_ptrs->j_hist_size * num_blocks;

    cudaMalloc ((void**)&dev_ptrs->j_hist_seg, dev_ptrs->j_hist_seg_size);
    cudaMemset(dev_ptrs->j_hist_seg, 0, dev_ptrs->j_hist_seg_size);
    CUDA_check_error ("Failed to allocate memory for j_hist_seg");
    smemSize = (num_bins + 1) * sizeof(float);

    // Launch kernel with one thread per voxel
    kernel_bspline_mi_hist_jnt <<<dimGrid1, dimBlock1, smemSize>>> (
            dev_ptrs->skipped_atomic,   // # voxels that map outside moving
            dev_ptrs->j_hist_seg,       // partial histogram (moving image)
            dev_ptrs->fixed_image,      // fixed  image voxels
            dev_ptrs->moving_image,     // moving image voxels
            mi_hist->fixed.offset,      // fixed histogram origin
            mi_hist->moving.offset,     // moving histogram origin
            1.0f/mi_hist->fixed.delta,  // fixed histogram delta
            1.0f/mi_hist->moving.delta, // moving histogram delta
            mi_hist->fixed.bins,        // # fixed bins
            mi_hist->moving.bins,       // # moving bins
            gbd.vox_per_rgn,            // voxels per region
            gbd.fix_dim,                // fixed  image dimensions
            gbd.mov_dim,                // moving image dimensions
            gbd.rdims,                  //       region dimensions
            gbd.cdims,                  // # control points in x,y,z
            gbd.img_origin,             // image origin
            gbd.img_spacing,            // image spacing
            gbd.mov_origin,             // moving image origin
            gbd.mov_spacing,            // moving image pixel spacing
            gbd.roi_dim,                // region dims
            gbd.roi_offset              // region offset
    );

    cudaThreadSynchronize();
    CUDA_check_error ("kernel hist_jnt");

    // Merge sub-histograms
    threads_per_block = 512;
    dim3 dimGrid2 (num_bins, 1, 1);
    dim3 dimBlock2 (threads_per_block, 1, 1);
    smemSize = 512 * sizeof(float);

    // this kernel can be ran with any thread-block size
    int num_sub_hists = num_blocks;
    kernel_bspline_mi_hist_merge <<<dimGrid2 , dimBlock2, smemSize>>> (
        dev_ptrs->j_hist,
        dev_ptrs->j_hist_seg,
        num_sub_hists
    );

    cudaThreadSynchronize();
    CUDA_check_error ("kernel hist_jnt_merge");

    /* copy result back to host
     *   -- Note CPU uses doubles whereas the GPU uses floats
     *      due to lack of double precision floats.  This is okay
     *      since the GPU's ability to add small numbers to large
     *      using single precision is more accurate than the CPU.
     *   
     *   -- However, this does result in the little bit of nastiness
     *      found below.  We copy these back to the CPU for the score
     *      computation, which the CPU completes very quickly.
     */
    float* j_hist_f = (float*)malloc(dev_ptrs->j_hist_size);

    cudaMemcpy (j_hist_f, dev_ptrs->j_hist, dev_ptrs->j_hist_size, cudaMemcpyDeviceToHost);
    CUDA_check_error ("Unable to copy joint histograms from GPU to CPU!\n");

    /* type cast to CPU friendly double */
    for (int i=0; i< mi_hist->moving.bins * mi_hist->fixed.bins; i++) {
        mi_hist->j_hist[i] = (double)j_hist_f[i];
    }

    free (j_hist_f);


    cudaFree (dev_ptrs->j_hist_seg);
    CUDA_check_error ("Error freeing sub-histograms from GPU memory!");


    // Get # of skipped voxels and compute num_vox 
    unsigned int skipped;
    int num_vox;
    cudaMemcpy(&skipped, dev_ptrs->skipped_atomic, sizeof(unsigned int), cudaMemcpyDeviceToHost);
    num_vox = (gbd.fix_dim.x * gbd.fix_dim.y * gbd.fix_dim.z) - skipped;


    // Now, we back compute bin 0,0 for the joint histogram
    int j = 0;
    for (i = 1; i < mi_hist->fixed.bins * mi_hist->moving.bins; i++) {
        j += mi_hist->j_hist[i];
    }

    mi_hist->j_hist[0] = num_vox - j;

    return num_vox;

}

void
CUDA_bspline_mi_grad (
    Bspline_state *bst,
    Bspline_xform *bxf,
    Volume* fixed,
    Volume* moving,
    float num_vox_f,
    float score,
    Dev_Pointers_Bspline *dev_ptrs
)
{
    Joint_histogram* mi_hist = bst->get_mi_hist();
    GPU_Bspline_Data gbd;
    build_gbd (&gbd, bxf, fixed, moving);

    Bspline_score* ssd = &bst->ssd;
    float* host_grad = ssd->curr_smetric_grad;

    if ((mi_hist->fixed.bins > GPU_MAX_BINS) ||
        (mi_hist->moving.bins > GPU_MAX_BINS)) {

        // Initialize histogram memory on GPU
        // (only necessary if histograms are CPU generated)
        float* f_tmp = (float*)malloc(dev_ptrs->f_hist_size);
        float* m_tmp = (float*)malloc(dev_ptrs->m_hist_size);
        float* j_tmp = (float*)malloc(dev_ptrs->j_hist_size);

        for (int i=0; i<mi_hist->fixed.bins; i++) {
            f_tmp[i] = (float)mi_hist->f_hist[i];
        }

        for (int i=0; i<mi_hist->moving.bins; i++) {
            m_tmp[i] = (float)mi_hist->m_hist[i];
        }

        for (int i=0; i<mi_hist->joint.bins; i++) {
            j_tmp[i] = (float)mi_hist->j_hist[i];
        }

        cudaMemcpy (dev_ptrs->f_hist, f_tmp,
                dev_ptrs->f_hist_size, cudaMemcpyHostToDevice);
        CUDA_check_error ("Unable to copy fixed histograms from CPU to GPU!\n");

        cudaMemcpy (dev_ptrs->m_hist, m_tmp,
                dev_ptrs->m_hist_size, cudaMemcpyHostToDevice);
        CUDA_check_error ("Unable to copy moving histograms from CPU to GPU!\n");

        cudaMemcpy (dev_ptrs->j_hist, j_tmp,
                dev_ptrs->j_hist_size, cudaMemcpyHostToDevice);
        CUDA_check_error ("Unable to copy joint histograms from CPU to GPU!\n");

        free (f_tmp);
        free (m_tmp);
        free (j_tmp);
    }

    // Initial dc_dv streams
    cudaMemset(dev_ptrs->dc_dv_x, 0, dev_ptrs->dc_dv_x_size);
       CUDA_check_error("cudaMemset(): dev_ptrs->dc_dv_x");
    cudaMemset(dev_ptrs->dc_dv_y, 0, dev_ptrs->dc_dv_y_size);
       CUDA_check_error("cudaMemset(): dev_ptrs->dc_dv_y");
    cudaMemset(dev_ptrs->dc_dv_z, 0, dev_ptrs->dc_dv_z_size);
       CUDA_check_error("cudaMemset(): dev_ptrs->dc_dv_z");
    

    // --- INITIALIZE GRID ---
    int i;
    int Grid_x = 0;
    int Grid_y = 0;
    int threads_per_block = 128;
    int num_threads = fixed->npix;
    int sqrt_num_blocks;
    int num_blocks;
    int found_flag = 0;

    // Search for a valid execution configuration
    // for the required # of blocks.
    for (threads_per_block = 192; threads_per_block > 32; threads_per_block -= 32) {
    num_blocks = (num_threads + threads_per_block - 1) / threads_per_block;
    sqrt_num_blocks = (int)sqrt((float)num_blocks);

        for (i = sqrt_num_blocks; i < 65535; i++) {
            if (num_blocks % i == 0) {
                Grid_x = i;
                Grid_y = num_blocks / Grid_x;
                found_flag = 1;
                break;
            }
        }

        if (found_flag == 1) {
            break;
        }
    }

    // Were we able to find a valid exec config?
    if (Grid_x == 0) {
        // If this happens we should consider falling back to a
        // CPU implementation, using a different CUDA algorithm,
        // or padding the input dc_dv stream to work with this
        // CUDA algorithm.
        printf("\n[ERROR] Unable to find suitable kernel_bspline_mi_dc_dv() configuration!\n");
        exit(0);
    } else {
#if defined (commentout)
        printf ("Grid [%i,%i], %d threads_per_block.\n", 
            Grid_x, Grid_y, threads_per_block);
#endif
    }

    dim3 dimGrid1(Grid_x, Grid_y, 1);
    dim3 dimBlock1(threads_per_block, 1, 1);


    int tile_padding = 64 - ((gbd.vox_per_rgn.x * gbd.vox_per_rgn.y * gbd.vox_per_rgn.z) % 64);

    // Launch kernel with one thread per voxel
    kernel_bspline_mi_dc_dv <<<dimGrid1, dimBlock1>>> (
        dev_ptrs->dc_dv_x,
        dev_ptrs->dc_dv_y,
        dev_ptrs->dc_dv_z,  
        dev_ptrs->f_hist,
        dev_ptrs->m_hist,
        dev_ptrs->j_hist,
        dev_ptrs->fixed_image,
        dev_ptrs->moving_image,
        mi_hist->fixed.offset,
        mi_hist->moving.offset,
        1.0f/mi_hist->fixed.delta,
        1.0f/mi_hist->moving.delta,
        mi_hist->fixed.bins,
        mi_hist->moving.bins,
        gbd.vox_per_rgn,
        gbd.fix_dim,
        gbd.mov_dim,
        gbd.rdims,
        gbd.cdims,
        gbd.img_origin,
        gbd.img_spacing,
        gbd.mov_origin,
        gbd.mov_spacing,
        gbd.roi_dim,
        gbd.roi_offset,
        num_vox_f,
        score,
        tile_padding
    );


    ////////////////////////////////
    // Prepare for the next kernel
    cudaThreadSynchronize();
    CUDA_check_error("kernel_bspline_mi_dc_dv()");

    // Clear out the condensed dc_dv streams
    cudaMemset(dev_ptrs->cond_x, 0, dev_ptrs->cond_x_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->cond_x");
    cudaMemset(dev_ptrs->cond_y, 0, dev_ptrs->cond_y_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->cond_y");
    cudaMemset(dev_ptrs->cond_z, 0, dev_ptrs->cond_z_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->cond_z");
    
    // Invoke kernel condense
    int num_tiles = (bxf->cdims[0]-3) * (bxf->cdims[1]-3) * (bxf->cdims[2]-3);
    CUDA_bspline_condense (
        dev_ptrs,
        bxf->vox_per_rgn, 
        num_tiles
    );
    
    // Prepare for the next kernel
    cudaThreadSynchronize();
    CUDA_check_error("kernel_bspline_condense ()");

    // Clear out the gradient
    cudaMemset(dev_ptrs->grad, 0, dev_ptrs->grad_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->grad");

    // Invoke kernel reduce
    CUDA_bspline_reduce (
        dev_ptrs,
        bxf->num_knots
    );

    // Prepare for the next kernel
    cudaThreadSynchronize();
    CUDA_check_error("[Kernel Panic!] kernel_bspline_mse_condense()");

    // --- RETREIVE THE GRAD FROM GPU ---------------------------
    cudaMemcpy(host_grad, dev_ptrs->grad, sizeof(float) * bxf->num_coeff, cudaMemcpyDeviceToHost);
    CUDA_check_error("Failed to copy dev_ptrs->grad to CPU");
    CUDA_check_error("Failed to copy dev_ptrs->grad to CPU");
    // ----------------------------------------------------------
}


/**
 * Calculates the B-spline score and gradient using CUDA implementation J.
 *
 * @param fixed The fixed volume
 * @param moving The moving volume
 * @param moving_grad The spatial gradient of the moving volume
 * @param bxf Pointer to the B-spline Xform
 * @param parms Pointer to the B-spline parameters
 * @param dev_ptrs Pointer the GPU device pointers
 *
 * @see CUDA_bspline_mse_score_dc_dv()
 * @see CUDA_bspline_condense ()
 * @see CUDA_bspline_reduce()
 *
 * @author James A. Shackleford
 */
void
CUDA_bspline_mse_pt1 (
    Volume* fixed,
    Volume* moving,
    Volume* moving_grad,
    Bspline_xform* bxf,
    Bspline_parms* parms,
    Dev_Pointers_Bspline* dev_ptrs)
{
#if defined (PROFILE_MSE)
    cuda_timer my_timer;
#endif


    // Reset our "voxels fallen outside" counter
    cudaMemset (dev_ptrs->skipped, 0, dev_ptrs->skipped_size);
    CUDA_check_error ("cudaMemset(): dev_ptrs->skipped");
    cudaMemset (dev_ptrs->score, 0, dev_ptrs->score_size);
    CUDA_check_error ("cudaMemset(): dev_ptrs->score");


#if defined (PROFILE_MSE)
    CUDA_timer_start (&my_timer);
#endif

    // Calculate the score and dc_dv
    CUDA_bspline_mse_score_dc_dv (dev_ptrs, bxf, fixed, moving);


#if defined (PROFILE_MSE)
    printf("[%f ms] score & dc_dv\n", CUDA_timer_report (&my_timer));
#endif

    // Prepare for the next kernel
    cudaThreadSynchronize();
    CUDA_check_error("[Kernel Panic!] kernel_bspline_g_mse_1");

    // Clear out the condensed dc_dv streams
    cudaMemset(dev_ptrs->cond_x, 0, dev_ptrs->cond_x_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->cond_x");
    cudaMemset(dev_ptrs->cond_y, 0, dev_ptrs->cond_y_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->cond_y");
    cudaMemset(dev_ptrs->cond_z, 0, dev_ptrs->cond_z_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->cond_z");


#if defined (PROFILE_MSE)
    CUDA_timer_start (&my_timer);
#endif

    // Invoke kernel condense
    int num_tiles = (bxf->cdims[0]-3) * (bxf->cdims[1]-3) * (bxf->cdims[2]-3);
    CUDA_bspline_condense (
        dev_ptrs,
        bxf->vox_per_rgn, 
        num_tiles
    );
    cudaThreadSynchronize();
    CUDA_check_error("kernel_bspline_mse_condense()");

#if defined (PROFILE_MSE)
    printf("[%f ms] condense\n", CUDA_timer_report (&my_timer));
    CUDA_timer_start (&my_timer);
#endif

    // Clear out the gradient
    cudaMemset(dev_ptrs->grad, 0, dev_ptrs->grad_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->grad");

    // Invoke kernel reduce
    CUDA_bspline_reduce (dev_ptrs, bxf->num_knots);

#if defined (PROFILE_MSE)
    printf("[%f ms] reduce\n\n", CUDA_timer_report (&my_timer));
#endif

    // Prepare for the next kernel
    cudaThreadSynchronize();
    CUDA_check_error("[Kernel Panic!] kernel_bspline_mse_condense()");
}



////////////////////////////////////////////////////////////////////////////////
// STUB: CUDA_bspline_mse_pt2
//
// KERNELS INVOKED:
//   kernel_sum_reduction_pt1()
//   kernel_sum_reduction_pt2()
//   kernel_bspline_grad_normalize()
////////////////////////////////////////////////////////////////////////////////
void
CUDA_bspline_mse_pt2 (
    Bspline_parms* parms, 
    Bspline_xform* bxf,
    Volume* fixed,
    plm_long*   vox_per_rgn,
    plm_long*   volume_dim,
    float* host_score,
    float* host_grad,
    float* host_grad_mean,
    float* host_grad_norm,
    Dev_Pointers_Bspline* dev_ptrs,
    plm_long *num_vox)
{

#if defined (PROFILE_MSE)
    cuda_timer my_timer;
#endif


    dim3 dimGrid;
    dim3 dimBlock;

    int num_elems = volume_dim[0] * volume_dim[1] * volume_dim[2];
    int num_blocks = (num_elems + 511) / 512;

    CUDA_exec_conf_1bpe (
        &dimGrid,         // OUTPUT: Grid  dimensions
        &dimBlock,        // OUTPUT: Block dimensions
        num_blocks,       // INPUT: Number of blocks
        512);             // INPUT: Threads per block

    int smemSize = 512*sizeof(float);


#if defined (PROFILE_MSE)
    CUDA_timer_start (&my_timer);
#endif

    // --- REDUCE SCORE VECTOR DOWN TO SINGLE VALUE -------------
    kernel_sum_reduction_pt1 <<<dimGrid, dimBlock, smemSize>>> (
        dev_ptrs->score,
        dev_ptrs->score,
        num_elems
    );

    cudaThreadSynchronize();
    CUDA_check_error("kernel_sum_reduction_pt1()");

    kernel_sum_reduction_pt2 <<<dimGrid, dimBlock>>> (
        dev_ptrs->score,
        dev_ptrs->score,
        num_elems
    );

    cudaThreadSynchronize();
    CUDA_check_error("kernel_sum_reduction_pt2()");
    // ----------------------------------------------------------

#if defined (PROFILE_MSE)
    printf("[%f ms] score reduction\n", CUDA_timer_report (&my_timer));
    CUDA_timer_start (&my_timer);
#endif

    // --- RETREIVE THE SCORE FROM GPU --------------------------
    cudaMemcpy(host_score, dev_ptrs->score, sizeof(float), cudaMemcpyDeviceToHost);
    CUDA_check_error("Failed to copy score from GPU to host");
    // ----------------------------------------------------------


#if defined (PROFILE_MSE)
    printf("[%f ms] score memcpy\n", CUDA_timer_report (&my_timer));
    CUDA_timer_start (&my_timer);
#endif


    // --- REDUCE SKIPPED VECTOR DOWN TO SINGLE VALUE -----------
    kernel_sum_reduction_pt1 <<<dimGrid, dimBlock, smemSize>>> (
        dev_ptrs->skipped,
        dev_ptrs->skipped,
        num_elems
    );

    cudaThreadSynchronize();
    CUDA_check_error("[Kernel Panic!] kernel_sum_reduction_pt1()");

    kernel_sum_reduction_pt2 <<<dimGrid, dimBlock>>> (
        dev_ptrs->skipped,
        dev_ptrs->skipped,
        num_elems
    );

    cudaThreadSynchronize();
    CUDA_check_error("kernel_sum_reduction_pt2()");

    float skipped;
    cudaMemcpy (&skipped, dev_ptrs->skipped,
        sizeof(float), cudaMemcpyDeviceToHost);
    // ----------------------------------------------------------

#if defined (PROFILE_MSE)
    printf("[%f ms] skipped reduction\n", CUDA_timer_report (&my_timer));
#endif

    // --- COMPUTE # VOXELS & SCORE -----------------------------
    *num_vox = (volume_dim[0] * volume_dim[1] * volume_dim[2]) - skipped;
    *host_score = *host_score / *num_vox;
    // ----------------------------------------------------------



    // --- COMPUTE THE GRADIENT ---------------------------------
    num_elems = bxf->num_coeff;
    num_blocks = (num_elems + 511) / 512;

    CUDA_exec_conf_1bpe (
        &dimGrid,         // OUTPUT: Grid  dimensions
        &dimBlock,        // OUTPUT: Block dimensions
        num_blocks,       // INPUT: Number of blocks
        512);             // INPUT: Threads per block


#if defined (PROFILE_MSE)
    CUDA_timer_start (&my_timer);
#endif
    
    kernel_bspline_grad_normalize <<<dimGrid, dimBlock>>> (
        dev_ptrs->grad,
        *num_vox,
        num_elems
    );

    cudaThreadSynchronize();
    CUDA_check_error("kernel_bspline_grad_normalize()");

#if defined (PROFILE_MSE)
    printf("[%f ms] gradient update\n", CUDA_timer_report (&my_timer));
    CUDA_timer_start (&my_timer);
#endif

    cudaMemcpy(host_grad, dev_ptrs->grad, sizeof(float) * bxf->num_coeff, cudaMemcpyDeviceToHost);
    CUDA_check_error("Failed to copy dev_ptrs->grad to CPU");


#if defined (PROFILE_MSE)
    printf("[%f ms] gradient memcpy\n", CUDA_timer_report (&my_timer));
#endif
    // ----------------------------------------------------------

}



//////////////////////////////////////////////////////////////////////////////
// STUB: CUDA_bspline_mse_score_dc_dv()
//
// KERNELS INVOKED:
//   kernel_bspline_mse_score_dc_dv()
//
// AUTHOR: James Shackleford
//   DATE: 19 August, 2009
//////////////////////////////////////////////////////////////////////////////
void
CUDA_bspline_mse_score_dc_dv (
    Dev_Pointers_Bspline* dev_ptrs,
    Bspline_xform* bxf,
    Volume* fixed,
    Volume* moving)
{
    dim3 dimGrid1;
    dim3 dimBlock1;
    GPU_Bspline_Data gbd;   

    build_gbd (&gbd, bxf, fixed, moving);

    CUDA_exec_conf_1tpe (
        &dimGrid1,          // OUTPUT: Grid  dimensions
        &dimBlock1,         // OUTPUT: Block dimensions
        fixed->npix,        // INPUT: Total # of threads
        192,                // INPUT: Threads per block
        true);              // INPUT: Is threads per block negotiable?

#if defined (commentout)
    int smemSize = 12 * sizeof(float) * dimBlock1.x;
#endif

    // --- BEGIN KERNEL EXECUTION ---
    cudaMemset(dev_ptrs->dc_dv_x, 0, dev_ptrs->dc_dv_x_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->dc_dv_x");

    cudaMemset(dev_ptrs->dc_dv_y, 0, dev_ptrs->dc_dv_y_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->dc_dv_y");

    cudaMemset(dev_ptrs->dc_dv_z, 0, dev_ptrs->dc_dv_z_size);
    CUDA_check_error("cudaMemset(): dev_ptrs->dc_dv_z");

    int tile_padding = 64 - 
    ((gbd.vox_per_rgn.x * gbd.vox_per_rgn.y * gbd.vox_per_rgn.z) % 64);

    kernel_bspline_mse_score_dc_dv <<<dimGrid1, dimBlock1>>> (
            dev_ptrs->score,
            dev_ptrs->skipped,
            dev_ptrs->dc_dv_x,
            dev_ptrs->dc_dv_y,
            dev_ptrs->dc_dv_z,
            dev_ptrs->fixed_image,
            dev_ptrs->moving_image,
            dev_ptrs->moving_grad,
            gbd.fix_dim,
            gbd.mov_dim,
            gbd.rdims,
            gbd.cdims,
            gbd.vox_per_rgn,
            gbd.img_origin,
            gbd.img_spacing,
            gbd.mov_origin,
            gbd.mov_spacing,
            tile_padding);
}


//////////////////////////////////////////////////////////////////////////////
// STUB: CUDA_bspline_condense ()
//
// KERNELS INVOKED:
//   kernel_bspline_condense ()
//
// AUTHOR: James Shackleford
//   DATE: September 16th, 2009
//////////////////////////////////////////////////////////////////////////////
void
CUDA_bspline_condense (
    Dev_Pointers_Bspline* dev_ptrs,
    plm_long* vox_per_rgn,
    int num_tiles
)
{
    dim3 dimGrid;
    dim3 dimBlock;

    int4 vox_per_region;
    vox_per_region.x = (int) vox_per_rgn[0];
    vox_per_region.y = (int) vox_per_rgn[1];
    vox_per_region.z = (int) vox_per_rgn[2];
    vox_per_region.w = 
	(int) vox_per_region.x * vox_per_region.y * vox_per_region.z;

    int pad = 64 - (vox_per_region.w % 64);

    vox_per_region.w += pad;

    CUDA_exec_conf_1bpe (
        &dimGrid,         // OUTPUT: Grid  dimensions
        &dimBlock,        // OUTPUT: Block dimensions
        num_tiles,        // INPUT: Number of blocks
        64);              // INPUT: Threads per block

    int smemSize = 576*sizeof(float);

    kernel_bspline_condense <<<dimGrid, dimBlock, smemSize>>> (
        dev_ptrs->cond_x,       // Return: condensed dc_dv_x values
        dev_ptrs->cond_y,       // Return: condensed dc_dv_y values
        dev_ptrs->cond_z,       // Return: condensed dc_dv_z values
        dev_ptrs->dc_dv_x,      // Input : dc_dv_x values
        dev_ptrs->dc_dv_y,      // Input : dc_dv_y values
        dev_ptrs->dc_dv_z,      // Input : dc_dv_z values
        dev_ptrs->LUT_Offsets,  // Input : tile offsets
        dev_ptrs->LUT_Knot,     // Input : linear knot indicies
        pad,                    // Input : amount of tile padding
        vox_per_region,         // Input : dims of tiles
        (float)1/6);            // Input : GPU Division is slow
}
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// STUB: CUDA_bspline_reduce()
//
// KERNELS INVOKED:
//   kernel_bspline_reduce()
//
// AUTHOR: James Shackleford
//   DATE: 19 August, 2009
////////////////////////////////////////////////////////////////////////////////
void
CUDA_bspline_reduce (
    Dev_Pointers_Bspline* dev_ptrs,
    int num_knots
)
{
    dim3 dimGrid;
    dim3 dimBlock;

    CUDA_exec_conf_1bpe (
        &dimGrid,         // OUTPUT: Grid  dimensions
        &dimBlock,        // OUTPUT: Block dimensions
        num_knots,        // INPUT: Number of blocks
        64);              // INPUT: Threads per block

    int smemSize = 195*sizeof(float);

    kernel_bspline_reduce <<<dimGrid, dimBlock, smemSize>>> (
        dev_ptrs->grad,     // Return: interleaved dc_dp values
        dev_ptrs->cond_x,   // Input : condensed dc_dv_x values
        dev_ptrs->cond_y,   // Input : condensed dc_dv_y values
        dev_ptrs->cond_z    // Input : condensed dc_dv_z values
    );
}
////////////////////////////////////////////////////////////////////////////////


// JAS 2010.11.13
// waiting for the cpu to generate large vector fields after a super fast
// gpu driven registration was too troublesome.  this stub function is called
// in the exact same fashion as the cpu equivalent, but is faster. ^_~
void
CUDA_bspline_interpolate_vf (
    Volume* interp,
    Bspline_xform* bxf
)
{
    dim3 dimGrid;
    dim3 dimBlock;

    // Coefficient LUT
    // ----------------------------------------------------------
    float* coeff;
    plm_long coeff_size = sizeof(float) * bxf->num_coeff;

    CUDA_alloc_copy ((void **)&coeff,
                     (void **)&bxf->coeff,
                     coeff_size);

    cudaBindTexture(0, tex_coeff,
                    coeff,
                    coeff_size);

    CUDA_check_error("Failed to bind coeff to texture reference!");


    // Build B-spline LUTs & attach to textures
    // ----------------------------------------------------------
    plm_long LUT_Bspline_x_size = 4*bxf->vox_per_rgn[0]* sizeof(float);
    plm_long LUT_Bspline_y_size = 4*bxf->vox_per_rgn[1]* sizeof(float);
    plm_long LUT_Bspline_z_size = 4*bxf->vox_per_rgn[2]* sizeof(float);

    float* LUT_Bspline_x_cpu = (float*)malloc(LUT_Bspline_x_size);
    float* LUT_Bspline_y_cpu = (float*)malloc(LUT_Bspline_y_size);
    float* LUT_Bspline_z_cpu = (float*)malloc(LUT_Bspline_z_size);

    for (int j = 0; j < 4; j++)
    {
        for (int i = 0; i < bxf->vox_per_rgn[0]; i++) {
            LUT_Bspline_x_cpu[j*bxf->vox_per_rgn[0] + i] =
                CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[0]);
        }

        for (int i = 0; i < bxf->vox_per_rgn[1]; i++) {
            LUT_Bspline_y_cpu[j*bxf->vox_per_rgn[1] + i] =
                CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[1]);
        }

        for (int i = 0; i < bxf->vox_per_rgn[2]; i++) {
            LUT_Bspline_z_cpu[j*bxf->vox_per_rgn[2] + i] =
                CPU_obtain_bspline_basis_function (j, i, bxf->vox_per_rgn[2]);
        }
    }

    float *LUT_Bspline_x, *LUT_Bspline_y, *LUT_Bspline_z;

    CUDA_alloc_copy ((void **)&LUT_Bspline_x,
                     (void **)&LUT_Bspline_x_cpu,
                     LUT_Bspline_x_size);

    cudaBindTexture(0, tex_LUT_Bspline_x, LUT_Bspline_x, LUT_Bspline_x_size);

    CUDA_alloc_copy ((void **)&LUT_Bspline_y,
                     (void **)&LUT_Bspline_y_cpu,
                     LUT_Bspline_y_size);

    cudaBindTexture(0, tex_LUT_Bspline_y, LUT_Bspline_y, LUT_Bspline_y_size);

    CUDA_alloc_copy ((void **)&LUT_Bspline_z,
                     (void **)&LUT_Bspline_z_cpu,
                     LUT_Bspline_z_size);

    cudaBindTexture(0, tex_LUT_Bspline_z, LUT_Bspline_z, LUT_Bspline_z_size);

    free (LUT_Bspline_x_cpu);
    free (LUT_Bspline_y_cpu);
    free (LUT_Bspline_z_cpu);


    // Get things ready for the kernel
    // ---------------------------------------------------------------
    int3 vol_dim, rdim, cdim, vpr;
    CUDA_array2vec_int3 (&vol_dim, interp->dim);
    CUDA_array2vec_int3 (&rdim, bxf->rdims);
    CUDA_array2vec_int3 (&cdim, bxf->cdims);
    CUDA_array2vec_int3 (&vpr, bxf->vox_per_rgn);

    plm_long vf_size = interp->npix * 3*sizeof(float);



    // Kernel setup & execution
    // ---------------------------------------------------------------
    int num_blocks = 
    CUDA_exec_conf_1tpe (
        &dimGrid,          // OUTPUT: Grid  dimensions
        &dimBlock,         // OUTPUT: Block dimensions
        interp->npix,      // INPUT: Total # of threads
        192,               // INPUT: Threads per block
        true);             // INPUT: Is threads per block negotiable?

    int tpb = dimBlock.x * dimBlock.y * dimBlock.z;

    size_t sMemSize = tpb * 3*sizeof(float);
    size_t vf_gpu_size = sMemSize * num_blocks;

    float* vf_gpu;
    CUDA_alloc_zero ((void**)&vf_gpu, vf_gpu_size, cudaAllocStern);

    kernel_bspline_interpolate_vf <<<dimGrid, dimBlock, sMemSize>>> (
            vf_gpu,     // out
            vol_dim,    // in
            rdim,       // in
            cdim,       // in
            vpr         // in
    );

    cudaThreadSynchronize();
    CUDA_check_error("kernel_bspline_interpolate_vf()");

    // notice that we don't copy the "garbage" at the end of gpu memory
    cudaMemcpy(interp->img, vf_gpu, vf_size, cudaMemcpyDeviceToHost);
    CUDA_check_error("error copying vf back to CPU");


    // Clean up
    // ---------------------------------------------------------------
    cudaUnbindTexture(tex_coeff);
    cudaUnbindTexture(tex_LUT_Bspline_x);
    cudaUnbindTexture(tex_LUT_Bspline_y);
    cudaUnbindTexture(tex_LUT_Bspline_z);

    cudaFree(vf_gpu);
    cudaFree(coeff);
    cudaFree(LUT_Bspline_x);
    cudaFree(LUT_Bspline_y);
    cudaFree(LUT_Bspline_z);
}






// generates many sub-histograms of the fixed image
__global__ void
kernel_bspline_mi_hist_fix (
    float* f_hist_seg,  // partial histogram (moving image)
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
    float3 mov_origin,  // moving image origin
    float3 mov_ps       // moving image pixel spacing
)
{
    // -- Initialize Shared Memory ----------------------------
    // Amount: 32 * # bins
    extern __shared__ float s_Fixed[];

    for (long i=0; i < bins; i++) {
        s_Fixed[threadIdx.x + i*block_size] = 0.0f;
    }
    __syncthreads();
    // --------------------------------------------------------


    // only process threads that map to voxels
    if (thread_idx_global <= fdim.x * fdim.y * fdim.z) {
        int4 q;     // Voxel index (local)
        int4 p;     // Tile index
        float3 f;   // Distance from origin (in mm )
        float3 m;   // Voxel Displacement   (in mm )
        float3 n;   // Voxel Displacement   (in vox)
        float3 d;   // Deformation vector
        int fv;     // fixed voxel
    
        fv = thread_idx_global;

        setup_indices (&p, &q, &f,
            fv, fdim, vpr, rdim, img_origin, img_spacing);

        int fell_out = find_correspondence (&d, &m, &n,
            f, mov_origin, mov_ps, mdim, cdim, vpr, p, q);

        // accumulate into segmented histograms
        int idx_fbin;
        int f_mem;
        idx_fbin = (int) floorf ((f_img[fv] - offset) * delta);
        f_mem = threadIdx.x + idx_fbin*block_size;
        s_Fixed[f_mem] += !fell_out;
    }

    __syncthreads();

    // JAS 2010.12.08
    // s_Fixed looks like this:
    // |<---- Bin 0 ---->|<---- Bin 1 ---->|<---- Bin 2 ---->|
    // +-----------------+-----------------+-----------------+   etc...
    // | t0 t1 t2 ... tN | t0 t1 t2 ... tN | t0 t1 t2 ... tN |
    // +-----------------+-----------------+-----------------+
    //
    // Now, we want to merge the bins down to 1 value per bin from
    // block_size values per bin.


    // merge segmented histograms
    if (threadIdx.x < bins)
    {
        float sum = 0.0f;

        // Stagger the starting shared memory bank access for each thread so as
        // to prevent bank conflicts, which reasult in half warp difergence /
        // serialization.
        const int startPos = (threadIdx.x & 0x0F);
        const int offset   = threadIdx.x * block_size;

        for (int i=0, accumPos = startPos; i < block_size; i++) {
            sum += s_Fixed[offset + accumPos];
            if (++accumPos == block_size) {
                accumPos = 0;
            }
        }
        f_hist_seg[block_idx*bins + threadIdx.x] = sum;
    }

    // JAS 2010.12.08
    // What we have done is assign a thread to each bin, but
    // the starting element within each bin has been staggered
    // to minimize shared memory bank conflicts.
    //
    // If # of bins > # of shared memory banks, conflicts will
    // occur, but are unavoidable and will at least be minimal.
    //
    // |<---- Bin 0 ---->|<---- Bin 1 ---->|<---- Bin 2 ---->|
    // +-----------------+-----------------+-----------------+   etc...
    // | t0 t1 t2 ... tN | t0 t1 t2 ... tN | t0 t1 t2 ... tN |
    // +-----------------+-----------------+-----------------+
    //   ^                    ^                    ^
    //   | t0 start           | t1 start           | t2 start
    //
    // Note here that theadsPerBlock must be a multiple of the
    // # of shared memory banks for this to work.  Either 16
    // or 32 for 1.X and 2.X compute capability devices, respectively.
    //
    //
    // Output to global memory is:
    // |<---- Sub 0 ---->|<---- Sub 1 ---->|<---- Sub 2 ---->|
    // +-----------------+-----------------+-----------------+   etc...
    // | b0 b1 b2 ... bN | b0 b1 b2 ... bN | b0 b1 b2 ... bN |
    // +-----------------+-----------------+-----------------+
    //
    // ...many histograms of non-overlapping image subregions.
    // Sum up the sub-histograms to get the total image histogram.
    // There are num_thread_blocks sub-histograms to merge, which
    // is done by a subsequent kernel.
}



// generates many sub-histograms of the moving image
// this kernel uses an 8-neighborhood partial volume interpolation
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
)
{
    // initialize shared memory
    // --------------------------------------------------------
    // Amount: 32 * # bins
    extern __shared__ float s_Moving[];

    for (long i=0; i < bins; i++) {
        s_Moving[threadIdx.x + i*block_size] = 0.0f;
    }
    // --------------------------------------------------------

    __syncthreads();

    // only process threads that map to voxels
    // --------------------------------------------------------
    if (thread_idx_global <= fdim.x * fdim.y * fdim.z) {
        int4 q;     // Voxel index (local)
        int4 p;     // Tile index
        float3 f;   // Distance from origin (in mm )
        float3 m;   // Voxel Displacement   (in mm )
        float3 n;   // Voxel Displacement   (in vox)
        int3 n_f;   // Voxel Displacement floor
        int3 n_r;   // Voxel Displacement round
        float3 d;   // Deformation vector
        int fv;     // fixed voxel
    
        fv = thread_idx_global;

        setup_indices (&p, &q, &f,
                fv, fdim, vpr, rdim, img_origin, img_spacing);


        int fell_out = find_correspondence (&d, &m, &n,
                f, mov_origin, mov_ps, mdim, cdim, vpr, p, q);

        if (!fell_out) {
            float3 li_1, li_2;
            clamp_linear_interpolate_3d (&n, &n_f, &n_r, &li_1, &li_2, mdim);

            int nn[8];
            get_nearest_neighbors (nn, n_f, mdim);

            float w[8];
            get_weights (w, li_1, li_2);    // (a.k.a. partial volumes)

            // Accumulate Into Segmented Histograms
            int idx_mbin, m_mem;
            #pragma unroll
            for (int i=0; i<8; i++) {
                idx_mbin = (int) floorf ((m_img[nn[i]] - offset) * delta);
                m_mem = threadIdx.x + idx_mbin*block_size;
                s_Moving[m_mem] += w[i];
            }
        }
    }
    // --------------------------------------------------------

    __syncthreads();

    // merge segmented histograms
    // --------------------------------------------------------
    if (threadIdx.x < bins)
    {
        float sum = 0.0f;

        // Stagger the starting shared memory bank access for each thread so as
        // to prevent bank conflicts, which reasult in half warp difergence /
        // serialization.
        const int startPos = (threadIdx.x & 0x0F);
        const int offset   = threadIdx.x * block_size;

        for (int i=0, accumPos = startPos; i < block_size; i++) {
            sum += s_Moving[offset + accumPos];
            if (++accumPos == block_size) {
                accumPos = 0;
            }
        }

        m_hist_seg[block_idx*bins + threadIdx.x] = sum;
    }
    // --------------------------------------------------------

    // Done.
    // We now have (num_thread_blocks) partial histograms that need to be
    // merged.  This will be done with another kernel to be ran immediately
    // following the completion of this kernel.
}


////////////////////////////////////////////////////////////////////////////////
// Generates the joint histogram
//
//                 --- Neightborhood of 8 ---
//
////////////////////////////////////////////////////////////////////////////////
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
)
{
/* This code requires compute capability 1.2 or greater.
 * DO NOT compile it for lesser target architectures or
 * nvcc will complain and stop the build; thus the #if
 */
#if defined (__CUDA_ARCH__) && __CUDA_ARCH__ >= 120

    // -- Initial shared memory for locks ---------------------
    extern __shared__ float shared_mem[]; 

    float* j_locks = (float*)shared_mem;
    int total_smem = f_bins * m_bins;

    shared_memset (shared_mem, 0.0f, total_smem);
    // --------------------------------------------------------

    long j_bins; // # of joint histogram bins
    j_bins = f_bins * m_bins;

    // -- Only process threads that map to voxels -------------
    if (thread_idx_global <= fdim.x * fdim.y * fdim.z) {
        int4 q;      // Voxel index (local)
        int4 p;      // Tile index
        float3 f;    // Distance from origin (in mm )
        float3 m;    // Voxel Displacement   (in mm )
        float3 n;    // Voxel Displacement   (in vox)
        float3 d;    // Deformation vector
        int3 n_f;    // Voxel Displacement floor
        int3 n_r;    // Voxel Displacement round
        int fv;      // fixed voxel
    
        fv = thread_idx_global;

        setup_indices (&p, &q, &f,
                fv, fdim, vpr, rdim, img_origin, img_spacing);

        int fell_out = find_correspondence (&d, &m, &n,
                f, mov_origin, mov_ps, mdim, cdim, vpr, p, q);

        // did the voxel map into the moving image?
        if (fell_out) {
            atomicAdd (skipped, 1);
        } else {
            float3 li_1, li_2;
            clamp_linear_interpolate_3d (&n, &n_f, &n_r, &li_1, &li_2, mdim);

            int nn[8];
            get_nearest_neighbors (nn, n_f, mdim);

            float w[8];
            get_weights (w, li_1, li_2);    // (a.k.a. partial volumes)


            // -- Read from histograms and compute dC/dp_j * dp_j/dv --
            int idx_fbin, offset_fbin;
            int idx_mbin;
            int idx_jbin;

            // Calculate fixed bin offset into joint
            idx_fbin = (int) floorf ((f_img[fv] - f_offset) * f_delta);
            offset_fbin = idx_fbin * m_bins;

            // Maybe one day nvcc will be smart enough to honor this pragma...
            // regardless, manual unrolling doesn't offer any visible speedup
            #pragma unroll
            for (int i=0; i<8; i++) {
                idx_mbin = (int) floorf ((m_img[nn[i]] - m_offset) * m_delta);
                idx_jbin = offset_fbin + idx_mbin;
                if (idx_jbin != 0) {
                    atomic_add_float (&j_locks[idx_jbin], w[i]);
                }
            }
        }
    }

    __syncthreads();

    // copy histogram segments from shared to global memory
    int idx;
    long j_stride = block_idx * j_bins;
    int chunks = (j_bins + block_size - 1)/block_size;
    for (int i=0; i<chunks; i++) {
        idx = threadIdx.x + i*block_size;
        if (idx < j_bins) {
            j_hist[j_stride + idx] = j_locks[idx];
        }
    }

#endif // __CUDA_ARCH__
}

////////////////////////////////////////////////////////////////////////////////
// Merge Partial/Segmented Histograms
//
//   This kernel is designed to be executed after kernel_bspline_mi_hist_XXX ()
//   has genereated many partial histograms (equal to the number of thread-
//   blocks kernel_bspline_mi_hist_XXX () was executed with).  Depending on
//   the image size, this could be as high as hundredes of thousands of
//   partial histograms needing to be merged.
//
//   >> Each thread-block is responsible for a bin number.
//
//   >> A thread-block will use multiple threads to pull down
//      multiple partial histogram bin values in parallel.
//
//   >> Because there are many more partial histograms than threads,
//      the threads in a thread-block will have to iterate through
//      all of the partial histograms using a for-loop.
//
//   >> The # of for-loop iterations is equal to the number of
//      partial histograms divided by the number of threads in a block.
//
//   >> Therefore, this kernel should be launched with:
//
//      -- num_seg_hist % num_threads = 0     (num_seg_hist % blockDim.x = 0)
//      -- num_blocks = num_bins
//
//   >> This means that a search must be executed to find the largest #
//      of threads that can fit within the number of partial histograms
//      we have.  This will exhibit the largest amount of parallelism.
//
////////////////////////////////////////////////////////////////////////////////
__global__ void
kernel_bspline_mi_hist_merge (
    float *f_hist,
    float *f_hist_seg,
    long num_seg_hist
)
{
    extern __shared__ float data[];

    float sum = 0.0f;

    // -- Work through all the sub-histograms ------------------------
    for (long i = threadIdx.x; i < num_seg_hist; i += blockDim.x) {
        sum += f_hist_seg[blockIdx.x + i * gridDim.x];
    }

    data[threadIdx.x] = sum;
    // ---------------------------------------------------------------

    __syncthreads();

    // -- Sum all of the thread sums for this bin --------------------
    for (long s = blockDim.x / 2; s > 0; s >>= 1) {
        if (threadIdx.x < s) {
            data[threadIdx.x] += data[threadIdx.x + s];
        }

        __syncthreads();
    }
    // ---------------------------------------------------------------


    // -- Write the final bin value to Global ------------------------
    if (threadIdx.x == 0) {
        f_hist[blockIdx.x] = data[0];
    }
    // ---------------------------------------------------------------

    // Done.
}



////////////////////////////////////////////////////////////////////////////////
// Computes dC/dv for MI using PVI-8 interpolation
//
//                 --- Neightborhood of 8 ---
//
////////////////////////////////////////////////////////////////////////////////
__global__ void
kernel_bspline_mi_dc_dv (
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
    int pad             // INPUT: Tile padding
)
{
    // -- Only process threads that map to voxels -------------
    if (thread_idx_global > fdim.x * fdim.y * fdim.z) {
        return;
    }
    // --------------------------------------------------------


    // --------------------------------------------------------
    int3 r;     // Voxel index (global)
    int4 q;     // Voxel index (local)
    int4 p;     // Tile index


    float3 f;       // Distance from origin (in mm )
    float3 m;       // Voxel Displacement   (in mm )
    float3 n;       // Voxel Displacement   (in vox)
    float3 d;       // Deformation vector

    int3 n_f;       // Voxel Displacement floor
    int3 n_r;       // Voxel Displacement round

    int fv;     // fixed voxel
    // --------------------------------------------------------
    
    fv = thread_idx_global;

    r.z = fv / (fdim.x * fdim.y);
    r.y = (fv - (r.z * fdim.x * fdim.y)) / fdim.x;
    r.x = fv - r.z * fdim.x * fdim.y - (r.y * fdim.x);
    
    setup_indices (&p, &q, &f,
            fv, fdim, vpr, rdim, img_origin, img_spacing);

    if (r.x > (roi_offset.x + roi_dim.x) ||
        r.y > (roi_offset.y + roi_dim.y) ||
        r.z > (roi_offset.z + roi_dim.z))
    {
        return;
    }

    int fell_out = find_correspondence (&d, &m, &n,
            f, mov_origin, mov_ps, mdim, cdim, vpr, p, q);

    if (fell_out) {
        return;
    }


    float3 li_1, li_2;
    clamp_linear_interpolate_3d (&n, &n_f, &n_r, &li_1, &li_2, mdim);

    int nn[8];
    get_nearest_neighbors (nn, n_f, mdim);

    float3 dw[8];
    get_weight_derivatives (dw, li_1, li_2);    // (a.k.a. partial volumes)

    __syncthreads();

    // -- Read from histograms and compute dC/dp_j * dp_j/dv --
    float dS_dP;
    float3 dc_dv;
    int idx_fbin, offset_fbin;
    int idx_mbin;
    int idx_jbin;

    float ht = 0.000001f;

    dc_dv.x = 0.0f;
    dc_dv.y = 0.0f;
    dc_dv.z = 0.0f;


    idx_fbin = (int) floorf ((f_img[fv] - f_offset) * f_delta);
    offset_fbin = idx_fbin * m_bins;

    // JAS 2010.11.13
    // nvcc is unable too honor "#pragma unroll" due to the conditional.
    // Unrolling gives somewhat significant speed up for compute < 2.0 devices
    // and a barely noticable speed up for 2.0 devices.
#if defined (commentout)
#pragma unroll
    for (int i=0; i<8; i++) {
        idx_mbin = (int) floorf ((m_img[nn[i]] - m_offset) * m_delta);
        idx_jbin = offset_fbin + idx_mbin;
        if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
            dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
            dc_dv.x -= dw[i].x * dS_dP;
            dc_dv.y -= dw[i].y * dS_dP;
            dc_dv.z -= dw[i].z * dS_dP;
        }
    }
#endif

    // PV w1
    idx_mbin = (int) floorf ((m_img[nn[0]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[0].x * dS_dP;
        dc_dv.y -= dw[0].y * dS_dP;
        dc_dv.z -= dw[0].z * dS_dP;
    }

    // PV w2
    idx_mbin = (int) floorf ((m_img[nn[1]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[1].x * dS_dP;
        dc_dv.y -= dw[1].y * dS_dP;
        dc_dv.z -= dw[1].z * dS_dP;
    }

    // PV w3
    idx_mbin = (int) floorf ((m_img[nn[2]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[2].x * dS_dP;
        dc_dv.y -= dw[2].y * dS_dP;
        dc_dv.z -= dw[2].z * dS_dP;
    }

    // PV w4
    idx_mbin = (int) floorf ((m_img[nn[3]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[3].x * dS_dP;
        dc_dv.y -= dw[3].y * dS_dP;
        dc_dv.z -= dw[3].z * dS_dP;
    }

    // PV w5
    idx_mbin = (int) floorf ((m_img[nn[4]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[4].x * dS_dP;
        dc_dv.y -= dw[4].y * dS_dP;
        dc_dv.z -= dw[4].z * dS_dP;
    }

    // PV w6
    idx_mbin = (int) floorf ((m_img[nn[5]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[5].x * dS_dP;
        dc_dv.y -= dw[5].y * dS_dP;
        dc_dv.z -= dw[5].z * dS_dP;
    }

    // PV w7
    idx_mbin = (int) floorf ((m_img[nn[6]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[6].x * dS_dP;
        dc_dv.y -= dw[6].y * dS_dP;
        dc_dv.z -= dw[6].z * dS_dP;
    }

    // PV w8
    idx_mbin = (int) floorf ((m_img[nn[7]] - m_offset) * m_delta);
    idx_jbin = offset_fbin + idx_mbin;
    if (j_hist[idx_jbin] > ht && f_hist[idx_fbin] > ht && m_hist[idx_mbin] > ht) {
        dS_dP = logf((num_vox_f * j_hist[idx_jbin]) / (f_hist[idx_fbin] * m_hist[idx_mbin])) - score;
        dc_dv.x -= dw[7].x * dS_dP;
        dc_dv.y -= dw[7].y * dS_dP;
        dc_dv.z -= dw[7].z * dS_dP;
    }
#if defined (commentout)
#endif
    // --------------------------------------------------------


    // -- Convert from voxels to mm ---------------------------
    dc_dv.x = dc_dv.x / mov_ps.x / num_vox_f;
    dc_dv.y = dc_dv.y / mov_ps.y / num_vox_f;
    dc_dv.z = dc_dv.z / mov_ps.z / num_vox_f;
    // --------------------------------------------------------

    __syncthreads();


    // -- Finally, write out dc_dv ----------------------------
    float* dc_dv_element_x;
    float* dc_dv_element_y;
    float* dc_dv_element_z;

    dc_dv_element_x = &dc_dv_x[((vpr.x * vpr.y * vpr.z) + pad) * p.w];
    dc_dv_element_y = &dc_dv_y[((vpr.x * vpr.y * vpr.z) + pad) * p.w];
    dc_dv_element_z = &dc_dv_z[((vpr.x * vpr.y * vpr.z) + pad) * p.w];

    dc_dv_element_x = &dc_dv_element_x[q.w];
    dc_dv_element_y = &dc_dv_element_y[q.w];
    dc_dv_element_z = &dc_dv_element_z[q.w];

    dc_dv_element_x[0] = dc_dv.x;
    dc_dv_element_y[0] = dc_dv.y;
    dc_dv_element_z[0] = dc_dv.z;
    // --------------------------------------------------------
}

/* JAS 05.27.2010
 * 
 * This kernel was written as an intended replacement for
 * bspline_cuda_score_j_mse_kernel1().  The intended goal
 * was to produce a kernel with neater notation and code
 * structure that also shared the LUT_Bspline_x,y,z textured
 * lookup table that is utilized by the hyper-fast gradient
 * kernel kernel_bspline_condense ().
 * 
 * It should be noted that the LUT_Bspline texture differs
 * from the CPU based q_lut in both structure and philosophy.
 * LUT_Bspline is three separate look-up-tables which contain
 * the pre-computed basis function values in each dimension,
 * whereas the q_lut has already pre-multiplied all of these
 * results.  For the GPU, the q-LUT requires in too many memory
 * load operations, even when employing the cacheing mechanisms
 * provided by textures.  The LUT_Bspline textures rely on the GPU
 * to perform these multiplications, thus achieving superior
 * run times.
 *
 * This code was authored with the intention of unifying the
 * design philosophy of the MSE B-spline GPU implementation,
 * which was spurred by my attempts to write the upcoming
 * GPU Gems 4 chapter.
 *
 * The code now also shares more similarities with
 * the CPU code.  So, now if you know one you know the other.
 *
 * This is about 6.5% faster (on my GTX 285) than
 *   bspline_cuda_score_j_mse_kernel1()
 */
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
    int pad             // tile padding
)
{
    /* Only process threads that map to voxels */
    if (thread_idx_global > fdim.x * fdim.y * fdim.z) {
        return;
    }

    int4 p;     // Tile index
    int4 q;     // Local Voxel index (within tile)
    float3 f;   // Distance from origin (in mm )

    float3 m;   // Voxel Displacement   (in mm )
    float3 n;   // Voxel Displacement   (in vox)
    int3 n_f;   // Voxel Displacement floor
    int3 n_r;   // Voxel Displacement round
    float3 d;   // Deformation vector
    int fv;     // fixed voxel
    
    fv = thread_idx_global;

    setup_indices (&p, &q, &f,
            fv, fdim, vpr, rdim, img_origin, img_spacing);

    int fell_out = find_correspondence (&d, &m, &n,
            f, mov_origin, mov_ps, mdim, cdim, vpr, p, q);

    if (fell_out) {
        skipped[fv]++;
        return;
    }

    float3 li_1, li_2;
    clamp_linear_interpolate_3d (&n, &n_f, &n_r, &li_1, &li_2, mdim);

    float m_val = get_moving_value (n_f, mdim, li_1, li_2);

    float diff = m_val - f_img[fv];
    score[fv] = diff * diff;

    write_dc_dv (dc_dv_x, dc_dv_y, dc_dv_z,
            m_grad, diff, n_r, mdim, vpr, pad, p, q);
}


/**
 * This kernel partially computes the gradient by generating condensed dc_dv values.
 *
 * @warning It is required that input data tiles be aligned to 64 byte boundaries.
 *
 * @see CUDA_pad_64()
 * @see kernel_pad_64()
 *
 * @param cond_x Pointer to condensed dc_dv x-values
 * @param cond_y Pointer to condensed dc_dv y-values
 * @param cond_z Pointer to condensed dc_dv z-values
 * @param dc_dv_x Pointer to dc_dv x-values
 * @param dc_dv_y Pointer to dc_dv y-values
 * @param dc_dv_z Pointer to dc_dv z-values
 * @param LUT_Tile_Offsets Pointer to offset lookup table
 * @param LUT_Knot Pointer to linear knot indices
 * @param pad Amount of tile padding, in bytes
 * @param tile_dim Dimensions of input volume tiles
 * @param one_over_six The value 1/6
 *
 * @author: James A. Shackleford
 */
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
    float one_over_six)     // Input : Precomputed (GPU division is slow)
{
    int tileOffset;
    int voxel_cluster;
    int voxel_idx;
    float3 voxel_val;
    int3 voxel_loc;
    int4 tile_pos;
    float A,B,C;


    // -- Setup Shared Memory ---------------------------------
    // -- SIZE: 9*64*sizeof(float)
    // --------------------------------------------------------
    extern __shared__ float sdata[]; 
    float* sBuffer_x = (float*)sdata;           // sBuffer_x[64]
    float* sBuffer_y = (float*)&sBuffer_x[64];      // sBuffer_y[64]
    float* sBuffer_z = (float*)&sBuffer_y[64];      // sBuffer_z[64]
    float* sBuffer_redux_x = (float*)&sBuffer_z[64];    // sBuffer_redux_x[64]
    float* sBuffer_redux_y = (float*)&sBuffer_redux_x[64];  // sBuffer_redux_y[64]
    float* sBuffer_redux_z = (float*)&sBuffer_redux_y[64];  // sBuffer_redux_z[64]
    float* sBuffer_redux_x2 = (float*)&sBuffer_redux_z[64]; // sBuffer_redux_x2[64]
    float* sBuffer_redux_y2 = (float*)&sBuffer_redux_x2[64];// sBuffer_redux_y2[64]
    float* sBuffer_redux_z2 = (float*)&sBuffer_redux_y2[64];// sBuffer_redux_z2[64]
    // --------------------------------------------------------


    // Clear Shared Memory!!
    sBuffer_x[threadIdx.x] = 0.0f;
    sBuffer_y[threadIdx.x] = 0.0f;
    sBuffer_z[threadIdx.x] = 0.0f;


    // First, get the offset of where our tile starts in memory.
    tileOffset = LUT_Tile_Offsets[block_idx];

    // Main Loop for Warp Work
    // (Here we condense a tile into 64x3 floats)
    for (voxel_cluster=0; voxel_cluster < tile_dim.w; voxel_cluster+=64)
    {

    // ----------------------------------------------------------
    //                  STAGE 1 IN POWERPOINT
    // ----------------------------------------------------------
    // Second, we pulldown the current voxel cluster.
    // Each thread in the warp pulls down 1 voxel (3 values)
    // ----------------------------------------------------------
    voxel_val.x = dc_dv_x[tileOffset + voxel_cluster + threadIdx.x];
    voxel_val.y = dc_dv_y[tileOffset + voxel_cluster + threadIdx.x];
    voxel_val.z = dc_dv_z[tileOffset + voxel_cluster + threadIdx.x];
    // ----------------------------------------------------------

    // Third, find the [x,y,z] location within the current tile
    // for the voxel this thread is processing.
    voxel_idx = (voxel_cluster + threadIdx.x);
    voxel_loc.z = voxel_idx / (tile_dim.x * tile_dim.y);
    voxel_loc.y = (voxel_idx - (voxel_loc.z * tile_dim.x * tile_dim.y)) / tile_dim.x;
    voxel_loc.x = voxel_idx - voxel_loc.z * tile_dim.x * tile_dim.y - (voxel_loc.y * tile_dim.x);

    // Fourth, we will perform all 64x3 calculations on the current voxel cluster.
    // (Every thead in the warp will be doing this at the same time for its voxel)

    tile_pos.w = 0; // Current tile position within [0,63]

    for (tile_pos.z = 0; tile_pos.z < 4; tile_pos.z++)
    {
        C = tex1Dfetch(tex_LUT_Bspline_z, tile_pos.z * tile_dim.z + voxel_loc.z);
        for (tile_pos.y = 0; tile_pos.y < 4; tile_pos.y++)
        {
        B = C * tex1Dfetch(tex_LUT_Bspline_y, tile_pos.y * tile_dim.y + voxel_loc.y);
        tile_pos.x = 0;

        // #### FIRST HALF ####

        // ---------------------------------------------------------------------------------
        // Do the 1st two x-positions out of four using our two
        // blocks of shared memory for reduction

        // Calculate the b-spline multiplier for this voxel @ this tile
        // position relative to a given control knot.
        // ---------------------------------------------------------------------------------
        A = B * tex1Dfetch(tex_LUT_Bspline_x, tile_pos.x * tile_dim.x + voxel_loc.x);

        // Perform the multiplication and store to redux shared memory
        sBuffer_redux_x[threadIdx.x] = voxel_val.x * A;
        sBuffer_redux_y[threadIdx.x] = voxel_val.y * A;
        sBuffer_redux_z[threadIdx.x] = voxel_val.z * A;
        tile_pos.x++;

        // Calculate the b-spline multiplier for this voxel @ the next tile
        // position relative to a given control knot.
        A = B * tex1Dfetch(tex_LUT_Bspline_x, tile_pos.x * tile_dim.x + voxel_loc.x);

        // Perform the multiplication and store to redux shared memory
        // for the second position
        sBuffer_redux_x2[threadIdx.x] = voxel_val.x * A;
        sBuffer_redux_y2[threadIdx.x] = voxel_val.y * A;
        sBuffer_redux_z2[threadIdx.x] = voxel_val.z * A;
        __syncthreads();
        // ---------------------------------------------------------------------------------


        // ---------------------------------------------------------------------------------
        // All 64 dc_dv values in the current cluster have been processed
        // for the current 2 tile positions (out of 64 total tile positions).
                
        // We now perform a sum reduction on these 64 dc_dv values to
        // condense the data down to one value.
        // ---------------------------------------------------------------------------------
        if (threadIdx.x < 32)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 32];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 32];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 32];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 32];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 32];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 32];
        }
        __syncthreads();

        if (threadIdx.x < 16)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 16];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 16];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 16];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 16];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 16];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 16];
        }
        __syncthreads();

        if (threadIdx.x < 8)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 8];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 8];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 8];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 8];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 8];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 8];
        }
        __syncthreads();

        if (threadIdx.x < 4)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 4];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 4];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 4];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 4];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 4];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 4];
        }
        __syncthreads();

        if (threadIdx.x < 2)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 2];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 2];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 2];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 2];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 2];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 2];
        }
        __syncthreads();

        if (threadIdx.x < 1)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 1];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 1];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 1];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 1];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 1];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 1];
        }
        __syncthreads();
        // ---------------------------------------------------------------------------------



        // ---------------------------------------------------------------------------------
        // We then accumulate this single condensed value into the element of
        // shared memory that correlates to the current tile position.
        // ---------------------------------------------------------------------------------
        if (threadIdx.x == 0)
        {
            sBuffer_x[tile_pos.w] += sBuffer_redux_x[0];
            sBuffer_y[tile_pos.w] += sBuffer_redux_y[0];
            sBuffer_z[tile_pos.w] += sBuffer_redux_z[0];
            tile_pos.w++;

            sBuffer_x[tile_pos.w] += sBuffer_redux_x2[0];
            sBuffer_y[tile_pos.w] += sBuffer_redux_y2[0];
            sBuffer_z[tile_pos.w] += sBuffer_redux_z2[0];
            tile_pos.w++;
        }
        __syncthreads();
        // ---------------------------------------------------------------------------------


        // #### SECOND HALF ####

        // ---------------------------------------------------------------------------------
        // Do the 2nd two x-positions out of four using our two
        // blocks of shared memory for reduction
        // ---------------------------------------------------------------------------------
        tile_pos.x++;
        A = B * tex1Dfetch(tex_LUT_Bspline_x, tile_pos.x * tile_dim.x + voxel_loc.x);

        // Perform the multiplication and store to redux shared memory
        sBuffer_redux_x[threadIdx.x] = voxel_val.x * A;
        sBuffer_redux_y[threadIdx.x] = voxel_val.y * A;
        sBuffer_redux_z[threadIdx.x] = voxel_val.z * A;
        tile_pos.x++;

        // Calculate the b-spline multiplier for this voxel @ the next tile
        // position relative to a given control knot.
        A = B * tex1Dfetch(tex_LUT_Bspline_x, tile_pos.x * tile_dim.x + voxel_loc.x);

        // Perform the multiplication and store to redux shared memory
        // for the second position
        sBuffer_redux_x2[threadIdx.x] = voxel_val.x * A;
        sBuffer_redux_y2[threadIdx.x] = voxel_val.y * A;
        sBuffer_redux_z2[threadIdx.x] = voxel_val.z * A;
        __syncthreads();
        // ---------------------------------------------------------------------------------


                    
        // ---------------------------------------------------------------------------------
        // All 64 dc_dv values in the current cluster have been processed
        // for the current 2 tile positions (out of 64 total tile positions).
        //
        // We now perform a sum reduction on these 64 dc_dv values to
        // condense the data down to one value.
        // ---------------------------------------------------------------------------------
        if (threadIdx.x < 32)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 32];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 32];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 32];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 32];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 32];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 32];
        }
        __syncthreads();

        if (threadIdx.x < 16)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 16];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 16];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 16];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 16];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 16];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 16];
        }
        __syncthreads();

        if (threadIdx.x < 8)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 8];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 8];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 8];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 8];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 8];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 8];
        }
        __syncthreads();

        if (threadIdx.x < 4)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 4];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 4];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 4];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 4];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 4];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 4];
        }
        __syncthreads();

        if (threadIdx.x < 2)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 2];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 2];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 2];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 2];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 2];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 2];
        }
        __syncthreads();

        if (threadIdx.x < 1)
        {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + 1];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + 1];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + 1];
            sBuffer_redux_x2[threadIdx.x] += sBuffer_redux_x2[threadIdx.x + 1];
            sBuffer_redux_y2[threadIdx.x] += sBuffer_redux_y2[threadIdx.x + 1];
            sBuffer_redux_z2[threadIdx.x] += sBuffer_redux_z2[threadIdx.x + 1];
        }
        __syncthreads();
        // ---------------------------------------------------------------------------------



        // ---------------------------------------------------------------------------------
        // We then accumulate this single condensed value into the element of
        // shared memory that correlates to the current tile position.
        // ---------------------------------------------------------------------------------
        if (threadIdx.x == 0)
        {
            sBuffer_x[tile_pos.w] += sBuffer_redux_x[0];
            sBuffer_y[tile_pos.w] += sBuffer_redux_y[0];
            sBuffer_z[tile_pos.w] += sBuffer_redux_z[0];
            tile_pos.w++;

            sBuffer_x[tile_pos.w] += sBuffer_redux_x2[0];
            sBuffer_y[tile_pos.w] += sBuffer_redux_y2[0];
            sBuffer_z[tile_pos.w] += sBuffer_redux_z2[0];
            tile_pos.w++;
        }
        __syncthreads();
        // ---------------------------------------------------------------------------------

        }
    } // LOOP: 64 B-Spline Values for current voxel_cluster

    } // LOOP: voxel_clusters


    // ----------------------------------------------------------
    //                STAGE 3 IN POWERPOINT
    // ----------------------------------------------------------
    // By this point every voxel cluster within the tile has been
    // processed for every possible tile position (there are 64).
    // ----------------------------------------------------------
    // HERE, EACH WARP OPERATES ON A SINGLE TILE'S SET OF 64!!
    // ----------------------------------------------------------
    tileOffset = 64*block_idx;

    tile_pos.x = 63 - threadIdx.x;

    int knot_num;

    knot_num = LUT_Knot[tileOffset + threadIdx.x];

    cond_x[ (64*knot_num) + tile_pos.x ] = sBuffer_x[threadIdx.x];
    cond_y[ (64*knot_num) + tile_pos.x ] = sBuffer_y[threadIdx.x];
    cond_z[ (64*knot_num) + tile_pos.x ] = sBuffer_z[threadIdx.x];
    // ----------------------------------------------------------

    // Done with tile.

    // END OF KERNEL
}



////////////////////////////////////////////////////////////////////////////////
// KERNEL: kernel_bspline_reduce()
//
// * Each threadblock contains only 2 warps.
// * Each threadblock operates on 32 knots (1 at a time)
//
// * Each knot in a condense stream contains 64 single precision floats
// * Each knot is spread across the 3 condense streams [x,y,z]
// * The "high warp" will handle floats 32-63
// * The "low warp"  will handle floats  0-31
//
// * The 2 warps will work together to sum reduce the 64 floats to 1 float
// * The sum reduction result is stored in shared memory
//
// AUTHOR: James Shackleford
// DATE  : August 27th, 2009
////////////////////////////////////////////////////////////////////////////////
__global__ void
kernel_bspline_reduce (
    float* grad,        // Return: interleaved dc_dp values
    float* cond_x,      // Input : condensed dc_dv_x values
    float* cond_y,      // Input : condensed dc_dv_y values
    float* cond_z)      // Input : condensed dc_dv_z values
{
    // -- Setup Shared Memory ---------------------------------
    // -- SIZE: ((3*64)+3)*sizeof(float)
    // --------------------------------------------------------
    extern __shared__ float sdata[]; 
    float* sBuffer = (float*)sdata;             // sBuffer[3]
    float* sBuffer_redux_x = (float*)&sBuffer[3];       // sBuffer_redux_x[64]
    float* sBuffer_redux_y = (float*)&sBuffer_redux_x[64];  // sBuffer_redux_y[64]
    float* sBuffer_redux_z = (float*)&sBuffer_redux_y[64];  // sBuffer_redux_z[64]
    // --------------------------------------------------------

    // Pull down the 64 condensed dc_dv values for the knot this warp pair is working on
    sBuffer_redux_x[threadIdx.x] = cond_x[64*block_idx + threadIdx.x];
    sBuffer_redux_y[threadIdx.x] = cond_y[64*block_idx + threadIdx.x];
    sBuffer_redux_z[threadIdx.x] = cond_z[64*block_idx + threadIdx.x];

    // This thread barrier is very important!
    __syncthreads();
    
    // Perform sum reduction on the 64 condensed dc_dv values
    for(unsigned int s = 32; s > 0; s >>= 1) {
        if (threadIdx.x < s) {
            sBuffer_redux_x[threadIdx.x] += sBuffer_redux_x[threadIdx.x + s];
            sBuffer_redux_y[threadIdx.x] += sBuffer_redux_y[threadIdx.x + s];
            sBuffer_redux_z[threadIdx.x] += sBuffer_redux_z[threadIdx.x + s];
        }

        // Wait for all threads in to complete the current tier.
        __syncthreads();
    }



    // Store 3 resulting floats into the output buffer (shared memory)
    // These 3 floats are the dc_dp value [x,y,z] for the current knot
    // This shared memory store is interleaved so that the final global
    // memory store will be coalaced.
    if (threadIdx.x == 0) {
        sBuffer[0] = sBuffer_redux_x[0];
    }
    
    if (threadIdx.x == 1) {
        sBuffer[1] = sBuffer_redux_y[0];
    }

    if (threadIdx.x == 2) {
        sBuffer[2] = sBuffer_redux_z[0];
    }

    // Prevent read before write race condition
    __syncthreads();


    if (threadIdx.x < 3) {
        grad[3*block_idx + threadIdx.x] = sBuffer[threadIdx.x];
    }
}


// This kernel normalizes each of the gradient values by the
// number of voxels before the final gradient sum reduction.
__global__ void
kernel_bspline_grad_normalize (
    float *grad,
    int num_vox,
    int num_elems
)
{
    if (thread_idx_global < num_elems) {
        grad[thread_idx_global] = 2.0 * grad[thread_idx_global] / num_vox;
    }
}


// JAS 2010.11.13
// waiting for the cpu to generate large vector fields after a super fast
// gpu driven registration was too troublesome.  this kernel function allows
// us to also generate vector fields with the gpu.  much faster on my machine.
__global__ void
kernel_bspline_interpolate_vf (
    float* vf,          // OUTPUT
    int3 fdim,          // fixed  image dimensions
    int3 rdim,          //       region dimensions
    int3 cdim,          // # control points in x,y,z
    int3 vpr            // voxels per region
)
{
    extern __shared__ float shared_mem[]; 

    /* Only process threads that map to voxels */
    if (thread_idx_global <= fdim.x * fdim.y * fdim.z) {
        int4 p;     // Tile index
        int4 q;     // Local Voxel index (within tile)
        float3 f;   // Distance from origin (in mm )
        float3 d;   // Deformation vector
        int fv;     // fixed voxel
        float3 null = {0.0f, 0.0f, 0.0f};
    
        fv = thread_idx_global;

        setup_indices (&p, &q, &f,
                fv, fdim, vpr, rdim, null, null);

        bspline_interpolate (&d, cdim, vpr, p, q);

        shared_mem[3*threadIdx.x + 0] = d.x;
        shared_mem[3*threadIdx.x + 1] = d.y;
        shared_mem[3*threadIdx.x + 2] = d.z;
    }

    __syncthreads();

    stog_memcpy (vf, shared_mem, 3*block_size);
}


// This kernel will reduce a stream to a single value.  It will work for
// a stream with an arbitrary number of elements.  It is the same as 
// bspline_cuda_compute_score_kernel, with the exception that it assumes
// all values in the stream are valid and should be included in the final
// reduced value.
__global__ void
kernel_sum_reduction_pt1 (
    float *idata, 
    float *odata, 
    int   num_elems
)
{
    // Shared memory is allocated on a per block basis.  Therefore, only allocate 
    // (sizeof(data) * blocksize) memory when calling the kernel.
    extern __shared__ float sdata[];
  
    // Load data into shared memory.
    if (thread_idx_global >= num_elems) {
        sdata[thread_idx_local] = 0.0;
    } else {
        sdata[thread_idx_local] = idata[thread_idx_global];
    }

    // Wait for all threads in the block to reach this point.
    __syncthreads();
  
    // Perform the reduction in shared memory.  Stride over the block and reduce
    // parts until it is down to a single value (stored in sdata[0]).
    for (unsigned int s = block_size / 2; s > 0; s >>= 1) {
        if (thread_idx_local < s) {
            sdata[thread_idx_local] += sdata[thread_idx_local + s];
        }

        // Wait for all threads to complete this stride.
        __syncthreads();
    }
  
    // Write the result for this block back to global memory.
    if (thread_idx_local == 0) {
        odata[thread_idx_global] = sdata[0];
    }
}


// This kernel sums together the remaining partial sums that are created
// by kernel_sum_reduction_pt1()
__global__ void
kernel_sum_reduction_pt2 (
    float *idata,
    float *odata,
    int num_elems
)
{
    if (thread_idx_global == 0) {
        float sum = 0.0;
        
        for(int i = 0; i < num_elems; i += block_size) {
            sum += idata[i];
        }

        odata[0] = sum;
    }
}



// This function overwries the coefficient LUT to the GPU global memory with
// the new coefficient LUT generated by the optimizer in preparation for the
// next iteration.
void
CUDA_bspline_push_coeff (Dev_Pointers_Bspline* dev_ptrs, Bspline_xform* bxf)
{
    // Copy the coefficient LUT to the GPU.
    cudaMemcpy(dev_ptrs->coeff, bxf->coeff, dev_ptrs->coeff_size, cudaMemcpyHostToDevice);
    CUDA_check_error("Failed to copy coefficients to GPU");
}


// This function sets all elements in the score (located on the GPU) to zero in
// preparation for the next iteration of the kernel.
extern "C" void
CUDA_bspline_zero_score (Dev_Pointers_Bspline* dev_ptrs)
{
    cudaMemset(dev_ptrs->score, 0, dev_ptrs->score_size);
    CUDA_check_error("Failed to clear the score stream on GPU\n");
}


// This function sets all elemtns in the gradients (located on the GPU) to
// zero in preparation for the next iteration of the kernel.
extern "C" void
CUDA_bspline_zero_grad (Dev_Pointers_Bspline* dev_ptrs) 
{
    cudaMemset(dev_ptrs->grad, 0, dev_ptrs->grad_size);
    CUDA_check_error("Failed to clear the grad stream on GPU\n");
}





////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CPU_obtain_bspline_basis_function()
//
// AUTHOR: James Shackleford
// DATE  : 09.04.2009
////////////////////////////////////////////////////////////////////////////////
float
CPU_obtain_bspline_basis_function (
    int t_idx, 
    int vox_idx, 
    int vox_per_rgn)
{
                                
    float i = (float)vox_idx / vox_per_rgn;
    float C;
                        
    switch(t_idx) {
    case 0:
        C = (1.0/6.0) * (- 1.0 * i*i*i + 3.0 * i*i - 3.0 * i + 1.0);
        break;
    case 1:
        C = (1.0/6.0) * (+ 3.0 * i*i*i - 6.0 * i*i           + 4.0);
        break;
    case 2:
        C = (1.0/6.0) * (- 3.0 * i*i*i + 3.0 * i*i + 3.0 * i + 1.0);
        break;
    case 3:
        C = (1.0/6.0) * (+ 1.0 * i*i*i);
        break;
    default:
        C = 0.0;
        break;
    }

    return C;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CPU_calc_offsets()
//
// This function accepts the number or voxels per control region
// and the dimensions of the control grid to generate where the linear
// memory offsets lie for the beginning of each tile in a 32-byte
// aligned tile-major data organization scheme (such as that which
// is produced by kernel_row_to_tile_major().
//
// Author: James Shackleford
// Data: July 30th, 2009
////////////////////////////////////////////////////////////////////////////////
int* CPU_calc_offsets (plm_long* tile_dims, plm_long* cdims)
{
    int vox_per_tile = (tile_dims[0] * tile_dims[1] * tile_dims[2]);
    int pad = 32 - (vox_per_tile % 32);
    int num_tiles = (cdims[0]-3)*(cdims[1]-3)*(cdims[2]-3);

    int* output = (int*)malloc(num_tiles*sizeof(int));

    int i;
    for(i = 0; i < num_tiles; i++)
	output[i] = (vox_per_tile + pad) * i;

    return output;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CPU_find_knots()
//
// This function takes a tile index as an input and generates
// the indicies of the 64 control knots that it affects.
//
//  Inputs:
//    rdims[3]   - The number of regions in (x,y,x)
//                 (i.e. bxf->rdims) [See: bspline_xform.h]
//
//    cdims[3]   - The number of control points in (x,y,x)
//                 (i.e. bxf->cdims) [See: bspline_xform.h]
//
//    tile_num   - The tile index of interest -- is affected by 64 local
//                 control points (knots).
//
// Returns:
//    knots[idx] - idx is [0,63] and is the index of a local control point
//                 affecting tile_num.  knots[idx] holds the global index
//                 of the control point specified by time_num & idx.
//
// Author: James Shackleford
// Data: July 13th, 2009
////////////////////////////////////////////////////////////////////////////////
void CPU_find_knots(int* knots, int tile_num, plm_long* rdims, plm_long* cdims)
{
    int tile_loc[3];
    int i, j, k;
    int idx = 0;
    int num_tiles_x = cdims[0] - 3;
    int num_tiles_y = cdims[1] - 3;
    int num_tiles_z = cdims[2] - 3;

    // First get the [x,y,z] coordinate of
    // the tile in the control grid.
    tile_loc[0] = tile_num % num_tiles_x;
    tile_loc[1] = ((tile_num - tile_loc[0]) / num_tiles_x) % num_tiles_y;
    tile_loc[2] = ((((tile_num - tile_loc[0]) / num_tiles_x) / num_tiles_y) % num_tiles_z);
    /*
      tile_loc[0] = tile_num % rdims[0];
      tile_loc[1] = ((tile_num - tile_loc[0]) / rdims[0]) % rdims[1];
      tile_loc[2] = ((((tile_num - tile_loc[0]) / rdims[0]) / rdims[1]) % rdims[2]);
    */

    // Tiles do not start on the edges of the grid, so we
    // push them to the center of the control grid.
    tile_loc[0]++;
    tile_loc[1]++;
    tile_loc[2]++;

    // Find 64 knots' [x,y,z] coordinates
    // and convert into a linear knot index
    for (k = -1; k < 3; k++)
    	for (j = -1; j < 3; j++)
    	    for (i = -1; i < 3; i++) {
    		knots[idx++] = (cdims[0]*cdims[1]*(tile_loc[2]+k)) +
                           (cdims[0]*(tile_loc[1]+j)) +
                           (tile_loc[0]+i);
        }

}
////////////////////////////////////////////////////////////////////////////////


__device__ inline void
clamp_linear_interpolate_3d (
    float3* n,
    int3* n_f,
    int3* n_r,
    float3* li_1,
    float3* li_2,
    int3 mdim
)
{
    /* x-dimension */
    n_f->x = (int) floorf (n->x);
    n_r->x = (int) rintf (n->x);

    li_2->x = n->x - n_f->x;
    if (n_f->x < 0) {
        n_f->x = 0;
        n_r->x = 0;
        li_2->x = 0.0f;
    }
    else if (n_f->x >= (mdim.x - 1)) {
        n_f->x = mdim.x - 2;
        n_r->x = mdim.x - 1;
        li_2->x = 1.0f;
    }
    li_1->x = 1.0f - li_2->x;


    /* y-dimension */
    n_f->y = (int) floorf (n->y);
    n_r->y = (int) rintf (n->y);

    li_2->y = n->y - n_f->y;
    if (n_f->y < 0) {
        n_f->y = 0;
        n_r->y = 0;
        li_2->y = 0.0f;
    }
    else if (n_f->y >= (mdim.y - 1)) {
        n_f->y = mdim.y - 2;
        n_r->y = mdim.y - 1;
        li_2->y = 1.0f;
    }
    li_1->y = 1.0f - li_2->y;


    /* z-dimension */
    n_f->z = (int) floorf (n->z);
    n_r->z = (int) rintf (n->z);

    li_2->z = n->z - n_f->z;
    if (n_f->z < 0) {
        n_f->z = 0;
        n_r->z = 0;
        li_2->z = 0.0f;
    }
    else if (n_f->z >= (mdim.z - 1)) {
        n_f->z = mdim.z - 2;
        n_r->z = mdim.z - 1;
        li_2->z = 1.0f;
    }
    li_1->z = 1.0f - li_2->z;
}


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
)
{
    // -- Get the deformation vector d ------------------------
    bspline_interpolate (d, cdim, vpr, p, q);

    // -- Correspondence --------------------------------------
    m->x = f.x + d->x;  // Displacement in mm
    m->y = f.y + d->y;
    m->z = f.z + d->z;

    // Displacement in voxels
    n->x = (m->x - mov_origin.x) / mov_ps.x;
    n->y = (m->y - mov_origin.y) / mov_ps.y;
    n->z = (m->z - mov_origin.z) / mov_ps.z;

    if (n->x < -0.5 || n->x > mdim.x - 0.5 ||
        n->y < -0.5 || n->y > mdim.y - 0.5 ||
        n->z < -0.5 || n->z > mdim.z - 0.5)
    {
        return 1;
    }
    return 0;
    // --------------------------------------------------------
}


__device__ inline float
get_moving_value (
    int3 n_f,
    int3 mdim,
    float3 li_1,
    float3 li_2
)
{
    // -- Compute coordinates of 8 nearest neighbors ----------
    int nn[8];
    get_nearest_neighbors (nn, n_f, mdim);
    // --------------------------------------------------------

    // -- Compute Moving Image Intensity ----------------------
    float w[8];
    get_weights (w, li_1, li_2);

    w[0] *= tex1Dfetch(tex_moving_image, nn[0]);
    w[1] *= tex1Dfetch(tex_moving_image, nn[1]);
    w[2] *= tex1Dfetch(tex_moving_image, nn[2]);
    w[3] *= tex1Dfetch(tex_moving_image, nn[3]);
    w[4] *= tex1Dfetch(tex_moving_image, nn[4]);
    w[5] *= tex1Dfetch(tex_moving_image, nn[5]);
    w[6] *= tex1Dfetch(tex_moving_image, nn[6]);
    w[7] *= tex1Dfetch(tex_moving_image, nn[7]);

    return w[0] + w[1] + w[2] + w[3] + w[4] + w[5] + w[6] + w[7];
    // --------------------------------------------------------
}


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
)
{
    /* Setup Global Voxel Indices */
    int3 r;     // Voxel index (global)
    r.z = fv / (fdim.x * fdim.y);
    r.y = (fv - (r.z * fdim.x * fdim.y)) / fdim.x;
    r.x = fv - r.z * fdim.x * fdim.y - (r.y * fdim.x);
    
    /* Setup Tile Indicies */
    p->x = r.x / vpr.x;
    p->y = r.y / vpr.y;
    p->z = r.z / vpr.z;
    p->w = ((p->z * rdim.y + p->y) * rdim.x) + p->x;

    /* Setup Local Voxel Indices */
    q->x = r.x - p->x * vpr.x;
    q->y = r.y - p->y * vpr.y;
    q->z = r.z - p->z * vpr.z;
    q->w = ((q->z * vpr.y + q->y) * vpr.x) + q->x;

    /* Set up fixed image coordinates (mm) */
    f->x = img_origin.x + img_spacing.x * r.x;
    f->y = img_origin.y + img_spacing.y * r.y;
    f->z = img_origin.z + img_spacing.z * r.z;
}


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
)
{
    float* m_grad_element;
    float* dc_dv_element_x;
    float* dc_dv_element_y;
    float* dc_dv_element_z;

    m_grad_element = &m_grad[3 * n_r.z * mdim.y * mdim.x];
    m_grad_element = &m_grad_element[3 * n_r.y * mdim.x];
    m_grad_element = &m_grad_element[3 * n_r.x];

    dc_dv_element_x = &dc_dv_x[((vpr.x * vpr.y * vpr.z) + pad) * p.w];
    dc_dv_element_y = &dc_dv_y[((vpr.x * vpr.y * vpr.z) + pad) * p.w];
    dc_dv_element_z = &dc_dv_z[((vpr.x * vpr.y * vpr.z) + pad) * p.w];

    dc_dv_element_x = &dc_dv_element_x[q.w];
    dc_dv_element_y = &dc_dv_element_y[q.w];
    dc_dv_element_z = &dc_dv_element_z[q.w];

    dc_dv_element_x[0] = diff * m_grad_element[0];
    dc_dv_element_y[0] = diff * m_grad_element[1];
    dc_dv_element_z[0] = diff * m_grad_element[2];
}


__device__ inline void
get_nearest_neighbors (
    int* nn,
    int3 n_f,
    int3 mdim
)
{
    nn[0] = (n_f.z * mdim.y + n_f.y) * mdim.x + n_f.x;
    nn[1] = nn[0] + 1;
    nn[2] = nn[0] + mdim.x;
    nn[3] = nn[0] + mdim.x + 1;
    nn[4] = nn[0] + mdim.x * mdim.y;
    nn[5] = nn[0] + mdim.x * mdim.y + 1;
    nn[6] = nn[0] + mdim.x * mdim.y + mdim.x;
    nn[7] = nn[0] + mdim.x * mdim.y + mdim.x + 1;
}


__device__ inline void
get_weights (
    float* w,
    float3 li_1,
    float3 li_2
)
{
    w[0] = li_1.x * li_1.y * li_1.z;
    w[1] = li_2.x * li_1.y * li_1.z;
    w[2] = li_1.x * li_2.y * li_1.z;
    w[3] = li_2.x * li_2.y * li_1.z;
    w[4] = li_1.x * li_1.y * li_2.z;
    w[5] = li_2.x * li_1.y * li_2.z;
    w[6] = li_1.x * li_2.y * li_2.z;
    w[7] = li_2.x * li_2.y * li_2.z;
}


__device__ inline void
get_weight_derivatives (
    float3* dw,
    float3 li_1,
    float3 li_2
)
{
    dw[0].x =  -1.0f * li_1.y * li_1.z;
    dw[0].y = li_1.x *  -1.0f * li_1.z;
    dw[0].z = li_1.x * li_1.y *  -1.0f;

    dw[1].x =  +1.0f * li_1.y * li_1.z;
    dw[1].y = li_2.x *  -1.0f * li_1.z;
    dw[1].z = li_2.x * li_1.y *  -1.0f;

    dw[2].x =  -1.0f * li_2.y * li_1.z;
    dw[2].y = li_1.x *  +1.0f * li_1.z;
    dw[2].z = li_1.x * li_2.y *  -1.0f;

    dw[3].x =  +1.0f * li_2.y * li_1.z;
    dw[3].y = li_2.x *  +1.0f * li_1.z;
    dw[3].z = li_2.x * li_2.y *  -1.0f;

    dw[4].x =  -1.0f * li_1.y * li_2.z;
    dw[4].y = li_1.x *  -1.0f * li_2.z;
    dw[4].z = li_1.x * li_1.y *  +1.0f;

    dw[5].x =  +1.0f * li_1.y * li_2.z;
    dw[5].y = li_2.x *  -1.0f * li_2.z;
    dw[5].z = li_2.x * li_1.y *  +1.0f;

    dw[6].x =  -1.0f * li_2.y * li_2.z;
    dw[6].y = li_1.x *  +1.0f * li_2.z;
    dw[6].z = li_1.x * li_2.y *  +1.0f;

    dw[7].x =  +1.0f * li_2.y * li_2.z;
    dw[7].y = li_2.x *  +1.0f * li_2.z;
    dw[7].z = li_2.x * li_2.y *  +1.0f;
}


__device__ inline void
bspline_interpolate (
    float3* d,
    int3 cdim,
    int3 vpr,
    int4 p,
    int4 q
)
{
    int i, j, k, z, cidx;
    double A,B,C,P;

    d->x = 0.0f;
    d->y = 0.0f;
    d->z = 0.0f;

    z = 0;
    for (k = 0; k < 4; k++) {
    C = tex1Dfetch (tex_LUT_Bspline_z, k * vpr.z + q.z);
        for (j = 0; j < 4; j++) {
        B = tex1Dfetch (tex_LUT_Bspline_y, j * vpr.y + q.y);
            for (i = 0; i < 4; i++) {
                A = tex1Dfetch (tex_LUT_Bspline_x, i * vpr.x + q.x);
                P = A * B * C;

                cidx = 3 * ((p.z + k) * cdim.x * cdim.y 
                            + (p.y + j) * cdim.x + (p.x + i));

                d->x += P * tex1Dfetch (tex_coeff, cidx + 0);
                d->y += P * tex1Dfetch (tex_coeff, cidx + 1);
                d->z += P * tex1Dfetch (tex_coeff, cidx + 2);

                z++;
            }
        }
    }
}

// JAS 11.04.2010
// nvcc has the limitation of not being able to use
// functions from other object files.  So in order
// to share device functions, we resort to this. :-/
#include "cuda_kernel_util.inc"
