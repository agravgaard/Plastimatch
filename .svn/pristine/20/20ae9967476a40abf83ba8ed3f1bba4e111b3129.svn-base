/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "itkConvolutionImageFilter.h"

#include "itk_image_conv.h"
#include "itk_image_stats.h"

/* ITK convolution is broken. I must be "holding it wrong."
   GCS 2014-08-27 */

typedef itk::Image<float, 2> ImageType;

void CreateKernel(ImageType::Pointer kernel, unsigned int width)
{
    ImageType::IndexType start;
    start.Fill(0);
 
    ImageType::SizeType size;
    size.Fill(width);
 
    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);
 
    kernel->SetRegions(region);
    kernel->Allocate();
 
    itk::ImageRegionIterator<ImageType> imageIterator(kernel, region);
 
    while(!imageIterator.IsAtEnd())
    {
        imageIterator.Set(1);
 
        ++imageIterator;
    }
}
 
void garbage ()
{
    unsigned int width = 3;
    ImageType::Pointer image = ImageType::New();
    ImageType::Pointer kernel = ImageType::New();
    CreateKernel(image, 300);
    CreateKernel(kernel, width);
 
    typedef itk::ConvolutionImageFilter<ImageType> FilterType;
 
    // Convolve image with kernel.
    FilterType::Pointer convolutionFilter = FilterType::New();
    convolutionFilter->SetInput(image);
#if ITK_VERSION_MAJOR >= 4
    convolutionFilter->SetKernelImage(kernel);
#else
    convolutionFilter->SetImageKernelInput(kernel);
#endif

    ImageType::Pointer out_img = convolutionFilter->GetOutput();

    ImageType::RegionType region = out_img->GetLargestPossibleRegion();
    itk::ImageRegionIterator<ImageType> imageIterator(out_img, region);
//    ImageType::RegionType region = image->GetLargestPossibleRegion();
//    itk::ImageRegionIterator<ImageType> imageIterator(image, region);
//    ImageType::RegionType region = kernel->GetLargestPossibleRegion();
//    itk::ImageRegionIterator<ImageType> imageIterator(kernel, region);

    printf (".....\n");
    while(!imageIterator.IsAtEnd())
    {
        printf ("%g ", (float) imageIterator.Get());
        ++imageIterator;
    }
    printf (".....\n");
}
 
template<class T> 
T
itk_image_conv (T img, T kernel)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ConvolutionImageFilter<ImageType> FilterType;
    typename FilterType::Pointer filter = FilterType::New();
    filter->SetInput (img);
#if ITK_VERSION_MAJOR >= 4
    filter->SetKernelImage(kernel);
#else
    filter->SetImageKernelInput(kernel);
#endif

    garbage();

    T out_img = filter->GetOutput();
    double min_val, max_val, avg;
    int non_zero, num_vox;
//    itk_image_stats (itk_image_out, &min_val, &max_val, 
    itk_image_stats (out_img, &min_val, &max_val, 
        &avg, &non_zero, &num_vox);

    printf (">> MIN %g AVG %g MAX %g NONZERO: (%d / %d)\n",
        min_val, avg, max_val, non_zero, num_vox);

    return filter->GetOutput ();
}

/* Explicit instantiations */
template PLMBASE_API FloatImageType::Pointer itk_image_conv (FloatImageType::Pointer, FloatImageType::Pointer);
