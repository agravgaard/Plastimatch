/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "plm_image.h"
#include "proj_matrix.h"
#include "proj_volume.h"
#include "volume.h"

class Proj_volume_private {
public:
    Proj_volume_private () {
        vol = Volume::New();
        pmat = new Proj_matrix;

        num_steps = 0;
        step_length = 0.;
        for (int d = 0; d < 2; d++) {
            image_dim[d] = 0;
            clipping_dist[d] = 0.;
        }
        for (int d = 0; d < 3; d++) {
            nrm[d] = 0.;
            src[d] = 0.;
            iso[d] = 0.;
            ul_room[d] = 0.;
            incr_c[d] = 0.;
            incr_r[d] = 0.;
        }
    }
    ~Proj_volume_private () {
        delete pmat;
    }
public:
    Volume::Pointer vol;
    Proj_matrix *pmat;
    int num_steps;
    double step_length;
    int image_dim[2];
    double image_spacing[2];
    double clipping_dist[2];
    double nrm[3];
    double src[3];
    double iso[3];
    double ul_room[3];
    double incr_r[3];
    double incr_c[3];
};

Proj_volume::Proj_volume () {
    d_ptr = new Proj_volume_private;
}

Proj_volume::~Proj_volume () {
    delete d_ptr;
}

void
Proj_volume::debug ()
{
    const double* nrm = this->get_nrm ();

    printf ("src = %f %f %f\n", d_ptr->src[0], d_ptr->src[1], d_ptr->src[2]);
    printf ("nrm = %f %f %f\n", nrm[0], nrm[1], nrm[2]);
    printf ("ul_room = %f %f %f\n", d_ptr->ul_room[0], 
        d_ptr->ul_room[1], d_ptr->ul_room[2]);
}

void
Proj_volume::set_geometry (
    const double src[3],           // position of source (mm)
    const double iso[3],           // position of isocenter (mm)
    const double vup[3],           // dir to "top" of projection plane
    double sid,                    // dist from proj plane to source (mm)
    const int image_dim[2],        // resolution of image
    const double image_center[2],  // image center (pixels)
    const double image_spacing[2], // pixel size (mm)
    const double clipping_dist[2], // dist from src to clipping planes (mm)
    const double step_length       // spacing between planes
)
{
    double tmp[3];

    /* save input settings */
    d_ptr->image_dim[0] = image_dim[0];
    d_ptr->image_dim[1] = image_dim[1];
	d_ptr->image_spacing[0] = image_spacing[0];
	d_ptr->image_spacing[1] = image_spacing[1];
    d_ptr->src[0] = src[0];
    d_ptr->src[1] = src[1];
    d_ptr->src[2] = src[2];
    d_ptr->iso[0] = iso[0];
    d_ptr->iso[1] = iso[1];
    d_ptr->iso[2] = iso[2];
    d_ptr->step_length = step_length;

    /* build projection matrix */
    d_ptr->pmat->set (
        src, 
        iso, 
        vup, 
        sid, 
        image_center,
        image_spacing,
        image_dim
    );

    /* populate aperture orientation unit vectors */
    double nrm[3], pdn[3], prt[3];
    d_ptr->pmat->get_nrm (nrm);

    if (nrm[0] == 0 && nrm[1] == 0)
    {
        if (nrm[2] == 0) 
        { 
            printf("source and isocenter are at the same location - no beam created\n");
        }
        else
        {
            printf("the vector nrm is parallel to the z axis, pdn is defined by default as x vector and pdr as -y\n");
            pdn[0] = 0;
            pdn[1] = -1;
            pdn[2] = 0; 
            prt[0] = 1; 
            prt[1] = 0; 
            prt[2] = 0;
        }
    }
    else
    {
        d_ptr->pmat->get_pdn (pdn);
        d_ptr->pmat->get_prt (prt);
    }

    /* compute position of aperture in room coordinates */
    double ic_room[3];
    vec3_scale3 (tmp, nrm, - sid);
    vec3_add3 (ic_room, src, tmp);

    /* compute incremental change in 3d position for each change 
       in aperture row/column. */
    vec3_scale3 (d_ptr->incr_c, prt, image_spacing[0]);
    vec3_scale3 (d_ptr->incr_r, pdn, image_spacing[1]);

    /* get position of upper left pixel on panel */
    vec3_copy (d_ptr->ul_room, ic_room);
    vec3_scale3 (tmp, d_ptr->incr_c, - image_center[0]);
    vec3_add2 (d_ptr->ul_room, tmp);
    vec3_scale3 (tmp, d_ptr->incr_r, - image_center[1]);
    vec3_add2 (d_ptr->ul_room, tmp);
}

void
Proj_volume::set_clipping_dist (const double clipping_dist[2])
{
    d_ptr->clipping_dist[0] = clipping_dist[0];
    d_ptr->clipping_dist[1] = clipping_dist[1];
    d_ptr->num_steps = (int) ceil (
        (clipping_dist[1] - clipping_dist[0]) / d_ptr->step_length);
}

void
Proj_volume::allocate ()
{
    plm_long dim[3] = { d_ptr->image_dim[0], d_ptr->image_dim[1], 
                        d_ptr->num_steps };
    float origin[3] = { 0, 0, 0 };
    float spacing[3] = { 1,1,1};
    float direction_cosines[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

    printf("%lg %lg %lg \n", (float) dim[0], (float) dim[1], (float) dim[2]);
    d_ptr->vol->create (dim, origin, spacing,
        direction_cosines, PT_FLOAT, 1);
}

const int*
Proj_volume::get_image_dim ()
{
    return d_ptr->image_dim;
}

int
Proj_volume::get_image_dim (int dim)
{
    return d_ptr->image_dim [dim];
}

const double*
Proj_volume::get_incr_c ()
{
    return d_ptr->incr_c;
}

const double*
Proj_volume::get_incr_r ()
{
    return d_ptr->incr_r;
}

const double*
Proj_volume::get_nrm ()
{
    d_ptr->pmat->get_nrm (d_ptr->nrm);
    return d_ptr->nrm;
}

Proj_matrix*
Proj_volume::get_proj_matrix ()
{
    return d_ptr->pmat;
}

const double*
Proj_volume::get_src ()
{
    return d_ptr->src;
}

const double*
Proj_volume::get_iso ()
{
    return d_ptr->iso;
}

const double*
Proj_volume::get_clipping_dist ()
{
    return d_ptr->clipping_dist;
}

double
Proj_volume::get_step_length ()
{
    return d_ptr->step_length;
}

const double*
Proj_volume::get_ul_room ()
{
    return d_ptr->ul_room;
}

Volume*
Proj_volume::get_vol ()
{
    return d_ptr->vol.get();
}

const Volume*
Proj_volume::get_vol () const
{
    return d_ptr->vol.get();
}

void
Proj_volume::save (const char *filename)
{
    Plm_image(d_ptr->vol).save_image(filename);
}

void
Proj_volume::load (const char *filename)
{
    Plm_image::Pointer plm_image = Plm_image::New (filename);
    d_ptr->vol = plm_image->get_volume ();
}
