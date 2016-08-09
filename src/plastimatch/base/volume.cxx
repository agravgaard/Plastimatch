/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "direction_matrices.h"
#include "interpolate.h"
#include "interpolate_macros.h"
#include "logfile.h"
#include "plm_int.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "volume_header.h"
#include "volume.h"

template<class T, class U> static void
convert_raw (T* new_img, const Volume* vol)
{
    U* old_img = (U*) vol->img;
    if (!new_img) {
        print_and_exit ("Memory allocation failed.\n");
    }
    for (plm_long v = 0; v < vol->npix; v++) {
        new_img[v] = (T) old_img[v];
    }
}

template<class T, class U> static T* 
convert_raw (const Volume* vol)
{
    T* new_img = (T*) malloc (sizeof(T) * vol->npix);
    convert_raw<T,U> (new_img, vol);
    return new_img;
}

#define CONVERT_INPLACE(new_type,old_type,new_type_enum)                \
    {                                                                   \
        new_type *new_img = convert_raw<new_type,old_type> (ref);       \
	ref->pix_size = sizeof(new_type);				\
	ref->pix_type = new_type_enum;					\
	free (ref->img);                                                \
	ref->img = (void*) new_img;					\
    }

Volume::Volume () {
    init ();
}

Volume::Volume (
    const plm_long dim[3], 
    const float origin[3], 
    const float spacing[3], 
    const float direction_cosines[9], 
    enum Volume_pixel_type vox_type, 
    int vox_planes
) {
    create (dim, origin, spacing, direction_cosines, vox_type, 
        vox_planes);
}

Volume::Volume (
    const plm_long dim[3], 
    const float origin[3], 
    const float spacing[3], 
    const Direction_cosines& direction_cosines, 
    enum Volume_pixel_type vox_type, 
    int vox_planes
) {
    create (dim, origin, spacing, direction_cosines.get_matrix(), 
        vox_type, vox_planes);
}

Volume::Volume (
    const Volume_header& vh, 
    enum Volume_pixel_type vox_type, 
    int vox_planes
) {
    create (vh, vox_type, vox_planes);
}

Volume::~Volume ()
{
    if (this->pix_type == PT_VF_FLOAT_PLANAR) {
	float** planes = (float**) this->img;
	free (planes[0]);
	free (planes[1]);
	free (planes[2]);
    }
    free (this->img);
}

void
Volume::init ()
{
	
    for (int d = 0; d < 3; d++) {
        dim[d] = 0;
        origin[d] = 0;
        spacing[d] = 0;
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            proj[3*i+j] = 0;
            step[3*i+j] = 0;
        }
    }
    npix = 0;
    pix_type = PT_UNDEFINED;
    vox_planes = 0;
    pix_size = 0;
    img = 0;
}

void
Volume::allocate (void)
{
    if (this->pix_type == PT_VF_FLOAT_PLANAR) {
	float** der = (float**) malloc (3*sizeof(float*));
	if (!der) {
	    fprintf (stderr, "Memory allocation failed.\n");
	    exit(1);
	}
	int alloc_size = this->npix;
	for (int i=0; i < 3; i++) {
	    der[i] = (float*) malloc (alloc_size*sizeof(float));
	    if (!der[i]) {
		fprintf (stderr, "Memory allocation failed.\n");
		exit(1);
	    }
	    memset (der[i], 0, alloc_size*sizeof(float));
	}
	this->img = (void*) der;
    } else {
	this->img = (void*) malloc (this->pix_size * this->npix);
	if (!this->img) {
	    fprintf (stderr, "Memory allocation failed (alloc size = %u).\n",
		(int) (this->pix_size * this->npix));
	    exit(1);
	}
	memset (this->img, 0, this->pix_size * this->npix);
    }
}

