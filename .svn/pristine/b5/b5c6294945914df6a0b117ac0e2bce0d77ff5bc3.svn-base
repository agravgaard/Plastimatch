/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "aperture.h"
#include "plm_image.h"

class Aperture_private {
public:
    Aperture_private ()
    {
        distance = 800;
        dim[0] = 10;
        dim[1] = 10;
        center[0] = 4.5;
        center[1] = 4.5;
        spacing[0] = 1;
        spacing[1] = 1;
    }
    Aperture_private (const Aperture_private* app)
    {
        distance = app->distance;
        dim[0] = app->dim[0];
        dim[1] = app->dim[1];
        center[0] = app->center[0];
        center[1] = app->center[1];
        spacing[0] = app->spacing[0];
        spacing[1] = app->spacing[1];
    }
public:
    Plm_image::Pointer aperture_image;
    Plm_image::Pointer range_compensator_image;

    double distance;
    int dim[2];
    double center[2];
    double spacing[2];
};

Aperture::Aperture ()
{
    this->d_ptr = new Aperture_private;

	/* The aperture and the range compensator are inizialized */
	this->allocate_aperture_images();

    this->vup[0] = 0.0;
    this->vup[1] = 0.0;
    this->vup[2] = 1.0;

    this->pdn[0] = 0.0;
    this->pdn[1] = 0.0;
    this->pdn[2] = -1.0;

    this->prt[0] = 0.0;
    this->prt[1] = 1.0;
    this->prt[2] = 0.0;

    //memset (this->ic,   0, 2*sizeof (double));
    memset (this->ic_room, 0, 3*sizeof (double));
    memset (this->ul_room, 0, 3*sizeof (double));
    memset (this->incr_r, 0, 3*sizeof (double));
    memset (this->incr_c, 0, 3*sizeof (double));
}

Aperture::Aperture (const Aperture::Pointer& ap)
{
    this->d_ptr = new Aperture_private (ap->d_ptr);

	/* The aperture and the range compensator are initialized */
	this->allocate_aperture_images();

    vec3_copy (this->vup, ap->vup);
    vec3_copy (this->pdn, ap->pdn);
    vec3_copy (this->prt, ap->prt);

    vec3_copy (this->ic_room, ap->ic_room);
    vec3_copy (this->ul_room, ap->ul_room);
    vec3_copy (this->incr_r, ap->incr_r);
    vec3_copy (this->incr_c, ap->incr_c);

	Volume::Pointer ap_tmp = ap->get_aperture_volume();
	unsigned char* ap_tmp_img = (unsigned char*) ap_tmp->img;
	Volume::Pointer rc_tmp = ap->get_range_compensator_volume();
	float* rc_tmp_img = (float*) rc_tmp->img;

	unsigned char* ap_img = (unsigned char*) this->get_aperture_volume()->img;
	float* rc_img = (float*) this->get_range_compensator_volume()->img;

	for (int i = 0; i < d_ptr->dim[0] * d_ptr->dim[1]; i++)
	{
		ap_img[i] = ap_tmp_img[i];
		rc_img[i] = rc_tmp_img[i];
	}
}

Aperture::~Aperture ()
{
    delete this->d_ptr;
}

double
Aperture::get_distance () const
{
    return d_ptr->distance;
}

void
Aperture::set_distance (double distance)
{
    d_ptr->distance = distance;
}

const int*
Aperture::get_dim () const
{
    return d_ptr->dim;
}

int
Aperture::get_dim (int dim) const
{
    return d_ptr->dim[dim];
}

void
Aperture::set_dim (const int* dim)
{
    d_ptr->dim[0] = dim[0];
    d_ptr->dim[1] = dim[1];
    d_ptr->center[0] = (dim[0]-1) / 2;
    d_ptr->center[1] = (dim[1]-1) / 2;

	this->allocate_aperture_images();
}

const double*
Aperture::get_center () const
{
    return d_ptr->center;
}

double
Aperture::get_center (int dim) const
{
    return d_ptr->center[dim];
}

void
Aperture::set_center (const float* center)
{
    d_ptr->center[0] = center[0];
    d_ptr->center[1] = center[1];
}

void
Aperture::set_center (const double* center)
{
    d_ptr->center[0] = center[0];
    d_ptr->center[1] = center[1];
}

void
Aperture::set_origin (const float* origin)
{
    /* GCS FIX: This should be saved internally as an origin, 
       then only converted to pixel coordinates as needed */

    /* Compute & save as pixel coordinates */
    for (int i = 0; i < 2; i++) {
        d_ptr->center[i] = ((double) -origin[i]) / d_ptr->spacing[i];
    }
}

