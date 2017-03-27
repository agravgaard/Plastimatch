//
#include "oraVTKMeanImageFilter.h"

#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtksys/SystemTools.hxx>
#include <vtkMutexLock.h>
#include <vtkConditionVariable.h>


namespace ora
{

vtkCxxRevisionMacro(VTKMeanImageFilter, "1.0")
vtkStandardNewMacro(VTKMeanImageFilter)

void VTKMeanImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Dimensionality: " << Dimensionality << "\n";
  os << indent << "Radius: " << Radius << "\n";
}

VTKMeanImageFilter::VTKMeanImageFilter() :
  vtkThreadedImageAlgorithm()
{
  Radius = 1;
  Dimensionality = 2;
  Averages = NULL;
  AveragesCount = 0;
  SeparableBuffer = NULL;
  WaitCondition = vtkConditionVariable::New();
  Mutex = vtkMutexLock::New();
  NumberOfFinishedThreads = 0;
}

VTKMeanImageFilter::~VTKMeanImageFilter()
{
  if (Averages)
    delete[] Averages;
  Averages = NULL;
  AveragesCount = 0;
  WaitCondition->Delete();
  WaitCondition = NULL;
  Mutex->Delete();
  Mutex = NULL;
}

template<class T>
void MeanFilter2DExecute(VTKMeanImageFilter *self, vtkImageData *inData,
    T *inPtr, vtkImageData *outData, T *outPtr, int outExt[6], int id,
    vtkInformation *inInfo, double *&averages, T *tmpBuff,
    vtkMutexLock *mutex, vtkConditionVariable *condition, int &numFinishedTh)
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
  int wxmin = we[0];
  int wxmax = we[1];
  int wymin = we[2];
  int wymax = we[3];
  T *po = static_cast<T*>(outData->GetScalarPointer(we[0], we[2], we[4]));
  int x, y, r, c, n;
  double d;
  const int R = self->GetRadius();
  const int D = 2 * R + 1;
  // temporary buffer bounds in order to realize separable convolution:
  // (buffer region covers basically the thread region, but extends it w.r.t.
  // the overlap regions in order to generate correct mean values)
  int rymin = (wymin > (ymin - R)) ? wymin : (ymin - R);
  int rymax = (wymax < (ymax + R)) ? wymax : (ymax + R);
  double *avg = &(averages[id]);
  T *piLine = NULL;
  T *poLine = NULL;
  int lineOff;

  *avg = 0;
  // 1st part: smooth along x-direction and store results into temp buffer
  for (y = ymin; y <= ymax; y++)
  {
    lineOff = y * inc1;
    piLine = pi + lineOff; // start of current line (INPUT)
    for (x = xmin; x <= xmax; x++)
    {
      c = 0;
      r = x - R;
      d = 0;
      for (n = 0; n < D; n++, r++)
      {
        if (r >= wxmin && r <= wxmax)
        {
          d += static_cast<double>(*(piLine + r));
          c++;
        }
      }
      if (c > 0)
        d /= static_cast<double>(c);
      else
        d = 0;
      *(tmpBuff + lineOff + x) = static_cast<T>(d);
      // mean intensity accumulation (of original image!)
      *avg += static_cast<double>(*(piLine + x));
    }
  }

  mutex->Lock();
  numFinishedTh++;
  if (numFinishedTh < self->GetNumberOfThreads()) // wait till all threads ready
    condition->Wait(mutex);
  else // last threads wakes up the waiting threads
    condition->Broadcast();
  mutex->Unlock();

  // 2nd part: smooth along y-direction based on temp buffer content
  for (y = ymin; y <= ymax; y++)
  {
    poLine = po + y * inc1; // start of current line (OUTPUT)
    for (x = xmin; x <= xmax; x++)
    {
      c = 0;
      r = y - R;
      d = 0;
      for (n = 0; n < D; n++, r++)
      {
        if (r >= rymin && r <= rymax)
        {
          d += static_cast<double>(*(tmpBuff + r * inc1 + x));
          c++;
        }
      }
      if (c > 0)
        d /= static_cast<double>(c);
      else
        d = 0;
      *(poLine + x) = static_cast<T>(d);
    }
  }
}