void 
Volume::create (
    const plm_long new_dim[3], 
    const float origin[3], 
    const float spacing[3], 
    const float direction_cosines[9], 
    enum Volume_pixel_type vox_type, 
    int vox_planes
)
{
    init ();
    for (int i = 0; i < 3; i++) {
	this->dim[i] = new_dim[i];
	this->origin[i] = origin[i];
	this->spacing[i] = spacing[i];
    }
    this->npix = this->dim[0] * this->dim[1] * this->dim[2];
    this->pix_type = vox_type;
    this->vox_planes = vox_planes;

    set_direction_cosines (direction_cosines);

    switch (vox_type) {
    case PT_UCHAR:
	this->pix_size = sizeof(unsigned char);
	break;
    case PT_SHORT:
	this->pix_size = sizeof(short);
	break;
    case PT_UINT16:
	this->pix_size = sizeof(uint16_t);
	break;
    case PT_UINT32:
	this->pix_size = sizeof(uint32_t);
	break;
    case PT_INT32:
	this->pix_size = sizeof(int32_t);
	break;
    case PT_FLOAT:
	this->pix_size = sizeof(float);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
	this->pix_size = 3 * sizeof(float);
	break;
    case PT_VF_FLOAT_PLANAR:
	this->pix_size = sizeof(float);
	break;
    case PT_UCHAR_VEC_INTERLEAVED:
	this->pix_size = this->vox_planes * sizeof(unsigned char);
	break;
    default:
	fprintf (stderr, "Unhandled type in volume_create().\n");
	exit (-1);
    }

    this->allocate ();
}

void 
Volume::create (
    const Volume_header& vh, 
    enum Volume_pixel_type vox_type, 
    int vox_planes
)
{
    this->create (vh.get_dim(), vh.get_origin(), vh.get_spacing(), 
	vh.get_direction_cosines(), vox_type, vox_planes);
}

const float*
Volume::get_origin () const
{
    return this->origin;
}

void
Volume::get_origin (float *origin) const
{
    for (int d = 0; d < 3; d++) {
        origin[d] = this->origin[d];
    }
}

void
Volume::set_origin (const float origin[3])
{
    for (int d = 0; d < 3; d++) {
        this->origin[d] = origin[d];
    }
}

void 
Volume::set_direction_cosines (
    const float direction_cosines[9]
)
{
    const float identity[9] = {1., 0., 0., 0., 1., 0., 0., 0., 1.};
    const float* dc;
    if (direction_cosines) {
	dc = direction_cosines;
    } else {
	dc = identity;
    }

    this->direction_cosines.set (dc);

    compute_direction_matrices (step, proj, 
        this->direction_cosines, this->spacing);
}

template<class T> T* Volume::get_raw ()
{
    return (T*) this->img;
}

template<class T> const T* Volume::get_raw () const
{
    return (const T*) this->img;
}

const float* 
Volume::get_step (void) const
{
    return this->step;
}

const float* 
Volume::get_proj (void) const
{
    return this->proj;
}

Volume*
volume_clone_empty (Volume* ref)
{
    Volume* vout;
    vout = new Volume (ref->dim, ref->origin, ref->spacing, 
	ref->direction_cosines, ref->pix_type, ref->vox_planes);
    return vout;
}

Volume*
volume_clone (const Volume* ref)
{
    Volume* vout;
    vout = new Volume (ref->dim, ref->origin, ref->spacing, 
	ref->direction_cosines, ref->pix_type, ref->vox_planes);
    switch (ref->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
    case PT_UINT16:
    case PT_UINT32:
    case PT_INT32:
    case PT_FLOAT:
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_UCHAR_VEC_INTERLEAVED:
	memcpy (vout->img, ref->img, ref->npix * ref->pix_size);
	break;
    case PT_VF_FLOAT_PLANAR:
    default:
	fprintf (stderr, "Unsupported clone\n");
	exit (-1);
	break;
    }
    return vout;
}

Volume*
Volume::clone_raw () {
    return volume_clone (this);
}

Volume::Pointer
Volume::clone ()
{
    return Volume::New (this->clone_raw ());
}

Volume::Pointer
Volume::clone_empty ()
{
    Volume* vout = volume_clone_empty (this);
    return Volume::Pointer (vout);
}