void
Aperture::set_origin (const double* origin)
{
    /* GCS FIX: This should be saved internally as an origin, 
       then only converted to pixel coordinates as needed */

    /* Compute & save as pixel coordinates */
    for (int i = 0; i < 2; i++) {
        d_ptr->center[i] = ((double) -origin[i]) / d_ptr->spacing[i];
    }
}

const double*
Aperture::get_spacing () const
{
    return d_ptr->spacing;
}

double
Aperture::get_spacing (int dim) const
{
    return d_ptr->spacing[dim];
}

void
Aperture::set_spacing (const float* spacing)
{
    d_ptr->spacing[0] = spacing[0];
    d_ptr->spacing[1] = spacing[1];
}

void
Aperture::set_spacing (const double* spacing)
{
    d_ptr->spacing[0] = spacing[0];
    d_ptr->spacing[1] = spacing[1];
}

void
Aperture::set_vup (const float* vup)
{
    this->vup[0] = vup[0];
    this->vup[1] = vup[1];
    this->vup[2] = vup[2];
}

void
Aperture::allocate_aperture_images ()
{
    plm_long dim[3] = {
        d_ptr->dim[0],
        d_ptr->dim[1],
        1
    };
    float origin[3] = { 0, 0, 0 };
    float spacing[3];
    spacing[0] = d_ptr->spacing[0];
    spacing[1] = d_ptr->spacing[1];
    spacing[2] = 1;

    Volume *ap_vol = new Volume (dim, origin, spacing, NULL, PT_UCHAR, 1);
    Volume *rc_vol = new Volume (dim, origin, spacing, NULL, PT_FLOAT, 1);

	/* Set the volume to values = 0 for range compensator and 1 for aperture */
	unsigned char* ap_img = (unsigned char*) ap_vol->img;
	float* rc_img = (float*) rc_vol->img;

	for (int i = 0; i < d_ptr->dim[0] * d_ptr->dim[1]; i++)
	{
		ap_img[i] = 1;
		rc_img[i] = 0;
	}

    d_ptr->aperture_image = Plm_image::New (ap_vol);
    d_ptr->range_compensator_image = Plm_image::New (rc_vol);
}

bool
Aperture::have_aperture_image ()
{
    return (bool) d_ptr->aperture_image;
}

Plm_image::Pointer&
Aperture::get_aperture_image ()
{
    return d_ptr->aperture_image;
}

Volume::Pointer&
Aperture::get_aperture_volume ()
{
    return d_ptr->aperture_image->get_volume_uchar ();
}

void 
Aperture::set_aperture_image (const char *ap_filename)
{
    d_ptr->aperture_image = Plm_image::New (ap_filename);
}

void
Aperture::set_aperture_volume (Volume::Pointer ap)
{
    d_ptr->aperture_image->set_volume(ap);
}

bool
Aperture::have_range_compensator_image ()
{
    return (bool) d_ptr->range_compensator_image;
}

Plm_image::Pointer&
Aperture::get_range_compensator_image ()
{
    return d_ptr->range_compensator_image;
}

Volume::Pointer&
Aperture::get_range_compensator_volume ()
{
    return d_ptr->range_compensator_image->get_volume_float();
}

void 
Aperture::set_range_compensator_image (const char *rc_filename)
{
    d_ptr->range_compensator_image = Plm_image::New (rc_filename);
}

void
Aperture::set_range_compensator_volume (Volume::Pointer rc)
{
    d_ptr->range_compensator_image->set_volume(rc);
}

