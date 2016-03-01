//
#ifndef ORADRRFILTER_H_
#define ORADRRFILTER_H_

//STL
#include <vector>
//ITK
#include <itkImageToImageFilter.h>
#include <itkConceptChecking.h>
#include <itkTransform.h>
#include <itkMacro.h>
#include <itkRealTimeClock.h>
//ORAIFUTILS
#include "oraProjectionGeometry.h"
#include "oraIntensityTransferFunction.h"

namespace ora
{

/** \class DRRFilter
 * \brief Defines an abstract base class for a standardized DRR filter.
 *
 * This class implements the basic definition of a DRR image filter
 * as proposed in "plastimatch digitally reconstructed radiographs
 * (DRR) application programming interface (API)" (design document).
 *
 * Please,
 * refer to this design document in order to retrieve more information on the
 * assumptions and restrictions regarding DRR filters!
 *
 * This class is templated over the input volume pixel type and the output
 * DRR pixel type.
 *
 * <b>Tests</b>:<br>
 * <br>
 *
 * @see itk::ImageToImageFilter
 *
 * @author phil
 * @version 1.0
 *
 * \ingroup ImageFilters
 **/
template<class TInputPixelType, class TOutputPixelType>
class DRRFilter:
public itk::ImageToImageFilter<itk::Image<TInputPixelType, 3>, itk::Image<
TOutputPixelType, 3> >
{
public:
	/** Standard class typedefs. */
	typedef DRRFilter Self;
	typedef itk::ImageToImageFilter<itk::Image<TInputPixelType, 3>, itk::Image<
	TOutputPixelType, 3> > Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self> ConstPointer;

	/** Accessibility typedefs. **/
	typedef itk::Image<TInputPixelType, 3> InputImageType;
	typedef typename InputImageType::Pointer InputImagePointer;
	typedef typename InputImageType::ConstPointer InputImageConstPointer;
	typedef typename InputImageType::PixelType InputImagePixelType;
	typedef itk::Image<TOutputPixelType, 3> OutputImageType;
	typedef typename OutputImageType::Pointer OutputImagePointer;
	typedef typename OutputImageType::ConstPointer OutputImageConstPointer;
	typedef typename OutputImageType::PixelType OutputImagePixelType;
	typedef typename OutputImageType::RegionType OutputImageRegionType;
	typedef unsigned char MaskPixelType;
	typedef itk::Image<MaskPixelType, 3> MaskImageType;
	typedef MaskImageType::Pointer MaskImagePointer;
	typedef std::vector<MaskImagePointer> MaskVectorType;
	typedef itk::Transform<double,
	itkGetStaticConstMacro(InputImageDimension),
	itkGetStaticConstMacro(InputImageDimension)> TransformType;
	typedef typename TransformType::Pointer TransformPointer;
	typedef ora::ProjectionGeometry GeometryType;
	typedef GeometryType::Pointer GeometryPointer;
	typedef GeometryType::ConstPointer GeometryConstPointer;
	typedef ora::IntensityTransferFunction ITFType;
	typedef ITFType::Pointer ITFPointer;
	typedef std::vector<GeometryPointer> GeometryVectorType;
	typedef itk::Vector<double, 3> VectorType;

	/** Run-time type information (and related methods). */
	itkTypeMacro(Self, Superclass)

	/** Set the input image (3D volume) based on ITK image data.
	 * @param input the ITK image data set to be used for DRR computation **/
	virtual void SetInput(InputImagePointer input);

	/** Generate information describing the output data.
	 * A DRR filter usually produces an image with a different size than its input
	 * image.
	 * @see itk::ProcessObject#GenerateOutputInformaton() **/
	virtual void GenerateOutputInformation();

	/** Generate the information describing the requested input region in the
	 * input image. In general, we need a different input requested region than
	 * output requested region.
	 * @see itk::ImageToImageFilter#GenerateInputRequestedRegion()
	 * @see itk::ImageSource#GenerateInputRequestedRegion()
	 * @see itk::ProcessObject#GenerateInputRequestedRegion()
	 * */
	virtual void GenerateInputRequestedRegion();

	/** @return object's modified time depending on the modified times of its
	 * internal components.
	 * @see itk::Object#GetMTime()
	 */
	virtual unsigned long GetMTime() const;

	/** Set the connected volume transformation (ITK representation). A filter
	 * may or may not tolerate a NULL pointer (initial state) for this member. A
	 * NULL pointer may indicate identity transformation.
	 **/
	virtual void SetTransform(TransformType *transform);
	/** Get volume transformation (ITK representation) **/
	itkGetObjectMacro(Transform, TransformType)

	/** Set number of independent DRR outputs. This means that there can be
	 * multiple DRR outputs which are independent from each other. Depending on
	 * the value of CurrentDRROutputIndex successive DRR computations will be
	 * related to the specified output.
	 * @see SetCurrentDRROutputIndex() **/
	virtual void SetNumberOfIndependentOutputs(int numOutputs);
	/** Get number of independent DRR outputs. **/
	itkGetMacro(NumberOfIndependentOutputs, int)
	/** Set index that determines to which DRR output successive DRR computations
	 * will relate. Range: 0 .. m_NumberOfIndependentOutputs-1
	 * @see SetNumberOfIndependentOutputs() **/
	virtual void SetCurrentDRROutputIndex(int currIndex);
	/** Get index that determines to which DRR output successive DRR computations
	 * will relate.
	 */
	itkGetMacro(CurrentDRROutputIndex, int)
	/** Removes the specified output. **/
	virtual void RemoveDRROutput(std::size_t index);

	/** Set an optional DRR mask for the specified index. NOTE: The mask is
	 * required to match the size of the DRR (in pixels) exactly in order to be
	 * considered!
	 * @param index output index to which the mask relates
	 * @param mask the mask image (pixels having values >0 will be computed)
	 **/
	virtual void SetDRRMask(int index, MaskImagePointer mask);
	/** Get the optional DRR mask for the specified output index. **/
	virtual MaskImagePointer GetDRRMask(int index) const;
	/** @return TRUE if the DRR implementation supports masks **/
	virtual bool IsSupportingDRRMasks() const = 0;

	/** Set a pointer to the associated projection geometry for the specified
	 * index. All projection geometry pointers must be valid when calling Update()!
	 **/
	virtual void SetProjectionGeometry(int index, GeometryPointer geometry);
	/** Get a pointer to the associated projection geometry for the specified
	 * index. All projection geometry pointers must be valid when calling Update()!
	 **/
	virtual GeometryPointer GetProjectionGeometry(int index) const;

	/** Set an optional intensity transfer function to be used during DRR
	 * computation. A NULL pointer may or may not be tolerated by a concrete
	 * DRR implementation. **/
	itkSetObjectMacro(ITF, ITFType)
	itkGetConstObjectMacro(ITF, ITFType)

	/** Indicate whether the DRR engine supports ITFs or not. **/
	bool IsSupportingITF() const
	{
		return (IsSupportingITFOnTheFly() || IsSupportingITFOffTheFly());
	}
	/** Indicate whether the DRR engine supports ITFs on the fly or not. "On the
	 * fly" means applying the ITF-mapping to each casted voxel intensity during
	 * DRR computation avoiding any pre-computational step to the volume. **/
	virtual bool IsSupportingITFOnTheFly() const = 0;
	/** Indicate whether the DRR engine supports ITFs off the fly or not. "Off the
	 * fly" means applying the ITF-mapping to the input volume prior to the
	 * essential DRR computation. Such an implementation may require additional
	 * memory as the pre-mapped volume must be stored internally. **/
	virtual bool IsSupportingITFOffTheFly() const = 0;
	/** Indicate whether the DRR Engine supports rigid transformation **/
	virtual bool IsSupportingRigidTransformation() const = 0;
        /** Indicate whether the DRR Engine supports rigid transformation **/
        virtual bool IsSupportingAffineTransformation() const = 0;
        /** Indicate whether the DRR Engine supports rigid transformation **/
        virtual bool IsSupportingElasticTransformation() const = 0;
	/** @return TRUE if current settings are sufficient for computing a DRR **/
	virtual bool DRRCanBeComputed() const = 0;
	/** @return TRUE if the DRR implementation is GPU-based **/
	virtual bool IsGPUBased() const = 0;
	/** @return TRUE if the DRR implementation is CPU-based **/
	virtual bool IsCPUBased() const = 0;
	/** @return TRUE if the DRR implementation is CPU-multi-threaded **/
	virtual bool IsCPUMultiThreaded() const = 0;

	/** Concept checking */
	#ifdef ITK_USE_CONCEPT_CHECKING
	  /** input image type must have numeric pixel type **/
	  itkConceptMacro(InputHasNumericTraitsCheck,
	      (itk::Concept::HasNumericTraits<InputImagePixelType>));
	  /** output image type must have numeric pixel type **/
	  itkConceptMacro(OutputHasNumericTraitsCheck,
	      (itk::Concept::HasNumericTraits<OutputImagePixelType>));
	#endif

protected:
	/** Internal typedefs **/
	typedef itk::RealTimeClock ClockType;
	typedef ClockType::Pointer ClockPointer;

	/** Internal transform describing the volume transformation
	 * (ITK-representation); it's integrity is internally guaranteed via
	 * Modified()/GetMTime(). **/
	TransformPointer m_Transform;
	/** Number of independent DRR outputs. This means that there can be
	 * multiple DRR outputs which are independent from each other. Depending on
	 * the value of CurrentDRROutputIndex successive DRR computations will be
	 * related to the specified output. */
	int m_NumberOfIndependentOutputs;
	/** Index that determines to which DRR output successive DRR computations
	 * currently relate. */
	int m_CurrentDRROutputIndex;
	/** Projection geometries. **/
	GeometryVectorType m_Geometries;
	/** Shortcut to current input volume. **/
	InputImagePointer m_Input;
	/** DRR masks. **/
	MaskVectorType m_Masks;
	/** Intensity transfer function. **/
	ITFPointer m_ITF;


	/** Default constructor. **/
	DRRFilter();
	/** Default destructor. **/
	virtual ~DRRFilter();

	/** Print description of this object. **/
	virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

	/** Can only operate on complete output region.
	 * @see itk::ProcessObject#EnlargeOutputRequestedRegion()
	 */
	virtual void EnlargeOutputRequestedRegion(itk::DataObject *data);

	/** In general apply individual requested region to each of the outputs.
	 * @see itk::ProcessObject#GenerateOutputRequestedRegion()
	 */
	virtual void GenerateOutputRequestedRegion(itk::DataObject *data);

	/** Executes the DRR computation. This method must be overridden in concrete
	 * subclasses that are CPU-single-threaded or GPU-threaded (inititiate the
	 * GPU-computation here).
	 * @see itk::ProcessObject#GenerateData()
	 * @see BeforeThreadedGenerateData()
	 * @see ThreadedGenerateData()
	 * @see AfterThreadedGenerateData() **/
	virtual void GenerateData();
	/** Executes the DRR computation. This method must be overridden in concrete
	 * subclasses that are CPU-threaded.
	 * @see GenerateData()
	 * @see BeforeThreadedGenerateData()
	 * @see AfterThreadedGenerateData() **/
	virtual void ThreadedGenerateData(
			const OutputImageRegionType& outputRegionForThread, int threadId);
	/** CPU-threaded DRR computation pre-processing entry point.
	 * @see GenerateData()
	 * @see ThreadedGenerateData()
	 * @see AfterThreadedGenerateData() **/
	virtual void BeforeThreadedGenerateData();
	/** CPU-threaded DRR computation post-processing entry point.
	 * @see GenerateData()
	 * @see ThreadedGenerateData()
	 * @see BeforeThreadedGenerateData() **/
	virtual void AfterThreadedGenerateData();

	/** Ensure that current output image (according to current output index) is
	 * allocated and has the correct size. **/
	virtual void EnsureCurrentImageIsCorrect(GeometryPointer geometry);

	/** We do not want the superclass (itk::ImageSource) to allocate the outputs;
	 * we can do this alone! @see EnsureCurrentImageIsCorrect() **/
	virtual void AllocateOutputs() {};

  /** Splits the requested region of the CURRENT output into pieces in order to
   * generate appropriate non-overlapping operation patches where parallel
   * threads can work on avoiding any interference.<br>
   * <b>NOTE</b>: In contrast to the superclass implementation, this method
   * operates not permanently on GetOutput()! It rather operates on the current
   * output since this filter interface supports multiple outputs!
   * @see itk::ImageSource#SplitRequestedRegion() */
  virtual int SplitRequestedRegion(int i, int num,
  		OutputImageRegionType& splitRegion);

private:
	/** Purposely not implemented. **/
	DRRFilter(const Self&);
	/** Purposely not implemented. **/
	void operator=(const Self&);

};

}

#include "oraDRRFilter.txx"

#endif /* ORADRRFILTER_H_ */
