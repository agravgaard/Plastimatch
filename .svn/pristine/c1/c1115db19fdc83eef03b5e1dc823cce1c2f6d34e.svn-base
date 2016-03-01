#ifndef ORAMULTIRESOLUTIONPYRAMIDMASKFILTER_H
#define ORAMULTIRESOLUTIONPYRAMIDMASKFILTER_H

#include <itkImageToImageFilter.h>
#include <itkArray2D.h>

namespace ora
{

// FIXME: DOCUMENTATION OF THIS CLASS IS COMPLETELY MISSING!

  template<class TMaskImage>
  class MultiResolutionPyramidMaskFilter :
    public itk::ImageToImageFilter<TMaskImage, TMaskImage>
{
public:
  typedef MultiResolutionPyramidMaskFilter Self;
  typedef itk::ImageToImageFilter< TMaskImage, TMaskImage > Superclass;
  typedef itk::SmartPointer< Self > Pointer;
  typedef itk::SmartPointer< const Self > ConstPointer;

  itkNewMacro(Self);

  itkTypeMacro(MultiResolutionPyramidMaskFilter, ImageToImageFilter);

  typedef itk::Array2D< unsigned int > ScheduleType;
  typedef itk::DataObject DataObject;

  itkStaticConstMacro(ImageDimension, unsigned int,
      TMaskImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
      TMaskImage::ImageDimension);
  typedef typename Superclass::InputImageType InputImageType;
  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename Superclass::InputImagePointer InputImagePointer;
  typedef typename Superclass::OutputImagePointer OutputImagePointer;
  typedef typename Superclass::InputImageConstPointer InputImageConstPointer;

  virtual void SetNumberOfLevels(unsigned int num);

  itkGetConstMacro(NumberOfLevels, unsigned int);

  virtual void SetSchedule(const ScheduleType & schedule);

  itkGetConstReferenceMacro(Schedule, ScheduleType);

  virtual void SetStartingShrinkFactors(unsigned int factor);

  virtual void SetStartingShrinkFactors(unsigned int *factors);

  const unsigned int * GetStartingShrinkFactors() const;

  static bool IsScheduleDownwardDivisible(const ScheduleType & schedule);

  virtual void GenerateOutputInformation();

  virtual void GenerateOutputRequestedRegion(DataObject *output);

  virtual void GenerateInputRequestedRegion();

  itkSetMacro(MaximumError, double);
  itkGetConstReferenceMacro(MaximumError, double);

  itkSetMacro(UseShrinkImageFilter, bool);
  itkGetConstMacro(UseShrinkImageFilter, bool);
  itkBooleanMacro(UseShrinkImageFilter);

#ifdef ITK_USE_CONCEPT_CHECKING
  itkConceptMacro( SameDimensionCheck,
    ( itk::Concept::SameDimension< ImageDimension, OutputImageDimension > ) );
  itkConceptMacro( OutputHasNumericTraitsCheck,
    ( itk::Concept::HasNumericTraits< typename TMaskImage::PixelType > ) );
#endif

protected:
  MultiResolutionPyramidMaskFilter();
  virtual ~MultiResolutionPyramidMaskFilter();
  void PrintSelf(std::ostream & os, itk::Indent indent) const;

  void GenerateData();

  double m_MaximumError;

  unsigned int m_NumberOfLevels;
  ScheduleType m_Schedule;

  bool m_UseShrinkImageFilter;

private:
  MultiResolutionPyramidMaskFilter(const Self &); //purposely not implemented
  void operator=(const Self &); //purposely not implemented
};
} // NAMESPACE ORA
#include "oraMultiResolutionPyramidMaskFilter.txx"
#endif //ORAMULTIRESOLUTIONPYRAMIDMASKFILTER_H
