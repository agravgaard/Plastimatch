/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "print_and_exit.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_int.h"
#include "plm_math.h"
#include "rasterize_slice.h"
#include "rasterizer.h"
#include "rtss.h"
#include "rtss_contour.h"
#include "rtss_roi.h"
#include "volume.h"

Rasterizer::Rasterizer ()
{
    want_prefix_imgs = false;
    want_labelmap = false;
    want_ss_img = false;

    acc_img = 0;
    uchar_vol = 0;
    labelmap_vol = 0;
    m_ss_img = 0;
    m_use_ss_img_vec = true;
    curr_struct_no = 0;
    curr_bit = 0;
    xor_overlapping = false;
}

Rasterizer::~Rasterizer (void)
{
    if (this->uchar_vol) {
	delete this->uchar_vol;
    }
    if (this->labelmap_vol) {
	delete this->labelmap_vol;
    }
    if (this->m_ss_img) {
	delete this->m_ss_img;
    }
    free (this->acc_img);
}

void
Rasterizer::init (
    Rtss *cxt,            /* Input */
    Plm_image_header *pih,             /* Input */
    bool want_prefix_imgs,             /* Input */
    bool want_labelmap,                /* Input */
    bool want_ss_img,                  /* Input */
    bool use_ss_img_vec,               /* Input */
    bool xor_overlapping               /* Input */
)
{
    int slice_voxels;

    pih->get_origin (this->origin);
    pih->get_spacing (this->spacing);
    pih->get_dim (this->dim);

    slice_voxels = this->dim[0] * this->dim[1];

    this->want_prefix_imgs = want_prefix_imgs;
    this->want_labelmap = want_labelmap;
    this->want_ss_img = want_ss_img;
    this->xor_overlapping = xor_overlapping;
    this->m_use_ss_img_vec = use_ss_img_vec;

    this->acc_img = (unsigned char*) malloc (
	slice_voxels * sizeof(unsigned char));

    /* Create output volume for mask image.  This is reused for each 
       structure */
    this->uchar_vol = new Volume (this->dim, this->origin, 
	this->spacing, 0, PT_UCHAR, 1);
    if (this->uchar_vol == 0) {
	print_and_exit ("ERROR: failed in allocating the volume");
    }

    /* Create output volume for labelmap */
    this->labelmap_vol = 0;
    if (want_labelmap) {
	this->labelmap_vol = new Volume (this->dim, 
	    this->origin, this->spacing, 0, PT_UINT32, 1);
	if (this->labelmap_vol == 0) {
	    print_and_exit ("ERROR: failed in allocating the volume");
	}
    }

    /* Create output volume for ss_img */
    if (want_ss_img) {
        this->m_ss_img = new Plm_image;
        if (use_ss_img_vec) {
            UCharVecImageType::Pointer ss_img = UCharVecImageType::New ();
            itk_image_set_header (ss_img, pih);
            int num_uchar = 1 + (cxt->num_structures-1) / 8;
            if (num_uchar < 2) num_uchar = 2;
            ss_img->SetVectorLength (num_uchar);
            ss_img->Allocate ();

            /* Zero out the image */
            itk::VariableLengthVector< unsigned char > ZeroPixel (num_uchar);
            ZeroPixel.Fill (itk::NumericTraits< unsigned char >::Zero);
            ss_img->FillBuffer (ZeroPixel);

            this->m_ss_img->set_itk (ss_img);
        }
        else {
            Volume *vol = new Volume (this->dim, 
                this->origin, this->spacing, 0, PT_UINT32, 1);
            if (vol == 0) {
                print_and_exit ("ERROR: failed allocating ss_img volume");
            }
            this->m_ss_img->set_volume (vol);
        }
    }

    /* Initialize to start with first structure */
    this->curr_struct_no = 0;
    this->curr_bit = 0;
}

