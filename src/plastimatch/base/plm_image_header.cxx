/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "itkImageRegionIterator.h"
#include <stdio.h>
#include <math.h>

#include "bspline_xform.h"
#include "direction_cosines.h"
#include "direction_matrices.h"
#include "itk_directions.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "volume.h"
#include "volume_header.h"

class Plm_image_header_private
{
public:
    OriginType m_origin;
};

/* -----------------------------------------------------------------------
   Constructors, destructors, operator=
   ----------------------------------------------------------------------- */
Plm_image_header::Plm_image_header ()
{
    d_ptr = new Plm_image_header_private;
}

Plm_image_header::Plm_image_header (
    plm_long dim[3], float origin[3], float spacing[3])
{
    d_ptr = new Plm_image_header_private;
    this->set_from_gpuit (dim, origin, spacing, 0);
}

Plm_image_header::Plm_image_header (
    plm_long dim[3], float origin[3], float spacing[3],
    float direction_cosines[9])
{
    d_ptr = new Plm_image_header_private;
    this->set_from_gpuit (dim, origin, spacing, direction_cosines);
}

Plm_image_header::Plm_image_header (
    const RegionType& region, const OriginType& origin,
    const SpacingType& spacing, const DirectionType& direction)
{
    d_ptr = new Plm_image_header_private;
    this->set (region, origin,spacing, direction);
}

Plm_image_header::Plm_image_header (Plm_image *pli) 
{
    d_ptr = new Plm_image_header_private;
    this->set_from_plm_image (pli);
}

Plm_image_header::Plm_image_header (const Plm_image& pli) 
{
    d_ptr = new Plm_image_header_private;
    this->set_from_plm_image (pli);
}

Plm_image_header::Plm_image_header (const Plm_image::Pointer& pli) 
{
    d_ptr = new Plm_image_header_private;
    this->set_from_plm_image (pli);
}

Plm_image_header::Plm_image_header (const Volume_header& vh) 
{
    d_ptr = new Plm_image_header_private;
    this->set (vh);
}

Plm_image_header::Plm_image_header (const Volume& vol) 
{
    d_ptr = new Plm_image_header_private;
    this->set (vol);
}

Plm_image_header::Plm_image_header (const Volume* vol) 
{
    d_ptr = new Plm_image_header_private;
    this->set (vol);
}

Plm_image_header::Plm_image_header (Volume* vol) 
{
    d_ptr = new Plm_image_header_private;
    this->set (vol);
}

template<class T> 
Plm_image_header::Plm_image_header (T image) {
    d_ptr = new Plm_image_header_private;
    this->set_from_itk_image (image);
}

Plm_image_header::Plm_image_header (const Plm_image_header& other)
{
    d_ptr = new Plm_image_header_private ();
    this->m_origin = other.m_origin;
    this->m_spacing = other.m_spacing;
    this->m_region = other.m_region;
    this->m_direction = other.m_direction;
}

Plm_image_header::~Plm_image_header ()
{
    delete d_ptr;
}

const Plm_image_header& 
Plm_image_header::operator= (const Plm_image_header& other)
{
    this->m_origin = other.m_origin;
    this->m_spacing = other.m_spacing;
    this->m_region = other.m_region;
    this->m_direction = other.m_direction;
    return *this;
}

/* -----------------------------------------------------------------------
   Getters and Setters
   ----------------------------------------------------------------------- */
int 
Plm_image_header::dim (int d) const
{
    return m_region.GetSize()[d];
}

float 
Plm_image_header::origin (int d) const
{
    return m_origin[d];
}

float 
Plm_image_header::spacing (int d) const
{
    return m_spacing[d];
}

void
Plm_image_header::set_dim (const plm_long dim[3])
{
    RegionType::SizeType itk_size;
    RegionType::IndexType itk_index;
    for (unsigned int d = 0; d < 3; d++) {
	itk_index[d] = 0;
	itk_size[d] = dim[d];
    }
    m_region.SetSize (itk_size);
    m_region.SetIndex (itk_index);
}

void
Plm_image_header::set_origin (const float origin[3])
{
    for (unsigned int d = 0; d < 3; d++) {
	this->m_origin[d] = origin[d];
    }
}

void
Plm_image_header::set_spacing (const float spacing[3])
{
    for (unsigned int d = 0; d < 3; d++) {
	this->m_spacing[d] = spacing[d];
    }
}

void
Plm_image_header::set_direction_cosines (const float direction_cosines[9])
{
    if (direction_cosines) {
	itk_direction_from_dc (&m_direction, direction_cosines);
    } else {
	itk_direction_set_identity (&m_direction);
    }
}

void
Plm_image_header::set_direction_cosines (const Direction_cosines& dc)
{
    itk_direction_from_dc (&m_direction, dc);
}