void 
Aperture::apply_smearing_to_aperture (float smearing, float reference_depth)
{
    /* Create a structured element of the right size */
    int strel_half_size[2];
    int strel_size[2];

	strel_half_size[0] = ROUND_INT(smearing * d_ptr->distance / (reference_depth * d_ptr->spacing[0]));
	strel_half_size[1] = ROUND_INT(smearing * d_ptr->distance / (reference_depth * d_ptr->spacing[1]));

    strel_size[0] = 1 + 2 * strel_half_size[0];
    strel_size[1] = 1 + 2 * strel_half_size[1];
    unsigned char *strel = new unsigned char[strel_size[0]*strel_size[1]];
    for (int r = 0; r < strel_size[1]; r++) {
        float rf = (float) (r - strel_half_size[1]) * d_ptr->spacing[1];
        for (int c = 0; c < strel_size[0]; c++) {
            float cf = (float) (c - strel_half_size[0]) * d_ptr->spacing[0];
            int idx = r*strel_size[0] + c;

            strel[idx] = 0;
            if ((rf*rf + cf*cf) <= smearing*smearing) {
                strel[idx] = 1;
            }
        }
    }

    /* Debugging information */
    for (int r = 0; r < strel_size[1]; r++) {
        for (int c = 0; c < strel_size[0]; c++) {
            int idx = r*strel_size[0] + c;
            printf ("%d ", strel[idx]);
        }
        printf ("\n");
    }

    /* Apply smear */
    Volume::Pointer& ap_vol = this->get_aperture_volume ();
    unsigned char* ap_img = (unsigned char*) ap_vol->img;
    Volume::Pointer ap_vol_new = ap_vol->clone ();
    unsigned char* ap_img_new = (unsigned char*) ap_vol_new->img;

    for (int ar = 0; ar < d_ptr->dim[1]; ar++) {
        for (int ac = 0; ac < d_ptr->dim[0]; ac++) {
            int aidx = ar * d_ptr->dim[0] + ac;
            unsigned char ap_acc = 0;
            for (int sr = 0; sr < strel_size[1]; sr++) {
                int pr = ar + sr - strel_half_size[1];
                if (pr < 0 || pr >= d_ptr->dim[1]) {
                    continue;
                }
                for (int sc = 0; sc < strel_size[0]; sc++) {
                    int pc = ac + sc - strel_half_size[0];
                    if (pc < 0 || pc >= d_ptr->dim[0]) {
                        continue;
                    }

                    int sidx = sr * strel_size[0] + sc;
                    if (strel[sidx] == 0) {
                        continue;
                    }

                    int pidx = pr * d_ptr->dim[0] + pc;
                    if (ap_img[pidx]) {
                        ap_acc = 1;
                    }
                }
            }
            ap_img_new[aidx] = ap_acc;
        }
    }

    /* Fixate updated aperture and rc into this object */
    d_ptr->aperture_image->set_volume (ap_vol_new);

    /* Clean up */
    delete[] strel;
}

void 
Aperture::apply_smearing_to_range_compensator (float smearing, float reference_depth)
{
    /* Create a structured element of the right size */
    int strel_half_size[2];
    int strel_size[2];

	strel_half_size[0] = ROUND_INT(smearing * d_ptr->distance / (reference_depth * d_ptr->spacing[0]));
	strel_half_size[1] = ROUND_INT(smearing * d_ptr->distance / (reference_depth * d_ptr->spacing[1]));

    strel_size[0] = 1 + 2 * strel_half_size[0];
    strel_size[1] = 1 + 2 * strel_half_size[1];
    unsigned char *strel = new unsigned char[strel_size[0]*strel_size[1]];
    for (int r = 0; r < strel_size[1]; r++) {
        float rf = (float) (r - strel_half_size[1]) * d_ptr->spacing[1];
        for (int c = 0; c < strel_size[0]; c++) {
            float cf = (float) (c - strel_half_size[0]) * d_ptr->spacing[0];
            int idx = r*strel_size[0] + c;

            strel[idx] = 0;
            if ((rf*rf + cf*cf) <= smearing*smearing) {
                strel[idx] = 1;
            }
        }
    }

    /* Debugging information */
    for (int r = 0; r < strel_size[1]; r++) {
        for (int c = 0; c < strel_size[0]; c++) {
            int idx = r*strel_size[0] + c;
            printf ("%d ", strel[idx]);
        }
        printf ("\n");
    }

    /* Apply smear */
    Volume::Pointer& rc_vol = this->get_range_compensator_volume ();
    float* rc_img = (float*) rc_vol->img;
    Volume::Pointer rc_vol_new = rc_vol->clone ();
    float* rc_img_new = (float*) rc_vol_new->img;

    for (int ar = 0; ar < d_ptr->dim[1]; ar++) {
        for (int ac = 0; ac < d_ptr->dim[0]; ac++) {
            int aidx = ar * d_ptr->dim[0] + ac;
            float rc_acc = FLT_MAX;
            for (int sr = 0; sr < strel_size[1]; sr++) {
                int pr = ar + sr - strel_half_size[1];
                if (pr < 0 || pr >= d_ptr->dim[1]) {
                    continue;
                }
                for (int sc = 0; sc < strel_size[0]; sc++) {
                    int pc = ac + sc - strel_half_size[0];
                    if (pc < 0 || pc >= d_ptr->dim[0]) {
                        continue;
                    }

                    int sidx = sr * strel_size[0] + sc;
                    if (strel[sidx] == 0) {
                        continue;
                    }

                    int pidx = pr * d_ptr->dim[0] + pc;
                    if (rc_img[pidx] < rc_acc) {
                        rc_acc = rc_img[pidx];
                    }
                }
            }
            rc_img_new[aidx] = rc_acc;
        }
    }

    /* Fixate updated aperture and rc into this object */
    d_ptr->range_compensator_image->set_volume (rc_vol_new);

    /* Clean up */
    delete[] strel;
}
