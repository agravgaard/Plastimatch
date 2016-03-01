//
#ifndef ORAONETOONEINTERPOLATEIMAGEFUNCTION_H_
#define ORAONETOONEINTERPOLATEIMAGEFUNCTION_H_

#include <itkInterpolateImageFunction.h>

namespace ora
{

/** \class OneToOneInterpolateImageFunction
 * \brief Passes-through image intensities at the specified positions.
 *
 * Passes-through image intensities at the specified positions. This requires
 * the input image to perfectly overlap the other image. This class is primarily
 * meant to be a tool for ora::MultiResolutionNWay2D3DRegistrationMethod.
 *
 * This class behaves similar as itk::NearestNeighborInterpolateImageFunction
 * as casting continuous index positions can result in 'wrong' index positions.
 *
 * <b>Tests</b>:<br>
 * TestOneToOneInterpolateImageFunction.cxx <br>
 * TestMultiResolutionNWay2D3DRegistrationMethod.cxx
 *
 * @see ora::MultiResolutionNWay2D3DRegistrationMethod
 * @see itk::NearestNeighborInterpolateImageFunction
 *
 * @author phil 
 * @version 1.1
 *
 * \ingroup ImageInterpolators
 */
template<class TInputImage, class TCoordRep = double>
class OneToOneInterpolateImageFunction:
    public itk::InterpolateImageFunction<TInputImage, TCoordRep>
{
public:
  /** Standard class typedefs. */
  typedef OneToOneInterpolateImageFunction Self;
  typedef itk::InterpolateImageFunction<TInputImage, TCoordRep> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(OneToOneInterpolateImageFunction, InterpolateImageFunction)

  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** OutputType typedef support. */
  typedef typename Superclass::OutputType OutputType;

  /** InputImageType typedef support. */
  typedef typename Superclass::InputImageType InputImageType;

  /** Dimension underlying input image. */
  itkStaticConstMacro(ImageDimension, unsigned int, Superclass::ImageDimension);

  /** Index typedef support. */
  typedef typename Superclass::IndexType IndexType;

  /** ContinuousIndex typedef support. */
  typedef typename Superclass::ContinuousIndexType ContinuousIndexType;

  /** Evaluate the function at a ContinuousIndex position
   *
   * Returns the interpolated image intensity at a
   * specified point position. No bounds checking is done.
   * The point is assume to lie within the image buffer.
   *
   * ImageFunction::IsInsideBuffer() can be used to check bounds before
   * calling the method. */
  virtual OutputType
      EvaluateAtContinuousIndex(const ContinuousIndexType &index) const;

protected:
  /** Default constructor **/
  OneToOneInterpolateImageFunction();
  /** Destructor **/
  virtual ~OneToOneInterpolateImageFunction();

  /** Print-out object information. **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  /** Purposely not implemented **/
  OneToOneInterpolateImageFunction(const Self&);
  /** Purposely not implemented **/
  void operator=(const Self&);

};

}

#include "oraOneToOneInterpolateImageFunction.txx"

#endif /* ORAONETOONEINTERPOLATEIMAGEFUNCTION_H_ */