void
Plm_image_header::set (const Plm_image_header& src)
{
    this->m_origin = src.m_origin;
    this->m_spacing = src.m_spacing;
    this->m_region = src.m_region;
    this->m_direction = src.m_direction;
}

void
Plm_image_header::set (
    const plm_long dim[3],
    const float origin[3],
    const float spacing[3],
    const float direction_cosines[9])
{
    this->set_dim (dim);
    this->set_origin (origin);
    this->set_spacing (spacing);
    this->set_direction_cosines (direction_cosines);
}

void
Plm_image_header::set (
    const plm_long dim[3],
    const float origin[3],
    const float spacing[3],
    const Direction_cosines& dc)
{
    this->set (dim, origin, spacing, dc.get_matrix());
}

void
Plm_image_header::set_from_gpuit (
    const plm_long dim[3],
    const float origin[3],
    const float spacing[3],
    const float direction_cosines[9])
{
    this->set (dim, origin, spacing, direction_cosines);
}

void
Plm_image_header::set_from_gpuit_bspline (Bspline_xform *bxf)
{
    this->set (
	bxf->img_dim,
	bxf->img_origin,
	bxf->img_spacing,
	bxf->dc);
}

void
Plm_image_header::set_from_plm_image (const Plm_image *pli)
{
    switch (pli->m_type) {
    case PLM_IMG_TYPE_ITK_UCHAR:
	this->set_from_itk_image (pli->m_itk_uchar);
	break;
    case PLM_IMG_TYPE_ITK_SHORT:
	this->set_from_itk_image (pli->m_itk_short);
	break;
    case PLM_IMG_TYPE_ITK_USHORT:
	this->set_from_itk_image (pli->m_itk_ushort);
	break;
    case PLM_IMG_TYPE_ITK_LONG:
	this->set_from_itk_image (pli->m_itk_int32);
	break;
    case PLM_IMG_TYPE_ITK_ULONG:
	this->set_from_itk_image (pli->m_itk_uint32);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT:
	this->set_from_itk_image (pli->m_itk_float);
	break;
    case PLM_IMG_TYPE_ITK_DOUBLE:
	this->set_from_itk_image (pli->m_itk_double);
	break;
    case PLM_IMG_TYPE_GPUIT_UCHAR:
    case PLM_IMG_TYPE_GPUIT_SHORT:
    case PLM_IMG_TYPE_GPUIT_UINT32:
    case PLM_IMG_TYPE_GPUIT_FLOAT:
    case PLM_IMG_TYPE_GPUIT_FLOAT_FIELD:
    {
	const Volume* vol = pli->get_vol ();
	set_from_gpuit (vol->dim, vol->origin, vol->spacing,
	    vol->direction_cosines);
	break;
    }
    case PLM_IMG_TYPE_ITK_UCHAR_VEC:
	this->set_from_itk_image (pli->m_itk_uchar_vec);
	break;
    case PLM_IMG_TYPE_ITK_FLOAT_FIELD:
    case PLM_IMG_TYPE_ITK_CHAR:
    default:
	print_and_exit ("Unhandled image type (%s) in set_from_plm_image\n",
	    plm_image_type_string (pli->m_type));
	break;
    }
}

void
Plm_image_header::set_from_plm_image (const Plm_image& pli)
{
    this->set_from_plm_image (&pli);
}

void
Plm_image_header::set_from_plm_image (const Plm_image::Pointer& pli)
{
    this->set_from_plm_image (pli.get());
}

void
Plm_image_header::set (const Volume_header& vh)
{
    this->set_from_gpuit (vh.get_dim(), vh.get_origin(), 
	vh.get_spacing(), vh.get_direction_cosines());
}

void
Plm_image_header::set_from_volume_header (const Volume_header& vh)
{
    this->set (vh);
}

void
Plm_image_header::set (const Volume& vol)
{
    this->set_from_gpuit (vol.dim, vol.origin,
	vol.spacing, vol.direction_cosines);
}

void
Plm_image_header::set (const Volume* vol)
{
    this->set_from_gpuit (vol->dim, vol->origin,
	vol->spacing, vol->direction_cosines);
}

void Plm_image_header::set (
    const RegionType& region, const OriginType& origin,
    const SpacingType& spacing, const DirectionType& direction)
{
    m_region = region;
    m_origin = origin;
    m_spacing = spacing;
    m_direction = direction;

    /* Adjust origin and set index to zero in case of non-zero 
       ITK region index */
    const IndexType& index = region.GetIndex();
    for (int d1 = 0; d1 < 3; d1++) {
        for (int d2 = 0; d2 < 3; d2++) {
            m_origin[d2] += index[d1] * spacing[d1] * direction[d2][d1];
        }
    }
    IndexType i2;
    i2[0] = i2[1] = i2[2] = 0;
    m_region.SetIndex (i2);
}

