/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "itkImageRegionIterator.h"

#include "itk_image_scale.h"

template<class T> 
void
itk_image_accumulate (
    T img_accumulate,
    double weight,
    T img)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ImageRegionIterator< ImageType > IteratorType;
    typename ImageType::RegionType rg = img->GetLargestPossibleRegion ();
    IteratorType it_a (img_accumulate, rg);
    IteratorType it_b (img, rg);

    for (it_a.GoToBegin(), it_b.GoToBegin(); !it_a.IsAtEnd(); ++it_a, ++it_b) {
        it_a.Set ((double) it_a.Get() + weight * (double) it_b.Get());
    }
}

/* Explicit instantiations */
template PLMBASE_API void itk_image_accumulate (FloatImageType::Pointer, double, FloatImageType::Pointer);