void
volume_convert_to_float (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
	CONVERT_INPLACE (float, unsigned char, PT_FLOAT);
	break;
    case PT_SHORT:
	CONVERT_INPLACE (float, short, PT_FLOAT);
	break;
    case PT_UINT16:
	CONVERT_INPLACE (float, uint16_t, PT_FLOAT);
	break;
    case PT_UINT32:
	CONVERT_INPLACE (float, uint32_t, PT_FLOAT);
	break;
    case PT_INT32:
	CONVERT_INPLACE (float, int32_t, PT_FLOAT);
	break;
    case PT_FLOAT:
	/* Nothing to do */
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to FLOAT\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_short (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
	fprintf (stderr, "Sorry, UCHAR to SHORT is not implemented\n");
	exit (-1);
	break;
    case PT_SHORT:
	/* Nothing to do */
	break;
    case PT_UINT16:
    case PT_UINT32:
    case PT_INT32:
	fprintf (stderr, "Sorry, UINT16/UINT32/INT32 to SHORT is not implemented\n");
	exit (-1);
	break;
    case PT_FLOAT:
	CONVERT_INPLACE (short, float, PT_SHORT);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to SHORT\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_uchar (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
	/* Nothing to do */
	break;
    case PT_SHORT:
	CONVERT_INPLACE (unsigned char, short, PT_UCHAR);
	break;
    case PT_UINT16:
	CONVERT_INPLACE (unsigned char, uint16_t, PT_UCHAR);
	break;
    case PT_UINT32:
	CONVERT_INPLACE (unsigned char, uint32_t, PT_UCHAR);
	break;
    case PT_INT32:
	CONVERT_INPLACE (unsigned char, int32_t, PT_UCHAR);
	break;
    case PT_FLOAT:
	CONVERT_INPLACE (unsigned char, float, PT_UCHAR);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to UCHAR\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_uint16 (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
	fprintf (stderr, "Sorry, UCHAR/SHORT to UINT16 is not implemented\n");
	exit (-1);
	break;
    case PT_UINT16:
	/* Nothing to do */
	break;
    case PT_UINT32:
	fprintf (stderr, "Sorry, UINT32 to UINT16 is not implemented\n");
	break;
    case PT_INT32:
	fprintf (stderr, "Sorry, UINT32 to INT32 is not implemented\n");
	break;
    case PT_FLOAT:
	CONVERT_INPLACE (uint16_t, float, PT_UINT32);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to UINT32\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_uint32 (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
	fprintf (stderr, "Sorry, UCHAR/SHORT to UINT32 is not implemented\n");
	exit (-1);
	break;
    case PT_UINT16:
	fprintf (stderr, "Sorry, UINT16 to UINT32 is not implemented\n");
	exit (-1);
	break;
    case PT_UINT32:
	/* Nothing to do */
	break;
    case PT_INT32:
	fprintf (stderr, "Sorry, INT32 to UINT32 is not implemented\n");
	exit (-1);
	break;
    case PT_FLOAT:
	CONVERT_INPLACE (uint32_t, float, PT_UINT32);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to UINT32\n");
	exit (-1);
	break;
    }
}

void
volume_convert_to_int32 (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_UCHAR:
    case PT_SHORT:
	fprintf (stderr, "Sorry, UCHAR/SHORT to INT32 is not implemented\n");
	exit (-1);
	break;
    case PT_UINT16:
	fprintf (stderr, "Sorry, UINT16 to INT32 is not implemented\n");
	exit (-1);
	break;
    case PT_INT32:
	/* Nothing to do */
	break;
    case PT_UINT32:
	fprintf (stderr, "Sorry, UINT32 to INT32 is not implemented\n");
	exit (-1);
	break;
    case PT_FLOAT:
	CONVERT_INPLACE (int32_t, float, PT_INT32);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to INT32\n");
	exit (-1);
	break;
    }
}

void
vf_convert_to_interleaved (Volume* vf)
{
    switch (vf->pix_type) {
    case PT_VF_FLOAT_INTERLEAVED:
	/* Nothing to do */
	break;
    case PT_VF_FLOAT_PLANAR:
	{
	    plm_long v;
	    float** planar = (float**) vf->img;
	    float* inter = (float*) malloc (3*sizeof(float*)*vf->npix);
	    if (!inter) {
		fprintf (stderr, "Memory allocation failed.\n");
		exit(1);
	    }
	    for (v = 0; v < vf->npix; v++) {
		inter[3*v + 0] = planar[0][v];
		inter[3*v + 1] = planar[1][v];
		inter[3*v + 2] = planar[2][v];
	    }
	    free (planar[0]);
	    free (planar[1]);
	    free (planar[2]);
	    free (planar);
	    vf->img = (void*) inter;
	    vf->pix_type = PT_VF_FLOAT_INTERLEAVED;
	    vf->pix_size = 3*sizeof(float);
	}
	break;
    case PT_UCHAR:
    case PT_SHORT:
    case PT_UINT16:
    case PT_UINT32:
    case PT_INT32:
    case PT_FLOAT:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to VF\n");
	exit (-1);
	break;
    }
}

void
vf_convert_to_planar (Volume* ref)
{
    switch (ref->pix_type) {
    case PT_VF_FLOAT_INTERLEAVED:
	{
	    float* img = (float*) ref->img;
	    float** der = (float**) malloc (3*sizeof(float*));
	    if (!der) {
		printf ("Memory allocation failed.\n");
		exit(1);
	    }
	    int alloc_size = ref->npix;
	    for (int i=0; i < 3; i++) {
		der[i] = (float*) malloc (alloc_size*sizeof(float));
		if (!der[i]) {
		    print_and_exit ("Memory allocation failed.\n");
		}
	    }
	    for (plm_long i = 0; i < ref->npix; i++) {
		der[0][i] = img[3*i + 0];
		der[1][i] = img[3*i + 1];
		der[2][i] = img[3*i + 2];
	    }
	    free (ref->img);
	    ref->img = (void*) der;
	    ref->pix_type = PT_VF_FLOAT_PLANAR;
	    ref->pix_size = sizeof(float);
	}
        break;
    case PT_VF_FLOAT_PLANAR:
	/* Nothing to do */
	break;
    case PT_UCHAR:
    case PT_SHORT:
    case PT_UINT32:
    case PT_INT32:
    case PT_FLOAT:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupportd conversion to VF\n");
	exit (-1);
	break;
    }
}

void
Volume::convert (Volume_pixel_type new_type)
{
    switch (new_type) {
    case PT_UCHAR:
        volume_convert_to_uchar (this);
        break;
    case PT_SHORT:
        volume_convert_to_short (this);
        break;
    case PT_UINT16:
        volume_convert_to_uint16 (this);
        break;
    case PT_UINT32:
        volume_convert_to_uint32 (this);
        break;
    case PT_INT32:
        volume_convert_to_int32 (this);
        break;
    case PT_FLOAT:
        volume_convert_to_float (this);
        break;
    case PT_VF_FLOAT_INTERLEAVED:
        vf_convert_to_interleaved (this);
        break;
    case PT_VF_FLOAT_PLANAR:
        vf_convert_to_planar (this);
        break;
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	print_and_exit (
            "Sorry, unsupported conversion type to %d in Volume::convert()\n",
            new_type);
	break;
    }
}

template<class T> static void
clone_inner (
    Volume::Pointer& vol_out, 
    const Volume* vol_in)
{
    switch (vol_in->pix_type) {
    case PT_UCHAR:
        convert_raw<T,unsigned char> (
            vol_out->get_raw<T>(), vol_in);
        break;
    case PT_UINT16:
        convert_raw<T,uint16_t> (
            vol_out->get_raw<T>(), vol_in);
	break;
    case PT_SHORT:
        convert_raw<T,short> (
            vol_out->get_raw<T>(), vol_in);
	break;
    case PT_UINT32:
        convert_raw<T,uint32_t> (
            vol_out->get_raw<T>(), vol_in);
	break;
    case PT_INT32:
        convert_raw<T,int32_t> (
            vol_out->get_raw<T>(), vol_in);
	break;
    case PT_FLOAT:
        convert_raw<T,float> (
            vol_out->get_raw<T>(), vol_in);
	break;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	fprintf (stderr, "Sorry, unsupported conversion to INT32\n");
	exit (-1);
	break;
    }
}

Volume::Pointer
Volume::clone (Volume_pixel_type new_type) const
{
    Volume::Pointer vol_out = Volume::New ();
    vol_out->create (
        this->dim, this->origin, this->spacing, 
	this->direction_cosines, new_type,
        this->vox_planes);
    switch (new_type) {
    case PT_UCHAR:
        clone_inner<unsigned char> (vol_out, this);
        return vol_out;
    case PT_UINT16:
        clone_inner<uint16_t> (vol_out, this);
        return vol_out;
    case PT_SHORT:
        clone_inner<short> (vol_out, this);
        return vol_out;
    case PT_UINT32:
        clone_inner<uint32_t> (vol_out, this);
        return vol_out;
    case PT_INT32:
        clone_inner<int32_t> (vol_out, this);
        return vol_out;
    case PT_FLOAT:
        clone_inner<float> (vol_out, this);
        return vol_out;
    case PT_VF_FLOAT_INTERLEAVED:
    case PT_VF_FLOAT_PLANAR:
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	print_and_exit (
            "Sorry, unsupported conversion type to %d in Volume::convert_gcs()\n",
            new_type);
        return vol_out;
    }
}

#if defined (commentout)
    switch (new_type) {
    case PT_UCHAR:
        volume_convert_to_uchar (this);
        break;
    case PT_SHORT:
        volume_convert_to_short (this);
        break;
    case PT_UINT16:
        volume_convert_to_uint16 (this);
        break;
    case PT_UINT32:
        volume_convert_to_uint32 (this);
        break;
    case PT_INT32:
        volume_convert_to_int32 (this);
        break;
    case PT_FLOAT:
        volume_convert_to_float (this);
        break;
    case PT_VF_FLOAT_INTERLEAVED:
        vf_convert_to_interleaved (this);
        break;
    case PT_VF_FLOAT_PLANAR:
        vf_convert_to_planar (this);
        break;
    case PT_UCHAR_VEC_INTERLEAVED:
    default:
	/* Can't convert this */
	print_and_exit (
            "Sorry, unsupported conversion type to %d in Volume::convert()\n",
            new_type);
	break;
    }
#endif

float
Volume::get_ijk_value (const float ijk[3])
{
    plm_long ijk_f[3];
    plm_long ijk_r[3];
    float li_1[3];
    float li_2[3];

    // Compute linear interpolation fractions
    li_clamp_3d (ijk, ijk_f, ijk_r, li_1, li_2, this);

    // Find linear indices of corner voxel
    plm_long idx_floor = volume_index (this->dim, ijk_f);

    // Calc. moving voxel intensity via linear interpolation
    float val;
    float* img = (float*) this->img;
    LI_VALUE (
        val, 
        li_1[0], li_2[0],
        li_1[1], li_2[1],
        li_1[2], li_2[2],
        idx_floor,
        img, this
    );
    return val;
}

/* GCS FIX: These functions are incorrect, they ignore direction cosines */
void 
Volume::get_xyz_from_ijk (double xyz[3], const int ijk[3])
{
    xyz[0] = this->origin[0] + ijk[0] * this->spacing[0];
    xyz[1] = this->origin[1] + ijk[1] * this->spacing[1];
    xyz[2] = this->origin[2] + ijk[2] * this->spacing[2];
}

void
Volume::get_ijk_from_xyz (float ijk[3], const float xyz[3], bool* in)
{
    *in = true;

    for (int i = 0; i < 3; i++)
    {
        ijk[i] = (float) (xyz[i]-this->origin[i])/this->spacing[i];
        if (ijk[i] < 0 || ijk[i] >= this->dim[i] -1)
        {
            *in = false;
            return;
        }
    }
    return;
}

void
Volume::get_ijk_from_xyz (int ijk[3], const float xyz[3], bool* in)
{
    *in = true;

    for (int i = 0; i < 3; i++)
    {
        ijk[i] = (int) floor(xyz[i]-this->origin[i])/this->spacing[i];
        if (ijk[i] < 0 || ijk[i] >= this->dim[i])
        {
            *in = false;
            return;
        }
    }
}

bool
Volume::is_inside (const float ijk[3]) const
{
    if (ijk[0] <= -0.5 || ijk[0] >= this->dim[0] - 0.5) return false;
    if (ijk[1] <= -0.5 || ijk[1] >= this->dim[1] - 0.5) return false;
    if (ijk[2] <= -0.5 || ijk[2] >= this->dim[2] - 0.5) return false;
    return true;
}

void
Volume::move_origin_to_idx (const plm_long ijk[3])
{
    float new_origin[3];
    this->position (new_origin, ijk);
    this->set_origin (new_origin);
}

void
Volume::scale_inplace (float scale)
{
    float *img;

    if (this->pix_type != PT_FLOAT) {
	print_and_exit ("Volume::scale_inplace requires PT_FLOAT type.\n");
    }

    img = (float*) this->img;
    for (plm_long i = 0; i < this->npix; i++) {
	img[i] = img[i] * scale;
    }
}

void
Volume::debug ()
{
    lprintf ("dim:%d %d %d\n",
	(int) dim[0],
	(int) dim[1],
	(int) dim[2]
    );
    lprintf ("org:%f %f %f\n",
	origin[0],
	origin[1],
	origin[2]
    );
    lprintf ("spac:%f %f %f\n",
	spacing[0],
	spacing[1],
	spacing[2]
    );
    lprintf ("dc:%8f %8f %8f\n%8f %8f %8f\n%8f %8f %8f\n",
	direction_cosines[0],
	direction_cosines[1],
	direction_cosines[2],
	direction_cosines[3],
	direction_cosines[4],
	direction_cosines[5],
	direction_cosines[6],
	direction_cosines[7],
	direction_cosines[8]
    );
}

void
Volume::direction_cosines_debug ()
{
    lprintf ("org:%f %f %f\n",
	origin[0],
	origin[1],
	origin[2]
    );
    lprintf ("spac:%f %f %f\n",
	spacing[0],
	spacing[1],
	spacing[2]
    );
    lprintf ("dc:\n%8f %8f %8f\n%8f %8f %8f\n%8f %8f %8f\n",
	direction_cosines[0],
	direction_cosines[1],
	direction_cosines[2],
	direction_cosines[3],
	direction_cosines[4],
	direction_cosines[5],
	direction_cosines[6],
	direction_cosines[7],
	direction_cosines[8]
    );
    lprintf ("step:\n%8f %8f %8f\n%8f %8f %8f\n%8f %8f %8f\n",
	step[3*0+0],
	step[3*0+1],
	step[3*0+2],
	step[3*1+0],
	step[3*1+1],
	step[3*1+2],
	step[3*2+0],
	step[3*2+1],
	step[3*2+2]
    );
    lprintf ("proj:\n%8f %8f %8f\n%8f %8f %8f\n%8f %8f %8f\n",
	proj[3*0+0],
	proj[3*0+1],
	proj[3*0+2],
	proj[3*1+0],
	proj[3*1+1],
	proj[3*1+2],
	proj[3*2+0],
	proj[3*2+1],
	proj[3*2+2]
    );
}

// Computes the intensity differences between two images
Volume*
volume_difference (Volume* vol, Volume* warped)
{
    plm_long i, j, k;
    int p = 0; // Voxel index
    short* temp2;
    short* temp1;
    short* temp3;
    Volume* temp;

    temp = (Volume*) malloc (sizeof(Volume));
    if (!temp) {
	fprintf (stderr, "Memory allocation failed.\n");
	exit(1);
    }

    for(i=0;i<3; i++){
	temp->dim[i] = vol->dim[i];
	temp->origin[i] = vol->origin[i];
	temp->spacing[i] = vol->spacing[i];
    }

    temp->npix = vol->npix;
    temp->pix_type = vol->pix_type;

    temp->img = (void*) malloc (sizeof(short)*temp->npix);
    if (!temp->img) {
	fprintf (stderr, "Memory allocation failed.\n");
	exit(1);
    }
    memset (temp->img, -1200, sizeof(short)*temp->npix);

    p = 0; // Voxel index
    temp2 = (short*)vol->img;
    temp1 = (short*)warped->img;
    temp3 = (short*)temp->img;

    for (i=0; i < vol->dim[2]; i++) {
	for (j=0; j < vol->dim[1]; j++) {
	    for (k=0; k < vol->dim[0]; k++) {
		temp3[p] = (temp2[p] - temp1[p]) - 1200;
		p++;
	    }
	}
    }
    return temp;
}

/* Explicit instantiations */
template PLMBASE_API unsigned char* Volume::get_raw<unsigned char> ();
template PLMBASE_API const unsigned char* Volume::get_raw<unsigned char> () const;
template PLMBASE_API float* Volume::get_raw<float> ();
template PLMBASE_API const float* Volume::get_raw<float> () const;