template<class T> 
void 
Plm_image_header::set_from_itk_image (const T& image)
{
    m_origin = itk_image_origin (image);
    m_spacing = image->GetSpacing ();
    m_region = itk_image_region (image);
    m_direction = image->GetDirection ();
}

template<class T> 
void 
Plm_image_header::set_from_itk_image (const T* image)
{
    m_origin = itk_image_origin (image);
    m_spacing = image->GetSpacing ();
    m_region = itk_image_region (image);
    m_direction = image->GetDirection ();
}

const OriginType& 
Plm_image_header::GetOrigin () const
{
    return m_origin;
}

const SpacingType& 
Plm_image_header::GetSpacing () const
{
    return m_spacing;
}

const RegionType& 
Plm_image_header::GetRegion () const 
{
    return m_region;
}

const DirectionType& 
Plm_image_header::GetDirection () const 
{
    return m_direction;
}

const SizeType& 
Plm_image_header::GetSize (void) const
{
    return m_region.GetSize ();
}

void
Plm_image_header::get_volume_header (Volume_header *vh) const
{
    this->get_origin (vh->get_origin());
    this->get_dim (vh->get_dim());
    this->get_spacing (vh->get_spacing());
    this->get_direction_cosines (vh->get_direction_cosines());
}

void 
Plm_image_header::get_origin (float origin[3]) const
{
    for (unsigned int d = 0; d < 3; d++) {
	origin[d] = m_origin[d];
    }
}

void 
Plm_image_header::get_spacing (float spacing[3]) const
{
    for (unsigned int d = 0; d < 3; d++) {
	spacing[d] = m_spacing[d];
    }
}

void 
Plm_image_header::get_dim (plm_long dim[3]) const
{
    RegionType::SizeType itk_size = m_region.GetSize ();
    for (unsigned int d = 0; d < 3; d++) {
	dim[d] = itk_size[d];
    }
}

void 
Plm_image_header::get_direction_cosines (float direction_cosines[9]) const
{
    dc_from_itk_direction (direction_cosines, &m_direction);
}

/* -----------------------------------------------------------------------
   Algorithms
   ----------------------------------------------------------------------- */
/* static */ void 
Plm_image_header::clone (Plm_image_header *dest, const Plm_image_header *src)
{
    dest->m_origin = src->m_origin;
    dest->m_spacing = src->m_spacing;
    dest->m_region = src->m_region;
    dest->m_direction = src->m_direction;
}

void 
Plm_image_header::expand_to_contain (
    const FloatPoint3DType& position)
{
    /* Compute index for this position */
    FloatPoint3DType idx = this->get_index (position);

    /* Get the step & proj matrices */
    /* GCS FIX: This is inefficient, already computed in get_index() */
    float spacing[3], step[9], proj[9];
    Direction_cosines dc (m_direction);
    this->get_spacing (spacing);
    compute_direction_matrices (step, proj, dc, spacing);

    RegionType::SizeType itk_size = m_region.GetSize();

    /* Expand the volume to contain the point */
    for (int d1 = 0; d1 < 3; d1++) {
        if (idx[d1] < 0) {
            float extra = (float) floor ((double) idx[d1]);
            for (int d2 = 0; d2 < 3; d2++) {
                m_origin[d2] += extra * step[d2*3+d1];
            }
            itk_size[d1] += (int) -extra;
        }
        else if (idx[d1] > itk_size[d1] - 1) {
            itk_size[d1] = (int) floor ((double) idx[d1]) + 1;
        }
    }
    m_region.SetSize (itk_size);
}

void 
Plm_image_header::set_geometry_to_contain (
    const Plm_image_header& reference_pih,
    const Plm_image_header& compare_pih)
{
    /* Initialize to reference image */
    this->set (reference_pih);

    /* Expand to contain all eight corners of compare image */
    FloatPoint3DType pos;
    float idx[3];
    idx[0] = 0;
    idx[1] = 0;
    idx[2] = 0;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);

    idx[0] = 0;
    idx[1] = 0;
    idx[2] = compare_pih.dim(2) - 1;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);

    idx[0] = 0;
    idx[1] = compare_pih.dim(1) - 1;
    idx[2] = 0;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);

    idx[0] = 0;
    idx[1] = compare_pih.dim(1) - 1;
    idx[2] = compare_pih.dim(2) - 1;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);

    idx[0] = compare_pih.dim(0) - 1;
    idx[1] = 0;
    idx[2] = 0;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);

    idx[0] = compare_pih.dim(0) - 1;
    idx[1] = 0;
    idx[2] = compare_pih.dim(2) - 1;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);

    idx[0] = compare_pih.dim(0) - 1;
    idx[1] = compare_pih.dim(1) - 1;
    idx[2] = 0;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);

    idx[0] = compare_pih.dim(0) - 1;
    idx[1] = compare_pih.dim(1) - 1;
    idx[2] = compare_pih.dim(2) - 1;
    pos = compare_pih.get_position (idx);
    this->expand_to_contain (pos);
}

