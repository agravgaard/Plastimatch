/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "itkImage.h"

#include "image_boundary.h"
#include "distance_map.h"
#include "itk_distance_map.h"
#include "itk_image_type.h"
#include "plm_image.h"
#include "volume.h"
#include "volume_header.h"

class Distance_map_private {
public:
    Distance_map_private () {
        inside_is_positive = false;
        use_squared_distance = false;
        maximum_distance = FLT_MAX;
        algorithm = Distance_map::DANIELSSON;
    }
public:
    Distance_map::Algorithm algorithm;
    bool inside_is_positive;
    bool use_squared_distance;
    float maximum_distance;
    UCharImageType::Pointer input;
    FloatImageType::Pointer output;
public:
    void run_native_danielsson ();
    void run_itk_signed_danielsson ();
    void run_itk_signed_maurer ();
    void run_itk_signed_native ();
    void run ();
protected:
    void forward_propagate_i (
        float *dm,
        const Volume::Pointer& vb,
        const float* sp2,
        plm_long j, 
        plm_long k);
    void backward_propagate_i (
        float *dm,
        const Volume::Pointer& vb,
        const float* sp2,
        plm_long j, 
        plm_long k);
    void forward_propagate_j (
        float *dm,
        const Volume::Pointer& vb,
        const float* sp2,
        plm_long k);
    void backward_propagate_j (
        float *dm,
        const Volume::Pointer& vb,
        const float* sp2,
        plm_long k);
};

/* Define some macros */
#define SQ_DIST(idx,sp2)                        \
    dm[3*idx+0]*dm[3*idx+0]*sp2[0]              \
        + dm[3*idx+1]*dm[3*idx+1]*sp2[1]        \
        + dm[3*idx+2]*dm[3*idx+2]*sp2[2]
#define SQ_DIST_I(idx,sp2)                      \
    (dm[3*idx+0]+1)*(dm[3*idx+0]+1)*sp2[0]      \
        + dm[3*idx+1]*dm[3*idx+1]*sp2[1]        \
        + dm[3*idx+2]*dm[3*idx+2]*sp2[2]
#define SQ_DIST_J(idx,sp2)                              \
    dm[3*idx+0]*dm[3*idx+0]*sp2[0]                      \
        + (dm[3*idx+1]+1)*(dm[3*idx+1]+1)*sp2[1]        \
        + dm[3*idx+2]*dm[3*idx+2]*sp2[2]
#define SQ_DIST_K(idx,sp2)                              \
    dm[3*idx+0]*dm[3*idx+0]*sp2[0]                      \
        + dm[3*idx+1]*dm[3*idx+1]*sp2[1]                \
        + (dm[3*idx+2]+1)*(dm[3*idx+2]+1)*sp2[2]

#define COPY_I(new_idx,old_idx)                 \
    dm[3*new_idx+0] = dm[3*old_idx+0] + 1;      \
    dm[3*new_idx+1] = dm[3*old_idx+1];          \
    dm[3*new_idx+2] = dm[3*old_idx+2];
#define COPY_J(new_idx,old_idx)                 \
    dm[3*new_idx+0] = dm[3*old_idx+0];          \
    dm[3*new_idx+1] = dm[3*old_idx+1] + 1;      \
    dm[3*new_idx+2] = dm[3*old_idx+2];
#define COPY_K(new_idx,old_idx)                 \
    dm[3*new_idx+0] = dm[3*old_idx+0];          \
    dm[3*new_idx+1] = dm[3*old_idx+1];          \
    dm[3*new_idx+2] = dm[3*old_idx+2] + 1;


void
Distance_map_private::forward_propagate_i (
    float *dm,
    const Volume::Pointer& vb,
    const float* sp2,
    plm_long j, 
    plm_long k)
{
    /* Forward propagate i */
    for (plm_long i = 1; i < vb->dim[0]; i++) {
        plm_long vo = vb->index (i-1, j, k);   /* "old" voxel */
        plm_long vn = vb->index (i, j, k);     /* "new" voxel */
        if (dm[3*vo] == FLT_MAX) {
            continue;
        }
        if (dm[3*vn] == FLT_MAX) {
            COPY_I (vn, vo);
            continue;
        }
        float odist = SQ_DIST_I(vo,sp2);
        float ndist = SQ_DIST(vn,sp2);
        if (odist < ndist) {
            COPY_I (vn, vo);
        }
    }
}

void
Distance_map_private::backward_propagate_i (
    float *dm,
    const Volume::Pointer& vb,
    const float* sp2,
    plm_long j, 
    plm_long k)
{
    /* Backward propagate i */
    for (plm_long i = vb->dim[0] - 2; i >= 0; i--) {
        plm_long vo = vb->index (i+1, j, k);   /* "old" voxel */
        plm_long vn = vb->index (i, j, k);     /* "new" voxel */
        if (dm[3*vo] == FLT_MAX) {
            continue;
        }
        if (dm[3*vn] == FLT_MAX) {
            COPY_I (vn, vo);
            continue;
        }
        float odist = SQ_DIST_I(vo,sp2);
        float ndist = SQ_DIST(vn,sp2);
        if (odist < ndist) {
            COPY_I (vn, vo);
        }
    }
}

