/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "itkImageRegionIterator.h"

#include "bspline_xform.h"
#include "itk_directions.h"
#include "itk_image.h"
#include "itk_volume_header.h"
#include "plm_int.h"

/* -----------------------------------------------------------------------
   prototypes
   ----------------------------------------------------------------------- */
static void
itk_direction_from_gpuit (
    DirectionType* itk_direction,
    float gpuit_direction_cosines[9]
);

static void
itk_direction_identity (DirectionType* itk_direction);

/* -----------------------------------------------------------------------
   functions
   ----------------------------------------------------------------------- */
void
Itk_volume_header::set_origin (float origin[3])
{
    for (unsigned int d = 0; d < 3; d++) {
	this->m_origin[d] = origin[d];
    }
}

void
Itk_volume_header::set_spacing (float spacing[3])
{
    for (unsigned int d = 0; d < 3; d++) {
	this->m_spacing[d] = spacing[d];
    }
}

void
Itk_volume_header::set_dim (plm_long dim[3])
{
    SizeType itk_size;
    IndexType itk_index;
    for (unsigned int d = 0; d < 3; d++) {
	itk_index[d] = 0;
	itk_size[d] = dim[d];
    }
    m_region.SetSize (itk_size);
    m_region.SetIndex (itk_index);
}

void
Itk_volume_header::set_from_gpuit (
    float gpuit_origin[3],
    float gpuit_spacing[3],
    plm_long gpuit_dim[3],
    float gpuit_direction_cosines[9])
{
    SizeType itk_size;
    IndexType itk_index;

    for (unsigned int d1 = 0; d1 < 3; d1++) {
	m_origin[d1] = gpuit_origin[d1];
	m_spacing[d1] = gpuit_spacing[d1];
	itk_index[d1] = 0;
	itk_size[d1] = gpuit_dim[d1];
    }
    if (gpuit_direction_cosines) {
	itk_direction_from_gpuit (&m_direction, gpuit_direction_cosines);
    } else {
	itk_direction_identity (&m_direction);
    }
    m_region.SetSize (itk_size);
    m_region.SetIndex (itk_index);
}

void
Itk_volume_header::set_from_gpuit_bspline (Bspline_xform *bxf)
{
    this->set_from_gpuit (
	bxf->img_origin,
	bxf->img_spacing,
	bxf->img_dim,
	0);
}

void 
Itk_volume_header::get_origin (float origin[3])
{
    for (unsigned int d = 0; d < 3; d++) {
	origin[d] = m_origin[d];
    }
}

void 
Itk_volume_header::get_spacing (float spacing[3])
{
    for (unsigned int d = 0; d < 3; d++) {
	spacing[d] = m_spacing[d];
    }
}

void 
Itk_volume_header::get_dim (plm_long dim[3])
{
    SizeType itk_size = m_region.GetSize ();
    for (unsigned int d = 0; d < 3; d++) {
	dim[d] = itk_size[d];
    }
}

void 
Itk_volume_header::get_direction_cosines (float direction_cosines[9])
{
    dc_from_itk_direction (direction_cosines, &m_direction);
}

void
Itk_volume_header::print (void) const
{
    SizeType itk_size;
    itk_size = m_region.GetSize ();

    printf ("Origin =");
    for (unsigned int d = 0; d < 3; d++) {
	printf (" %g", m_origin[d]);
    }
    printf ("\nSize =");
    for (unsigned int d = 0; d < 3; d++) {
	printf (" %lu", itk_size[d]);
    }
    printf ("\nSpacing =");
    for (unsigned int d = 0; d < 3; d++) {
	printf (" %g", m_spacing[d]);
    }
    printf ("\nDirection =");
    for (unsigned int d1 = 0; d1 < 3; d1++) {
	for (unsigned int d2 = 0; d2 < 3; d2++) {
	    printf (" %g", m_direction[d1][d2]);
	}
    }
    printf ("\n");
}

void 
Itk_volume_header::get_image_center (float center[3])
{
    for (int d1 = 0; d1 < 3; d1++) {
	center[d1] = this->m_origin[d1];
    }
    for (int d1 = 0; d1 < 3; d1++) {
        float v = this->m_spacing[d1] * (this->Size(d1) - 1.) / 2;
        for (int d2 = 0; d2 < 3; d2++) {
            center[d2] += v * m_direction[d2][d1];
        }
    }
}

/* Return 1 if the two headers are the same */
int
Itk_volume_header::compare (Itk_volume_header *pli1, Itk_volume_header *pli2)
{
    int d;
    for (d = 0; d < 3; d++) {
	if (pli1->m_origin[d] != pli2->m_origin[d]) return 0;
	if (pli1->m_spacing[d] != pli2->m_spacing[d]) return 0;
	if (pli1->Size(d) != pli2->Size(d)) return 0;
    }

    /* GCS FIX: check direction cosines */

    return 1;
}

/* -----------------------------------------------------------------------
   global functions
   ----------------------------------------------------------------------- */
static void
itk_direction_from_gpuit (
    DirectionType* itk_direction,
    float gpuit_direction_cosines[9]
)
{
    for (unsigned int d1 = 0; d1 < 3; d1++) {
	for (unsigned int d2 = 0; d2 < 3; d2++) {
	    (*itk_direction)[d1][d2] = gpuit_direction_cosines[d1*3+d2];
	}
    }
}

static void
itk_direction_identity (DirectionType* itk_direction)
{
    for (unsigned int d1 = 0; d1 < 3; d1++) {
	for (unsigned int d2 = 0; d2 < 3; d2++) {
	    (*itk_direction)[d1][d2] = (float) (d1 == d2);
	}
    }
}
