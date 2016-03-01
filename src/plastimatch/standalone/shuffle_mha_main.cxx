/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* Correct mha files which have incorrect patient orientations */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "itkImageSliceConstIteratorWithIndex.h"
#include "itkImageSliceIteratorWithIndex.h"

#include "itk_image.h"
#include "itk_image_load.h"
#include "itk_image_save.h"
#include "itk_image_type.h"

int 
main (int argc, char* argv[])
{
    if (argc != 3 && argc != 4) {
	printf ("Usage: %s shuffle infile [outfile]\n", argv[0]);
	printf ("If you don't specify an outfile, the infile will be overwritten.\n");
	printf ("Shuffle value hard coded to flipping Z axis\n");
	exit (1);
    }

    char* fn_out;
    if (argc == 4) {
	fn_out = argv[3];
    } else {
	fn_out = argv[2];
    }

    /* Load input */
    FloatImageType::Pointer v_in = itk_image_load_float(argv[2], 0);

    /* Allocate memory for output */
    FloatImageType::Pointer v_out = FloatImageType::New();
    v_out->SetRegions (v_in->GetLargestPossibleRegion());
    v_out->SetOrigin (v_in->GetOrigin());
    v_out->SetSpacing (v_in->GetSpacing());
    v_out->Allocate();

    /* Set up slice iterators */
    typedef itk::ImageSliceConstIteratorWithIndex<FloatImageType> ConstIteratorType;
    typedef itk::ImageSliceIteratorWithIndex<FloatImageType> IteratorType;
    ConstIteratorType it_in (v_in, v_in->GetLargestPossibleRegion());
    IteratorType it_out (v_out, v_out->GetLargestPossibleRegion());
    RegionType rgn_out = v_out->GetLargestPossibleRegion();
    it_in.SetFirstDirection(0);
    it_in.SetSecondDirection(1);
    it_out.SetFirstDirection(0);
    it_out.SetSecondDirection(1);

    /* Still setting up slice iterators.  The ITK class is missing 
	functionality of reverse iteration for each direction 
	independently. */
    FloatImageType::IndexType idx_out;
    idx_out[0] = 0;
    idx_out[1] = 0;
    idx_out[2] = rgn_out.GetSize()[2] - 1;

    it_in.GoToBegin ();
    it_out.SetIndex (idx_out);

    /* For each slice of input, copy to output */
    while (!it_in.IsAtEnd()) {
	while (!it_in.IsAtEndOfSlice()) {
	    while (!it_in.IsAtEndOfLine()) {
		it_out.Set (it_in.Get());
		++it_in;
		++it_out;
	    }
	    it_in.NextLine ();
	    it_out.NextLine ();
	}
	it_in.NextSlice ();
	idx_out[0] = 0;
	idx_out[1] = 0;
	idx_out[2] --;
	it_out.SetIndex (idx_out);
    }

    itk_image_save_float (v_out, fn_out);

    return 0;
}