void 
Distance_map_private::forward_propagate_j (
    float *dm,
    const Volume::Pointer& vb,
    const float* sp2,
    plm_long k)
{
    /* Propagate within j = 0 */
    this->forward_propagate_i (dm, vb, sp2, 0, k);
    this->backward_propagate_i (dm, vb, sp2, 0, k);

    /* Forward scan j */
    for (plm_long j = 1; j < vb->dim[1]; j++) {
        /* Propagate j */
        for (plm_long i = 0; i < vb->dim[0]; i++) {
            plm_long vo = vb->index (i, j-1, k);   /* "old" voxel */
            plm_long vn = vb->index (i, j, k);     /* "new" voxel */
            if (dm[3*vo] == FLT_MAX) {
                continue;
            }
            if (dm[3*vn] == FLT_MAX) {
                COPY_J (vn, vo);
                continue;
            }
            float odist = SQ_DIST_J(vo,sp2);
            float ndist = SQ_DIST(vn,sp2);
            if (odist < ndist) {
                COPY_J (vn, vo);
            }
        }
        /* Propagate along i */
        this->forward_propagate_i (dm, vb, sp2, j, k);
        this->backward_propagate_i (dm, vb, sp2, j, k);
    }
}

void 
Distance_map_private::backward_propagate_j (
    float *dm,
    const Volume::Pointer& vb,
    const float* sp2,
    plm_long k)
{
    /* Backward scan j */
    for (plm_long j = vb->dim[1] - 2; j >= 0; j--) {
        /* Propagate j */
        for (plm_long i = 0; i < vb->dim[0]; i++) {
            plm_long vo = vb->index (i, j+1, k);   /* "old" voxel */
            plm_long vn = vb->index (i, j, k);     /* "new" voxel */
            if (dm[3*vo] == FLT_MAX) {
                continue;
            }
            if (dm[3*vn] == FLT_MAX) {
                COPY_J (vn, vo);
                continue;
            }
            float odist = SQ_DIST_J(vo,sp2);
            float ndist = SQ_DIST(vn,sp2);
            if (odist < ndist) {
                COPY_J (vn, vo);
            }
        }
        /* Propagate along i */
        this->forward_propagate_i (dm, vb, sp2, j, k);
        this->backward_propagate_i (dm, vb, sp2, j, k);
    }
}

void
Distance_map_private::run_native_danielsson ()
{
    /* Compute boundary of image
       vb = volume of boundary, imgb = img of boundary */
    Plm_image pib (do_image_boundary (this->input));
    Volume::Pointer vb = pib.get_volume_uchar();
    unsigned char *imgb = (unsigned char*) vb->img;

    /* Convert image to native volume 
       vs = volume of set, imgs = img of set */
    Plm_image pi (this->input);
    Volume::Pointer vs = pi.get_volume_uchar();
    unsigned char *imgs = (unsigned char*) vs->img;

    /* Allocate and initialize "Danielsson array" */
    float *dm = new float[3*vb->npix];
    for (plm_long v = 0; v < vb->npix; v++) {
        bool inside = (bool) imgb[v];
        if (inside) {
            dm[3*v+0] = 0;
            dm[3*v+1] = 0;
            dm[3*v+2] = 0;
        } else {
            dm[3*v+0] = FLT_MAX;
            dm[3*v+1] = FLT_MAX;
            dm[3*v+2] = FLT_MAX;
        }
    }
    float sp2[3] = {
        vb->spacing[0] * vb->spacing[0],
        vb->spacing[1] * vb->spacing[1],
        vb->spacing[2] * vb->spacing[2]
    };

    /* GCS FIX -- This is only implemented as distance to set, 
       not distance to boundary. */

    /* GCS FIX -- I'm not entirely sure if it is required to scan 
       both forward and backward for j direction.  Need to test. */

    /* Propagate within k = 0 */
    this->forward_propagate_j (dm, vb, sp2, 0);
    this->backward_propagate_j (dm, vb, sp2, 0);

    /* Forward scan k */
    for (plm_long k = 1; k < vb->dim[2]; k++) {
        /* Propagate from prev to curr k */
        for (plm_long j = 0; j < vb->dim[1]; j++) {
            for (plm_long i = 0; i < vb->dim[0]; i++) {
                plm_long vo = vb->index (i, j, k-1);   /* "old" voxel */
                plm_long vn = vb->index (i, j, k);     /* "new" voxel */
                if (dm[3*vo] == FLT_MAX) {
                    continue;
                }
                if (dm[3*vn] == FLT_MAX) {
                    COPY_K (vn, vo);
                    continue;
                }
                float odist = SQ_DIST_K(vo,sp2);
                float ndist = SQ_DIST(vn,sp2);
                if (odist < ndist) {
                    COPY_K (vn, vo);
                }
            }
        }
        /* Propagate within curr k */
        this->forward_propagate_j (dm, vb, sp2, k);
        this->backward_propagate_j (dm, vb, sp2, k);
    }

    /* Backward scan k */
    for (plm_long k = vb->dim[2] - 2; k >= 0; k--) {
        /* Propagate from prev to curr k */
        for (plm_long j = 0; j < vb->dim[1]; j++) {
            for (plm_long i = 0; i < vb->dim[0]; i++) {
                plm_long vo = vb->index (i, j, k+1);   /* "old" voxel */
                plm_long vn = vb->index (i, j, k);     /* "new" voxel */
                if (dm[3*vo] == FLT_MAX) {
                    continue;
                }
                if (dm[3*vn] == FLT_MAX) {
                    COPY_K (vn, vo);
                    continue;
                }
                float odist = SQ_DIST_K(vo,sp2);
                float ndist = SQ_DIST(vn,sp2);
                if (odist < ndist) {
                    COPY_K (vn, vo);
                }
            }
        }
        /* Propagate within curr k */
        this->forward_propagate_j (dm, vb, sp2, k);
        this->backward_propagate_j (dm, vb, sp2, k);
    }

    /* Fill in output image */
    Plm_image::Pointer dmap = Plm_image::New (
        new Plm_image (
            new Volume (Volume_header (vb), PT_FLOAT, 1)));
    Volume::Pointer dmap_vol = dmap->get_volume_float ();
    float *dmap_img = (float*) dmap_vol->img;
    for (plm_long v = 0; v < vb->npix; v++) {
        if (!this->use_squared_distance) {
            dmap_img[v] = sqrt(SQ_DIST(v,sp2));
        }
        if (dmap_img[v] >= maximum_distance) {
            dmap_img[v] = maximum_distance;
        }
        if ((this->inside_is_positive && !imgs[v])
            || (!this->inside_is_positive && imgs[v]))
        {
            dmap_img[v] = -dmap_img[v];
        }
    }
    
    /* Free temporary memory */
    delete[] dm;

    /* Fixate distance map into private class */
    this->output = dmap->itk_float ();
}

