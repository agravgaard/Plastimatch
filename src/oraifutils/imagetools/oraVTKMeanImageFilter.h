//
#ifndef ORAVTKMEANIMAGEFILTER_H_
#define ORAVTKMEANIMAGEFILTER_H_

#include <vtkThreadedImageAlgorithm.h>

// forward declarations:
class vtkMutexLock;
class vtkConditionVariable;


namespace ora
{

/** \class VTKMeanImageFilter
 * \brief Mean image filter using separable convolution with temporary buffer.
 *
 * A mean image filter using separable convolution in order to increase
 * performance. On the other hand, this approach requires a temporary buffer
 * which has the same size as the input image.
 *
 * FIXME: implement 3D code later
 * The code is applicable to both 3D and 2D images, and is optimized for each
 * of these two dimensions. This filter works solely on the first scalar
 * component! Furthermore, it is assumed that the input scalar type equals the
 * output scalar type!
 *
 * Dimension and radius for the filtering process must be specified.
 * Additionally, this filter provides the detected mean intensity of the input
 * image.
 *
 * <b>Tests</b>:<br>
 * TestVTKMeanImageFilter.cxx <br>
 *
 * @see ora::vtkThreadedImageAlgorithm
 *
 * @author phil 
 * @version 1.1
 */
class VTKMeanImageFilter: public vtkThreadedImageAlgorithm
{
public:
  /** Standard New-construction. **/
  static VTKMeanImageFilter *New();
  vtkTypeRevisionMacro(VTKMeanImageFilter, vtkThreadedImageAlgorithm)

  /** Dimensionality is either 2 or 3 **/
  vtkSetClampMacro(Dimensionality, int, 2, 3)
  vtkGetMacro(Dimensionality, int)
  virtual void SetDimensionalityTo2()
  {
    SetDimensionality(2);
  }
  virtual void SetDimensionalityTo3()
  {
    SetDimensionality(3);
  }

  vtkSetClampMacro(Radius, int, 1, VTK_LARGE_INTEGER)
  vtkGetMacro(Radius, int)

  /** @return the detected mean intensity of the input image. Could be
   * interesting for some applications. **/
  double GetMeanIntensityOfOriginalImage();

protected:
  /** Dimensionality of the convolution: 2D or 3D (this is because the internal
   * code is optimized for each dimensionality). **/
  int Dimensionality;
  /** Radius of 'blurring' in pixels. **/
  int Radius;
  /** Internal temporary buffer for storing intermediate results. The size of
   * this buffer equals the size of the input image during computation, but is
   * deleted immediately after computation. **/
  void *SeparableBuffer;
  /** Helper array for multi-threaded extraction of the mean intensity. **/
  double *Averages;
  /** Helper for indexing the averages-array. **/
  int AveragesCount;
  /** Mutex for internal thread-synchronization. **/
  vtkMutexLock *Mutex;
  /** Wait-condition for internal thread-synchronization. **/
  vtkConditionVariable *WaitCondition;
  /** Helper counting the number of finished threads during computation. **/
  int NumberOfFinishedThreads;

  /** Default constructor **/
  VTKMeanImageFilter();
  /** Destructor **/
  virtual ~VTKMeanImageFilter();

  /** Print out object information. **/
  void PrintSelf(ostream& os, vtkIndent indent);

  /** Entry-point for distributed threaded code. **/
  void ThreadedRequestData(vtkInformation *request,
      vtkInformationVector **inputVector, vtkInformationVector *outputVector,
      vtkImageData ***inData, vtkImageData **outData, int outExt[6], int id);

  /** Main entry-point for distributing the threaded code: before threading,
   * threading, after threading. **/
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
      vtkInformationVector* outputVector);

private:
  /** Purposely not implemented. **/
  VTKMeanImageFilter(const VTKMeanImageFilter&);
  /** Purposely not implemented. **/
  void operator=(const VTKMeanImageFilter&);

};

}

#endif /* ORAVTKMEANIMAGEFILTER_H_ */
