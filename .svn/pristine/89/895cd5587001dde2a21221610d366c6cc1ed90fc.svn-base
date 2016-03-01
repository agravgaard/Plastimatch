/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include "itkImageRegionIterator.h"
#include "itkVariableLengthVector.h"

#include "itk_image_type.h"
#include "plm_int.h"
#include "ss_img_stats.h"

void
ss_img_stats (
    UCharVecImageType::Pointer img
)
{
    UCharVecImageType::RegionType rg = img->GetLargestPossibleRegion ();

    typedef itk::ImageRegionIterator< UCharVecImageType > UCharVecIteratorType;
    UCharVecIteratorType it (img, rg);

    int vector_length = img->GetVectorLength();

    printf ("SS_IMAGE: At most %d structures\n", vector_length * 8);
    uint32_t *hist = new uint32_t[vector_length * 8];

    for (int i = 0; i < vector_length; i++) {
	for (int j = 0; j < 8; j++) {
	    hist[i*8+j] = 0;
	}
    }

    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
	itk::VariableLengthVector< unsigned char > v = it.Get();
	for (int i = 0; i < vector_length; i++) {
	    unsigned char c = v[i];
	    for (int j = 0; j < 8; j++) {
		if (c & (1 << j)) {
		    hist[i*8+j] ++;
		}
	    }
	}
    }

    for (int i = 0; i < vector_length; i++) {
	for (int j = 0; j < 8; j++) {
	    printf ("S %4d  NVOX %10d\n", i*8+j, hist[i*8+j]);
	}
    }
    delete[] hist;
}
