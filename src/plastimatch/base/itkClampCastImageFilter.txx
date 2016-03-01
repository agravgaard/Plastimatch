/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __itkClampCastImageFilter_txx
#define __itkClampCastImageFilter_txx

#include "itkClampCastImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNumericTraits.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"

namespace itk
{

template <class TInputImage, class TOutputImage>
ClampCastImageFilter<TInputImage, TOutputImage>
::ClampCastImageFilter()
{
    /* Do nothing */
}

template <class TInputImage, class TOutputImage>
void 
ClampCastImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
    /* Do nothing */
}

template <class TInputImage, class TOutputImage>
void 
ClampCastImageFilter<TInputImage, TOutputImage>
::ThreadedGenerateData (
    const OutputImageRegionType& outputRegionForThread,
#if ITK_VERSION_MAJOR == 3
    int threadId
#else /* ITK 4 */
    ThreadIdType threadId
#endif
) {
    itkDebugMacro(<<"Actually executing");

    // Get the input and output pointers
    InputImagePointer  inputPtr  = this->GetInput();
    OutputImagePointer outputPtr = this->GetOutput(0);

    /* Get iterators */
    typedef ImageRegionConstIterator<TInputImage> InputIterator;
    typedef ImageRegionIterator<TOutputImage> OutputIterator;
    InputIterator  in_it(inputPtr, outputRegionForThread);
    OutputIterator out_it(outputPtr, outputRegionForThread);

    /* Get min and max values of output pixel type */
    OutputImagePixelType out_min = NumericTraits<
	OutputImagePixelType>::NonpositiveMin();
    OutputImagePixelType out_max = NumericTraits<
	OutputImagePixelType>::max();

    /* support progress methods/callbacks */
    ProgressReporter progress (this, threadId, 
	outputRegionForThread.GetNumberOfPixels());

    /* Make gcc accept the below without complaining about 
       signed/unsigned compare */
//#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6)
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wsign-compare"
//#endif

    /* walk through image, clamping and casting each pixel */
    while (!out_it.IsAtEnd())
    {
	const InputImagePixelType in_value = in_it.Get();
	if (!NumericTraits<OutputImagePixelType>::is_signed && in_value < 0)
	{
	    out_it.Set (0);
	}
	else if (in_value > out_max) {
	    out_it.Set (out_max);
	} else if (in_value < out_min) {
	    out_it.Set (out_min);
	} else {
	    out_it.Set (static_cast<OutputImagePixelType> (in_it.Get()));
	}
	++in_it;
	++out_it;
	progress.CompletedPixel();
    }

//#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6)
//#pragma GCC diagnostic pop
//#endif

}

} // end namespace itk

#endif