/* Return true if an image was processed */
bool
Rasterizer::process_next (
    Rtss *cxt                          /* In/out */
)
{
    Rtss_roi* curr_structure;
    unsigned char* uchar_img = (unsigned char*) this->uchar_vol->img;
    size_t slice_voxels;

    /* If done, return false */
    if (this->curr_struct_no >= cxt->num_structures) {
	this->curr_struct_no = cxt->num_structures + 1;
	return false;
    }

    /* If not using ss_img_vec, stop at 32 structures */
    if (!this->m_use_ss_img_vec && this->curr_struct_no >= 32) {
        printf ("Warning: too many structures.  Dropping some...\n");
	this->curr_struct_no = cxt->num_structures + 1;
	return false;
    }

    curr_structure = cxt->slist[this->curr_struct_no];
    slice_voxels = this->dim[0] * this->dim[1];

    memset (uchar_img, 0, this->dim[0] * this->dim[1] 
	* this->dim[2] * sizeof(unsigned char));

    /* Loop through polylines in this structure */
    for (size_t i = 0; i < curr_structure->num_contours; i++) {
	Rtss_contour* curr_contour;
	unsigned char* uchar_slice;
	plm_long slice_no;

	curr_contour = curr_structure->pslist[i];
	if (curr_contour->num_vertices == 0) {
	    continue;
	}
	slice_no = ROUND_PLM_LONG(
            (curr_contour->z[0] - this->origin[2]) / this->spacing[2]);
	if (slice_no < 0 || slice_no >= this->dim[2]) {
	    continue;
	}

	/* Render contour to binary */
	memset (this->acc_img, 0, slice_voxels * sizeof(unsigned char));
	rasterize_slice (
	    this->acc_img, 
	    this->dim, 
	    this->spacing, 
	    this->origin,
	    curr_contour->num_vertices, 
	    curr_contour->x, 
	    curr_contour->y);

	/* Copy from acc_img into mask image */
	if (this->want_prefix_imgs) {
	    uchar_slice = &uchar_img[slice_no * slice_voxels];
	    for (size_t k = 0; k < slice_voxels; k++) {
		uchar_slice[k] ^= this->acc_img[k];
	    }
	}

	/* Copy from acc_img into labelmask image */
	if (this->want_labelmap) {
	    uint32_t* labelmap_img;
	    uint32_t* uint32_slice;
	    labelmap_img = (uint32_t*) this->labelmap_vol->img;
	    uint32_slice = &labelmap_img[slice_no * slice_voxels];
	    for (size_t k = 0; k < slice_voxels; k++) {
		if (this->acc_img[k]) {
		    uint32_slice[k] = this->curr_bit + 1;
		}
	    }
	}

        /* Copy from acc_img into ss_img */
	if (this->want_ss_img) {

            if (this->m_use_ss_img_vec) {
                UCharVecImageType::Pointer ss_img = 
                    this->m_ss_img->m_itk_uchar_vec;

                /* GCS FIX: This code is replicated in ss_img_extract */
                unsigned int uchar_no = this->curr_bit / 8;
                unsigned int bit_no = this->curr_bit % 8;
                unsigned char bit_mask = 1 << bit_no;
                if (uchar_no > ss_img->GetVectorLength()) {
                    print_and_exit (
                        "Error: bit %d was requested from image of %d bits\n", 
                        this->curr_bit, 
                        ss_img->GetVectorLength() * 8);
                }
                /* GCS FIX: This is inefficient, due to undesirable construct 
                   and destruct of itk::VariableLengthVector of each pixel */
                UCharVecImageType::IndexType idx = {{0, 0, slice_no}};
                size_t k = 0;
                for (idx.m_Index[1] = 0; 
                     idx.m_Index[1] < this->dim[1]; 
                     idx.m_Index[1]++) {
                    for (idx.m_Index[0] = 0; 
                         idx.m_Index[0] < this->dim[0]; 
                         idx.m_Index[0]++) {
                        if (this->acc_img[k]) {
                            itk::VariableLengthVector<unsigned char> v 
                                = ss_img->GetPixel (idx);
                            if (this->xor_overlapping) {
                                v[uchar_no] ^= bit_mask;
                            } else {
                                v[uchar_no] |= bit_mask;
                            }
                            ss_img->SetPixel (idx, v);
                        }
                        k++;
                    }
                }
            }
            else {
                uint32_t* ss_img = 0;
                uint32_t* uint32_slice;
                uint32_t bit_mask = 1 << this->curr_bit;
                ss_img = (uint32_t*) this->m_ss_img->get_vol()->img;
                uint32_slice = &ss_img[slice_no * slice_voxels];
                for (size_t k = 0; k < slice_voxels; k++) {
                    if (this->acc_img[k]) {
                        if (this->xor_overlapping) {
                            uint32_slice[k] ^= bit_mask;
                        } else {
                            uint32_slice[k] |= bit_mask;
                        }
                    }
                }
            }

	}
    }

    this->curr_struct_no ++;
    if (curr_structure->num_contours > 0) {
	curr_structure->bit = this->curr_bit;
	this->curr_bit ++;
    }

    return true;
}

const char*
Rasterizer::current_name (
    Rtss *cxt
)
{
    if (this->curr_struct_no < cxt->num_structures + 1) {
	Rtss_roi *curr_structure;
	curr_structure = cxt->slist[this->curr_struct_no-1];
	return curr_structure->name.c_str();
    } else {
	return "";
    }
}

void
Rasterizer::rasterize (
    Rtss *cxt,            /* Input */
    Plm_image_header *pih,             /* Input */
    bool want_prefix_imgs,             /* Input */
    bool want_labelmap,                /* Input */
    bool want_ss_img,                  /* Input */
    bool use_ss_img_vec,               /* Input */
    bool xor_overlapping               /* Input */
)
{
    this->init (cxt, pih, false, true, true, use_ss_img_vec, xor_overlapping);
    while (this->process_next (cxt)) {}
}
