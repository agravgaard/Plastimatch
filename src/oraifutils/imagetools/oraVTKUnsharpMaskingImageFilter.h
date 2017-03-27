//
#ifndef ORAVTKUNSHARPMASKINGIMAGEFILTER_H_
#define ORAVTKUNSHARPMASKINGIMAGEFILTER_H_

#include <vtkThreadedImageAlgorithm.h>

// forward declarations:
class vtkImageData;
namespace ora
{
class VTKMeanImageFilter;
}


namespace ora
{

/**
 * FIXME:
 * @author phil 
 * @version 1.0
 */
class VTKUnsharpMaskingImageFilter : public vtkThreadedImageAlgorithm
{
public:
  /** Standard New-construction. **/
  static VTKUnsharpMaskingImageFilter *New();
  vtkTypeRevisionMacro(VTKUnsharpMaskingImageFilter, vtkThreadedImageAlgorithm)

  vtkSetClampMacro(Radius, int, 1, VTK_LARGE_INTEGER)
  vtkGetMacro(Radius, int)

  vtkSetMacro(AutoRadius, bool)
  vtkGetMacro(AutoRadius, bool)
  vtkBooleanMacro(AutoRadius, bool)

  vtkGetMacro(InternalRadius, int)

  vtkGetObjectMacro(UnsharpMask, vtkImageData)

  vtkGetMacro(MeanIntensity, double)

  /** @see vtkObject#Modified() **/
  virtual void Modified();

protected:
  /** Radius of 'blurring' in pixels (unsharp part). **/
  int Radius;
  /** Flag indicating if either the radius should automatically be set or if the
   * Radius-property should be useed. The auto-radius is empiric:
   * take the maximum of the following values: dimension size divided by 32.
   * After computation, the empirically determined auto-radius can be read out
   * with the InternalRadius-getter. **/
  bool AutoRadius;
  /** Internal, empirically determined radius in auto-radius-mode. **/
  int InternalRadius;
  /** Internal multi-threaed mean image filter for generating the 'unsharp' part
   * that acts as a mask for the original image. **/
  VTKMeanImageFilter *MeanFilter;
  /** Holds the internally generated unsharp mask. **/
  vtkImageData *UnsharpMask;
  /** Mean intensity of input image which was automatically determined during
   * blurring of the input image. **/
  double MeanIntensity;

  /** Default constructor **/
  VTKUnsharpMaskingImageFilter();
  /** Destructor **/
  virtual ~VTKUnsharpMaskingImageFilter();

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
  VTKUnsharpMaskingImageFilter(const VTKUnsharpMaskingImageFilter&);
  /** Purposely not implemented. **/
  void operator=(const VTKUnsharpMaskingImageFilter&);

};

}

#endif /* ORAVTKUNSHARPMASKINGIMAGEFILTER_H_ */
