//
#include "oraVTKUnsharpMaskingImageFilter.h"

#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include "oraVTKMeanImageFilter.h"


namespace ora
{

vtkCxxRevisionMacro(VTKUnsharpMaskingImageFilter, "1.0")
vtkStandardNewMacro(VTKUnsharpMaskingImageFilter)

VTKUnsharpMaskingImageFilter::VTKUnsharpMaskingImageFilter()
{
  Radius = 3;
  AutoRadius = true;
  InternalRadius = 0;
  MeanFilter = VTKMeanImageFilter::New();
  UnsharpMask = NULL;
  MeanIntensity = 0;
}

VTKUnsharpMaskingImageFilter::~VTKUnsharpMaskingImageFilter()
{
  UnsharpMask = NULL;
  MeanFilter->Delete();
  MeanFilter = NULL;
}

void VTKUnsharpMaskingImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "AutoRadius: " << AutoRadius << "\n";
  os << indent << "InternalRadius: " << InternalRadius << "\n";
  os << indent << "Radius: " << Radius << "\n";
  os << indent << "MeanFilter: " << MeanFilter << "\n";
  os << indent << "UnsharpMask: " << UnsharpMask << "\n";
  os << indent << "MeanIntensity: " << MeanIntensity << "\n";
}

template<class T>
void UnsharpMaskingExecute(VTKUnsharpMaskingImageFilter *self,
    vtkImageData *inData, T *inPtr, vtkImageData *outData, T *outPtr,
    int outExt[6], int id, vtkInformation *inInfo, vtkImageData *mask,
    double mean)
{
  int we[6]; // WHOLE EXTENT <- (read access; IN)
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), we);
  vtkIdType inc0, inc1, inc2;
  outData->GetIncrements(inc0, inc1, inc2); // OUT inc = IN inc
  // IN pointer (absolute)
  T *pi = static_cast<T*>(inData->GetScalarPointer(we[0], we[2], we[4]));
  int xmin = outExt[0];
  int xmax = outExt[1];
  int ymin = outExt[2];
  int ymax = outExt[3];
  int zmin = outExt[4];
  int zmax = outExt[5];
  T *po = static_cast<T*>(outData->GetScalarPointer(we[0], we[2], we[4]));
  T *pm = static_cast<T*>(mask->GetScalarPointer(we[0], we[2], we[4]));
  int x, y, z, t;
  T *piSlice = NULL; // shortcuts
  T *poSlice = NULL;
  T *pmSlice = NULL;
  T *piLine = NULL;
  T *poLine = NULL;
  T *pmLine = NULL;
  T tmean = static_cast<T>(mean);

  // compute for each pixel: OUT(x,y,z) = MEAN + IN(x,y,z) - UMASK(x,y,z)
  // ... where OUT is the output image, IN is the input image, mean is the
  // average intensity value of the input image and UMASK is the unsharp mask
  // (blurred input image)
  for (z = zmin; z <= zmax; z++)
  {
    t = z * inc2;
    piSlice = pi + t;
    poSlice = po + t;
    pmSlice = pm + t;
    for (y = ymin; y <= ymax; y++)
    {
      t = y * inc1;
      piLine = piSlice + t;
      poLine = poSlice + t;
      pmLine = pmSlice + t;
      for (x = xmin; x <= xmax; x++)
      {
        *(poLine + x) = tmean + *(piLine + x) - *(pmLine + x);
      }
    }
  }

}

void VTKUnsharpMaskingImageFilter::ThreadedRequestData(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector,
    vtkImageData ***inData, vtkImageData **outData, int outExt[6], int id)
{
  void *inPtr = inData[0][0]->GetScalarPointerForExtent(outExt);
  void *outPtr = outData[0]->GetScalarPointerForExtent(outExt);

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  // this filter expects the output type to be same as input
  if (outData[0]->GetScalarType() != inData[0][0]->GetScalarType())
  {
    vtkErrorMacro(<< "Execute: output ScalarType, "
        << vtkImageScalarTypeNameMacro(outData[0]->GetScalarType())
        << " must match input scalar type");
    return;
  }

  switch (inData[0][0]->GetScalarType())
  {
    vtkTemplateMacro(
        UnsharpMaskingExecute(this, inData[0][0],
            static_cast<VTK_TT *>(inPtr), outData[0],
            static_cast<VTK_TT *>(outPtr),
            outExt, id, inInfo, UnsharpMask, MeanIntensity))
      ;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType")
      return;
  }
}

int VTKUnsharpMaskingImageFilter::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // BEFORE THREADED GENERATE DATA:

  vtkImageData *indata = dynamic_cast<vtkImageData *> (this->GetInput());
  if (!indata)
    return 0;
//  MeanFilter->Delete();
//  MeanFilter = NULL;
//  MeanFilter = VTKMeanImageFilter::New();
  MeanFilter->SetInput(indata);
  int r;
  int dims[3];
  indata->GetDimensions(dims);
  if (AutoRadius)
  {
    // -> empirically determine a radius:
    InternalRadius = dims[0] / 32;
    r = dims[1] / 32;
    if (r > InternalRadius)
      InternalRadius = r;
    r = dims[2] / 32;
    if (r > InternalRadius)
      InternalRadius = r;
    r = InternalRadius;
  }
  else
  {
    r = Radius; // take set parameter
  }
  MeanFilter->SetRadius(r);
  MeanFilter->SetNumberOfThreads(this->GetNumberOfThreads()); // -> take over
  if (dims[2] > 1)
    MeanFilter->SetDimensionalityTo3();
  else
    MeanFilter->SetDimensionalityTo2();
  MeanFilter->Update(); // generate the UNSHARP MASK
  UnsharpMask = MeanFilter->GetOutput();
  MeanIntensity = MeanFilter->GetMeanIntensityOfOriginalImage();

  // THREADED GENERATE DATA:
  int ret = Superclass::RequestData(request, inputVector, outputVector);


  // AFTER THREADED GENERATE DATA:

  return ret;
}

void VTKUnsharpMaskingImageFilter::Modified()
{
  Superclass::Modified();
  MeanFilter->Modified();
}

}
