/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkContourExtractor2DImageFilter.h"
#include "itkImage.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkAndConstantToImageFilter.h"
#include "itkImageSliceConstIteratorWithIndex.h"

#include "cxt_extract.h"
#include "itk_image.h"
#include "itk_image_origin.h"
#include "itk_image_type.h"
#include "rtss.h"
#include "rtss_contour.h"
#include "rtss_roi.h"
#include "slice_extract.h"
#include "ss_img_extract.h"
#include "volume.h"

#if defined (commentout)
static bool
debug_uchar_slice (UCharImage2DType::Pointer uchar_slice)
{
    typedef itk::ImageRegionIterator< UCharImage2DType > IteratorType;
    IteratorType it1 (uchar_slice, uchar_slice->GetBufferedRegion());

    it1.GoToBegin();
    while (!it1.IsAtEnd()) {
	unsigned char p = it1.Get ();
	if (p != 0) {
	    printf ("Got pixel: %d\n", p);
	    return true;
	}
	++it1;
    }
    return false;
}

static bool
debug_uint32_slice (UInt32Image2DType::Pointer slice, uint32_t val)
{
    typedef itk::ImageRegionIterator< UInt32Image2DType > IteratorType;
    IteratorType it1 (slice, slice->GetBufferedRegion());

    it1.GoToBegin();
    while (!it1.IsAtEnd()) {
	uint32_t p = it1.Get ();
	if (p &= val) {
	    printf ("Got pixel: %d\n", p);
	    return true;
	}
	++it1;
    }
    return false;
}
#endif

static void
run_marching_squares (
    Rtss_roi *curr_structure, 
    const UCharImage2DType::Pointer uchar_slice,
    unsigned int slice_no,
    const OriginType& origin,
    const SpacingType& spacing,
    const DirectionType& direction
)
{
    typedef itk::ContourExtractor2DImageFilter<UCharImage2DType> 
        ContourFilterType;
    typedef ContourFilterType::VertexType VertexType;

    /* Run marching squares on the slice */
    /* Note: due to an ITK bug, the contour filter cannot be 
       "re-run" with different inputs.  Instead we should 
       delete and create a new one for each contour. */
    ContourFilterType::Pointer contour_filter 
	= ContourFilterType::New();
    contour_filter->SetInput (uchar_slice);
    contour_filter->SetContourValue (0.5);
    try {
	contour_filter->Update();
    }
    catch (itk::ExceptionObject &err) {
	std::cout << "Exception during marching squares." 
            << std::endl;
	std::cout << err << std::endl;
	exit (1);
    }

    /* Add marching squares output to cxt.  Loop through 
       contours on this slice... */
    for (unsigned int i = 0; 
	 i < contour_filter->GetNumberOfOutputs(); i++)
    {
	ContourFilterType::VertexListConstPointer vertices 
	    = contour_filter->GetOutput(i)->GetVertexList();
	Rtss_contour *curr_polyline = curr_structure->add_polyline ();

	curr_polyline->num_vertices = vertices->Size();
	curr_polyline->x = (float*) 
	    malloc (vertices->Size() * sizeof(float));
	curr_polyline->y = (float*) 
	    malloc (vertices->Size() * sizeof(float));
	curr_polyline->z = (float*) 
	    malloc (vertices->Size() * sizeof(float));
	curr_polyline->slice_no = slice_no;
	/* Loop through vertices of this output contour */
	for (unsigned int k = 0; k < vertices->Size(); k++) {
	    const VertexType& vertex = vertices->ElementAt (k);
	    curr_polyline->x[k] = origin[0] 
                + direction[0][0] * vertex[0] * spacing[0] 
                + direction[0][1] * vertex[1] * spacing[1] 
                + direction[0][2] * slice_no * spacing[2];
	    curr_polyline->y[k] = origin[1] 
                + direction[1][0] * vertex[0] * spacing[0] 
                + direction[1][1] * vertex[1] * spacing[1] 
                + direction[1][2] * slice_no * spacing[2];
	    curr_polyline->z[k] = origin[2] 
                + direction[2][0] * vertex[0] * spacing[0] 
                + direction[2][1] * vertex[1] * spacing[1] 
                + direction[2][2] * slice_no * spacing[2];
	}
    }
}

