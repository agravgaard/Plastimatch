/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __itkClampCastImageFilter_h
#define __itkClampCastImageFilter_h

#include "itkConfigure.h"
#include "itkImageToImageFilter.h"

namespace itk
{

template <class TInputImage, class TOutputImage>
class ITK_EXPORT ClampCastImageFilter 
    : public ImageToImageFilter<TInputImage, TOutputImage>
{
  public:
    /** Standard class typedefs. */
    typedef ClampCastImageFilter               Self;
    typedef ImageToImageFilter<TInputImage,TOutputImage> Superclass;
    typedef SmartPointer<Self>                 Pointer;
    typedef SmartPointer<const Self>           ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);  

    /** Run-time type information (and related methods). */
    itkTypeMacro(ClampCastImageFilter, UnaryFunctorImageFilter);

    /** Some additional typedefs.  */
    typedef typename TInputImage::ConstPointer InputImagePointer;
    typedef typename TInputImage::RegionType   InputImageRegionType;
    typedef typename TInputImage::PixelType    InputImagePixelType;

    /** Some additional typedefs.  */
    typedef typename TOutputImage::Pointer     OutputImagePointer;
    typedef typename TOutputImage::RegionType  OutputImageRegionType;
    typedef typename TOutputImage::PixelType   OutputImagePixelType;

  protected:
    ClampCastImageFilter();
    ~ClampCastImageFilter() {};
    void PrintSelf(std::ostream& os, Indent indent) const;

#if ITK_VERSION_MAJOR == 3
    void ThreadedGenerateData (
	const OutputImageRegionType& outputRegionForThread,
	int threadId);
#else /* ITK 4 */
    void ThreadedGenerateData (
	const OutputImageRegionType& outputRegionForThread,
	ThreadIdType threadId);
#endif

  private:
    ClampCastImageFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented
};

  
} // end namespace itk
  
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkClampCastImageFilter.txx"
#endif
  
#endif