void
Distance_map_private::run_itk_signed_danielsson ()
{
    this->output = itk_distance_map_danielsson (
        this->input,
        this->use_squared_distance,
        this->inside_is_positive);
}

void
Distance_map_private::run_itk_signed_maurer ()
{
    this->output = itk_distance_map_maurer (
        this->input,
        this->use_squared_distance,
        this->inside_is_positive);
}

void
Distance_map_private::run ()
{
    switch (this->algorithm) {
    case Distance_map::DANIELSSON:
        this->run_native_danielsson ();
        break;
    case Distance_map::ITK_DANIELSSON:
        this->run_itk_signed_danielsson ();
        break;
    case Distance_map::ITK_MAURER:
    default:
        this->run_itk_signed_maurer ();
        break;
    }
}

Distance_map::Distance_map () {
    d_ptr = new Distance_map_private;
}

Distance_map::~Distance_map () {
    delete d_ptr;
}

void
Distance_map::set_input_image (const std::string& image_fn)
{
    Plm_image pli (image_fn);
    d_ptr->input = pli.itk_uchar();
}

void
Distance_map::set_input_image (const char* image_fn)
{
    Plm_image pli (image_fn);
    d_ptr->input = pli.itk_uchar();
}

void
Distance_map::set_input_image (UCharImageType::Pointer image)
{
    d_ptr->input = image;
}

void
Distance_map::set_input_image (const Plm_image::Pointer& image)
{
    Plm_image::Pointer pi_clone = image->clone ();
    d_ptr->input = pi_clone->itk_uchar ();
}

void 
Distance_map::set_use_squared_distance (bool use_squared_distance)
{
    d_ptr->use_squared_distance = use_squared_distance;
}

void 
Distance_map::set_maximum_distance (float maximum_distance)
{
    d_ptr->maximum_distance = maximum_distance;
}

void 
Distance_map::set_inside_is_positive (bool inside_is_positive)
{
    d_ptr->inside_is_positive = inside_is_positive;
}

void 
Distance_map::set_algorithm (const std::string& algorithm)
{
    if (algorithm == "danielsson" || algorithm == "native_danielsson") {
        d_ptr->algorithm = Distance_map::DANIELSSON;
    }
    else if (algorithm == "itk-danielsson") {
        d_ptr->algorithm = Distance_map::ITK_DANIELSSON;
    }
    else if (algorithm == "maurer") {
        d_ptr->algorithm = Distance_map::ITK_MAURER;
    }
    else if (algorithm == "itk-maurer" || algorithm == "itk_maurer") {
        d_ptr->algorithm = Distance_map::ITK_MAURER;
    }
    /* Else do nothing */
}

void
Distance_map::run ()
{
    d_ptr->run ();
}

FloatImageType::Pointer
Distance_map::get_output_image ()
{
    return d_ptr->output;
}