void
Plm_image_header::print (void) const
{
    RegionType::SizeType itk_size;
    itk_size = m_region.GetSize ();
    float dc[9];
    this->get_direction_cosines (dc);

    lprintf ("Origin =");
    for (unsigned int d = 0; d < 3; d++) {
	lprintf (" %g", m_origin[d]);
    }
    lprintf ("\nSize =");
    for (unsigned int d = 0; d < 3; d++) {
	lprintf (" %lu", itk_size[d]);
    }
    lprintf ("\nSpacing =");
    for (unsigned int d = 0; d < 3; d++) {
	lprintf (" %g", m_spacing[d]);
    }
    lprintf ("\nDirection =");
    for (unsigned int d1 = 0; d1 < 3; d1++) {
	for (unsigned int d2 = 0; d2 < 3; d2++) {
	    lprintf (" %g", dc[d1*3+d2]);
	}
    }

    lprintf ("\n");
}

FloatPoint3DType
Plm_image_header::get_index (const FloatPoint3DType& pos) const
{
    FloatPoint3DType idx;
    FloatPoint3DType tmp;

    float spacing[3], step[9], proj[9];
    Direction_cosines dc (m_direction);
    this->get_spacing (spacing);

    compute_direction_matrices (step, proj, dc, spacing);

    for (int d1 = 0; d1 < 3; d1++) {
        tmp[d1] = pos[d1] - m_origin[d1];
        idx[d1] = 0;
    }
    for (int d1 = 0; d1 < 3; d1++) {
        for (int d2 = 0; d2 < 3; d2++) {
            idx[d1] += tmp[d2] * proj[d1*3+d2];
        }
    }

    return idx;
}

FloatPoint3DType
Plm_image_header::get_position (const float index[3]) const
{
    FloatPoint3DType pos;

    for (int d = 0; d < 3; d++) {
        pos[d] = m_origin[d];
    }
    for (int d = 0; d < 3; d++) {
        for (int dc = 0; dc < 3; dc++) {
            pos[dc] += m_spacing[d] * index[d] * m_direction[d][dc];
        }
    }
    return pos;
}

void 
Plm_image_header::get_image_center (float center[3]) const
{
    int d;
    for (d = 0; d < 3; d++) {
	center[d] = this->m_origin[d] 
	    + this->m_spacing[d] * (this->dim(d) - 1) / 2;
    }
}

plm_long
Plm_image_header::get_num_voxels (void) const
{
    return this->dim(0) * this->dim(1) * this->dim(2);
}

void 
Plm_image_header::get_image_extent (float extent[3]) const
{
    int d;
    for (d = 0; d < 3; d++) {
	extent[d] = this->m_spacing[d] * (this->dim(d) - 1);
    }
}

/* Return true if the two headers are the same, within tolerance */
bool
Plm_image_header::compare (Plm_image_header *pli1, Plm_image_header *pli2,
    float threshold)
{
    int d;
    for (d = 0; d < 3; d++) {
        if (fabs (pli1->m_origin[d] - pli2->m_origin[d]) > threshold) {
           return false;
        }
        if (fabs (pli1->m_spacing[d] - pli2->m_spacing[d]) > threshold) {
            return false;
        }
        if (pli1->dim(d) != pli2->dim(d)) {
            return false;
        }
    }

    /* GCS FIX: check direction cosines */

    return true;
}

/* Explicit instantiations */
template PLMBASE_API Plm_image_header::Plm_image_header (CharImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (UCharImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (ShortImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (UShortImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (Int32ImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (UInt32ImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (FloatImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (DoubleImageType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (DeformationFieldType::Pointer image);
template PLMBASE_API Plm_image_header::Plm_image_header (UCharVecImageType::Pointer image);

template PLMBASE_API void Plm_image_header::set_from_itk_image (const CharImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UCharImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const ShortImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UShortImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const Int32ImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UInt32ImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const FloatImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const DoubleImageType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const DeformationFieldType::Pointer& image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UCharVecImageType::Pointer& image);

template PLMBASE_API void Plm_image_header::set_from_itk_image (const CharImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UCharImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const ShortImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UShortImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const Int32ImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UInt32ImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const FloatImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const DoubleImageType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const DeformationFieldType* image);
template PLMBASE_API void Plm_image_header::set_from_itk_image (const UCharVecImageType* image);
