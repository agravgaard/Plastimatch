/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#if (OPENMP_FOUND)
#include <omp.h>
#endif

#include "geometry_chooser.h"
#include "itk_image_load.h"
#include "logfile.h"
#include "plm_image_header.h"
#include "plm_int.h"
#include "vf_convolve.h"
#include "vf_invert.h"
#include "volume.h"
#include "volume_header.h"
#include "xform.h"

class Vf_invert_private {
public:
    Vf_invert_private () {
        iterations = 20;
        vf_out = 0;
    }
    ~Vf_invert_private () {
        delete vf_out;
    }
public:
    int iterations;
    Geometry_chooser gchooser;
    DeformationFieldType::Pointer input_vf;
    Volume *vf_out;
};

Vf_invert::Vf_invert () {
    this->d_ptr = new Vf_invert_private;
}

Vf_invert::~Vf_invert () {
    delete this->d_ptr;
}

void 
Vf_invert::set_input_vf (const char* vf_fn)
{
    d_ptr->input_vf = itk_image_load_float_field (vf_fn);
    d_ptr->gchooser.set_reference_image (d_ptr->input_vf);
}

void 
Vf_invert::set_input_vf (
    const DeformationFieldType::Pointer vf)
{
    d_ptr->input_vf = vf;
    d_ptr->gchooser.set_reference_image (d_ptr->input_vf);
}

void 
Vf_invert::set_fixed_image (const char* image_fn)
{
    d_ptr->gchooser.set_fixed_image (image_fn);
}

void 
Vf_invert::set_fixed_image (
    const FloatImageType::Pointer image)
{
    d_ptr->gchooser.set_fixed_image (image);
}

void 
Vf_invert::set_dim (const plm_long dim[3])
{
    d_ptr->gchooser.set_dim (dim);
}

void 
Vf_invert::set_origin (const float origin[3])
{
    d_ptr->gchooser.set_origin (origin);
}

void 
Vf_invert::set_spacing (const float spacing[3])
{
    d_ptr->gchooser.set_spacing (spacing);
}

void 
Vf_invert::set_direction_cosines (const float direction_cosines[9])
{
    Direction_cosines dc (direction_cosines);
    d_ptr->gchooser.set_direction_cosines (dc);
}

void 
Vf_invert::set_iterations (int iterations)
{
    d_ptr->iterations = iterations;
}

void 
Vf_invert::run ()
{
    /* Compute geometry of output volume */
    const Plm_image_header *pih = d_ptr->gchooser.get_geometry ();
    Volume_header vh (pih);

    /* Create mask volume */
    Volume *mask = new Volume (vh, PT_UCHAR, 1);

    /* Create tmp volume */
    Volume *vf_inv = new Volume (vh, PT_VF_FLOAT_INTERLEAVED, 1);

    /* Convert input vf to native, interleaved format */
    Xform xf_itk;
    xf_itk.set_itk_vf (d_ptr->input_vf);
    Xform *xf = new Xform;
    const Plm_image_header pih_in (d_ptr->input_vf);
    xform_to_gpuit_vf (xf, &xf_itk, &pih_in);
    Volume::Pointer vf_in = xf->get_gpuit_vf ();
    vf_convert_to_interleaved (vf_in.get());

    /* Populate mask & tmp volume */
    unsigned char *img_mask = (unsigned char*) mask->img;
    float *img_in = (float*) vf_in->img;
    float *img_inv = (float*) vf_inv->img;

#pragma omp parallel for 
    LOOP_Z_OMP (k, vf_in) {
        plm_long fijk[3];      /* Index within fixed image (vox) */
        float fxyz[3];         /* Position within fixed image (mm) */
        fijk[2] = k;
        fxyz[2] = vf_in->origin[2] + fijk[2] * vf_in->step[2*3+2];
        LOOP_Y (fijk, fxyz, vf_in) {
            LOOP_X (fijk, fxyz, vf_in) {
                float mijk[3];
                plm_long mijk_r[3], midx;
                plm_long v = volume_index (vf_in->dim, fijk);
		float *dxyz = &img_in[3*v];
		float mo_xyz[3] = {
		    fxyz[0] + dxyz[0] - vf_inv->origin[0],
		    fxyz[1] + dxyz[1] - vf_inv->origin[1],
		    fxyz[2] + dxyz[2] - vf_inv->origin[2]
		};

		mijk[2] = PROJECT_Z(mo_xyz,vf_inv->proj);
		mijk[1] = PROJECT_Y(mo_xyz,vf_inv->proj);
		mijk[0] = PROJECT_X(mo_xyz,vf_inv->proj);

                if (!vf_inv->is_inside (mijk)) continue;

                mijk_r[2] = ROUND_PLM_LONG (mijk[2]);
                mijk_r[1] = ROUND_PLM_LONG (mijk[1]);
                mijk_r[0] = ROUND_PLM_LONG (mijk[0]);
                midx = volume_index (vf_inv->dim, mijk_r);
                img_inv[3*midx+0] = -img_in[3*v+0];
                img_inv[3*midx+1] = -img_in[3*v+1];
                img_inv[3*midx+2] = -img_in[3*v+2];
                img_mask[midx] ++;
            }
        }
    }

    /* We're done with input volume now. */
    delete xf;

    /* Create tmp & output volumes */
    Volume *vf_out = new Volume (vh, PT_VF_FLOAT_INTERLEAVED, 3);
    float *img_out = (float*) vf_out->img;
    Volume *vf_smooth = new Volume (vh, PT_VF_FLOAT_INTERLEAVED, 3);
    float *img_smooth = (float*) vf_smooth->img;

    /* Iterate, pasting and smoothing */
    printf ("Paste and smooth loop\n");
    for (int it = 0; it < d_ptr->iterations; it++) {
        printf ("Iteration %d/%d\n", it, d_ptr->iterations);
        /* Paste */
        for (plm_long v = 0, k = 0; k < vf_out->dim[2]; k++) {
            for (plm_long j = 0; j < vf_out->dim[1]; j++) {
                for (plm_long i = 0; i < vf_out->dim[0]; i++, v++) {
                    if (img_mask[v]) {
                        img_smooth[3*v+0] = img_inv[3*v+0];
                        img_smooth[3*v+1] = img_inv[3*v+1];
                        img_smooth[3*v+2] = img_inv[3*v+2];
                    } else {
                        img_smooth[3*v+0] = img_out[3*v+0];
                        img_smooth[3*v+1] = img_out[3*v+1];
                        img_smooth[3*v+2] = img_out[3*v+2];
                    }
                }
            }
        }

        /* Smooth the estimate into vf_out.  The volumes are ping-ponged. */
        float ker[3] = { 0.3, 0.4, 0.3 };
        printf ("Convolving\n");
        vf_convolve_x (vf_out, vf_smooth, ker, 3);
        vf_convolve_y (vf_smooth, vf_out, ker, 3);
        vf_convolve_z (vf_out, vf_smooth, ker, 3);
    }
    printf ("Done.\n");

    /* We're done with all these images now */
    delete mask;
    delete vf_inv;
    delete vf_smooth;

    /* Save the output image! */
    d_ptr->vf_out = vf_out;
}

const Volume*
Vf_invert::get_output_volume ()
{
    return d_ptr->vf_out;
}