/* This function only fills in the polylines.  Structure names, id, etc. 
   will be assigned to default values if they are not already set. 

   By default, 32 structures will be searched.  If num_structs > 0, 
   only structures with bits between 0 and num_structs-1 will be processed.

   In the case that we are doing a cxt->mha, then warp, then mha->cxt, 
   the cxt->mha step will mark the bit values in the cxt.  In this case, 
   the caller should set check_cxt_bits, so that this function will 
   look at the "bit" field in each cxt structure to see which bit
   should be processed.
*/
template<class T>
void
cxt_extract (
    Rtss *cxt, 
    T image, 
    int num_structs, 
    bool check_cxt_bits
)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ContourExtractor2DImageFilter<UCharImage2DType> 
	    ContourFilterType;
    typedef ContourFilterType::VertexType VertexType;
    typedef itk::ImageSliceConstIteratorWithIndex<ImageType> IteratorType;
    typedef itk::AndConstantToImageFilter<UInt32Image2DType, 
	    uint32_t, UCharImage2DType> AndFilterType;

    IteratorType slice_it (image, image->GetLargestPossibleRegion());

    typename AndFilterType::Pointer and_filter 
	    = AndFilterType::New();

    if (num_structs < 0) {
	num_structs = 32;     /* Max 32 structs in 32-bit xormap */
    }

    /* If structure names are unknown, name them, and give them 
       arbitrary id numbers */
    for (int j = cxt->num_structures; j < num_structs; j++) {
	/* Get a free id */
	int k = 1;
	while (cxt->find_structure_by_id (k)) k++;
	/* Add the structure */
	cxt->add_structure (
            std::string ("Unknown structure"), std::string(), k);
    }

    /* Loop through slices */
    int slice_no = 0;
    slice_it.SetFirstDirection(0);
    slice_it.SetSecondDirection(1);
    while (!slice_it.IsAtEnd())
    {
	typename ImageType::IndexType idx;
	UInt32Image2DType::Pointer uint32_slice;
	UCharImage2DType::Pointer uchar_slice;

	/* Make a copy of the current slice */
	idx = slice_it.GetIndex();
	uint32_slice = slice_extract (image, idx[2]);

	and_filter->SetInput (uint32_slice);
	for (int j = 0; j < num_structs; j++)
	{
	    /* And the current slice with the mask for this structure */
	    Rtss_roi *curr_structure = cxt->slist[j];

	    /* Choose the bit value for this structure */
	    uint32_t val;
	    if (check_cxt_bits) {
		if (curr_structure->bit == -1) {
		    /* Skip if this structure is not represented in image */
		    continue;
		} else {
		    val = (1 << curr_structure->bit);
		}
	    } else {
		val = (1 << j);
	    }

	    /* Mask the slice with this bit */
	    and_filter->SetConstant (val);
	    try {
		and_filter->Update ();
	    }
	    catch (itk::ExceptionObject &err) {
		std::cout << "Exception during and operation." << std::endl;
		std::cout << err << std::endl;
		exit (1);
	    }
	    uchar_slice = and_filter->GetOutput ();

	    run_marching_squares (curr_structure, uchar_slice, slice_no,
		itk_image_origin (image), image->GetSpacing(), 
                image->GetDirection());

	}
	slice_it.NextSlice();
	slice_no ++;
    }
}

template<>
void
cxt_extract (
    Rtss *cxt, 
    UCharVecImageType::Pointer image, 
    int num_structs, 
    bool check_cxt_bits
)
{
    typedef itk::ContourExtractor2DImageFilter<UCharImage2DType> 
	ContourFilterType;
    typedef ContourFilterType::VertexType VertexType;
    typedef itk::ImageSliceConstIteratorWithIndex<UCharVecImageType> 
	IteratorType;
    typedef itk::AndConstantToImageFilter<
	UCharImage2DType, unsigned char, UCharImage2DType> AndFilterType;

    AndFilterType::Pointer and_filter 
	= AndFilterType::New();

    if (num_structs < 0) {
	num_structs = image->GetVectorLength() * 8;
    }

    /* If structure names are unknown, name them, and give them 
       arbitrary id numbers */
    for (int j = cxt->num_structures; j < num_structs; j++) {
	/* Get a free id */
	int k = 1;
	while (cxt->find_structure_by_id (k)) k++;
	/* Add the structure */
	cxt->add_structure (
            std::string ("Unknown structure"), std::string(), k);
    }


    /* Loop through slices */
    int num_slices = image->GetLargestPossibleRegion().GetSize(2);
    unsigned int num_uchar = image->GetVectorLength();

    for (int slice_no = 0; slice_no < num_slices; slice_no++) {

	/* Make a copy of the current slice */
	UCharVecImage2DType::Pointer ucharvec_slice 
	    = slice_extract (image, slice_no);

	/* Loop through uchars for this slice */
	for (unsigned int uchar_no = 0; uchar_no < num_uchar; uchar_no++) {
	    /* Extract a single uchar slice from uchar_vec slice */
	    UCharImage2DType::Pointer uchar_slice = 
		ss_img_extract_uchar (ucharvec_slice, uchar_no);
	    and_filter->SetInput (uchar_slice);

	    /* Look for structures which use this uchar */
	    for (int j = 0; j < num_structs; j++) {
		Rtss_roi *curr_structure = cxt->slist[j];
		int bit;
		unsigned char mask;

		/* Choose the bit value for this structure */
		if (check_cxt_bits) {
		    bit = curr_structure->bit;
		} else {
		    bit = j;
		}

		/* Make a mask value */
		if (bit < (int) (uchar_no*8) || bit > (int) (uchar_no*8+7)) {
		    /* Skip structures not in this uchar */
		    /* Note: this also skips empty structures, which 
		       have a bit value of -1 */
		    continue;
		}
		bit -= uchar_no*8;
		mask = (1 << bit);

		/* And the current slice with the mask for this structure */
		and_filter->SetConstant (mask);
		try {
		    and_filter->Update ();
		}
		catch (itk::ExceptionObject &err) {
		    std::cout << "Exception during and operation." << std::endl;
		    std::cout << err << std::endl;
		    exit (1);
		}
		uchar_slice = and_filter->GetOutput ();

		run_marching_squares (curr_structure, uchar_slice, slice_no, 
		itk_image_origin (image), image->GetSpacing(), 
                    image->GetDirection());
	    }
	}
    }
}

/* Explicit instantiations */
template PLMBASE_API void cxt_extract (Rtss *cxt, UInt32ImageType::Pointer image, int num_structs, bool check_cxt_bits);