template<class T>
void MeanFilter3DExecute(VTKMeanImageFilter *self, vtkImageData *inData,
    T *inPtr, vtkImageData *outData, T *outPtr, int outExt[6], int id,
    vtkInformation *inInfo, double *&averages, T *tmpBuff,
    vtkMutexLock *mutex, vtkConditionVariable *condition, int &numFinishedTh)
{
  // FIXME: implement optimized 3D code
  vtkErrorWithObjectMacro(self, << "3D currently not implemented!")
}

double VTKMeanImageFilter::GetMeanIntensityOfOriginalImage()
{
  vtkImageData *out = this->GetOutput();
  if (AveragesCount > 0 && out)
  {
    double d = 0;
    for (int i = 0; i < AveragesCount; i++)
      d += Averages[i];
    int we[6];
    out->GetWholeExtent(we);
    d /= static_cast<double>((we[1] - we[0] + 1) * (we[3] - we[2] + 1) *
        (we[5] - we[4] + 1));
    return d;
  }
  else
  {
    return 0;
  }
}

template<class T>
void DeleteSeparableBuffer(void *&buff, T dummy)
{
  T *tbuff = static_cast<T*>(buff);
  if (tbuff)
    //delete[] tbuff;
    free(tbuff);
  tbuff = NULL;
}

template<class T>
void AllocateSeparableBuffer(void *&buff, int we[6], T dummy)
{
  DeleteSeparableBuffer<T>(buff, dummy);
  long int size = (we[1] - we[0] + 1) * (we[3] - we[2] + 1) *
          (we[5] - we[4] + 1);
  //buff = (void*)new T[size];
  buff = calloc(size, sizeof(T));
}

int VTKMeanImageFilter::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // BEFORE THREADED GENERATE DATA:

  // prepare temp buffer for separable convolution:
  int we[6]; // WHOLE EXTENT <- (read access; IN)
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), we);
  vtkImageData *indata = dynamic_cast<vtkImageData *>(this->GetInput());
  if (!indata)
    return 0;
  switch (indata->GetScalarType())
  {
    vtkTemplateMacro(
        AllocateSeparableBuffer(SeparableBuffer, we, 0))
      ;
    default:
      vtkErrorMacro(<< "RequestData: Input has unknown ScalarType")
      return 0;
  }
  // prepare averages array:
  int nth = this->GetNumberOfThreads();
  if (Averages)
    delete[] Averages;
  Averages = NULL;
  AveragesCount = 0;
  if (nth > 0)
  {
    AveragesCount = nth;
    Averages = new double[AveragesCount];
    for (int i = 0; i < AveragesCount; i++)
      Averages[i] = 0; // init
  }


  // THREADED GENERATE DATA:

  NumberOfFinishedThreads = 0;
  int ret = Superclass::RequestData(request, inputVector, outputVector);


  // AFTER THREADED GENERATE DATA:

  switch (indata->GetScalarType())
  {
    vtkTemplateMacro(
        DeleteSeparableBuffer(SeparableBuffer, 0))
      ;
    default:
      vtkErrorMacro(<< "RequestData: Input has unknown ScalarType")
      return 0;
  }
  SeparableBuffer = NULL;

  return ret;
}

void VTKMeanImageFilter::ThreadedRequestData(vtkInformation *request,
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

  if (Dimensionality == 2)
  {
    switch (inData[0][0]->GetScalarType())
    {
      vtkTemplateMacro(
          MeanFilter2DExecute(this, inData[0][0],
              static_cast<VTK_TT *>(inPtr), outData[0],
              static_cast<VTK_TT *>(outPtr),
              outExt, id, inInfo, Averages,
              static_cast<VTK_TT *>(SeparableBuffer), Mutex, WaitCondition,
              NumberOfFinishedThreads))
        ;
      default:
        vtkErrorMacro(<< "Execute (2D): Unknown ScalarType")
        return;
    }
  }
  else if (Dimensionality == 3)
  {
    switch (inData[0][0]->GetScalarType())
    {
      vtkTemplateMacro(
          MeanFilter3DExecute(this, inData[0][0],
              static_cast<VTK_TT *>(inPtr), outData[0],
              static_cast<VTK_TT *>(outPtr),
              outExt, id, inInfo, Averages,
              static_cast<VTK_TT *>(SeparableBuffer), Mutex, WaitCondition,
              NumberOfFinishedThreads))
        ;
      default:
        vtkErrorMacro(<< "Execute (3D): Unknown ScalarType")
        return;
    }
  }
  else
  {
    vtkErrorMacro(<< "Execute: Dimensionality " << Dimensionality << " not supported!");
    return;
  }
}

}
