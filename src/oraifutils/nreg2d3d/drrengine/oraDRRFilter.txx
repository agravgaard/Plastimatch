//
#ifndef ORADRRFILTER_TXX_
#define ORADRRFILTER_TXX_

#include "oraDRRFilter.h"
//ITK
#include <itkMath.h>

namespace ora
{


template<class TInputPixelType, class TOutputPixelType>
DRRFilter<TInputPixelType, TOutputPixelType>::DRRFilter()
{
  this->m_NumberOfIndependentOutputs = 0;
  this->SetNumberOfIndependentOutputs(1); // sets geometries, masks to NULL
  this->m_CurrentDRROutputIndex = 0;
  this->SetInput(NULL);
  this->SetITF(NULL);
  this->SetTransform(NULL);
}

template<class TInputPixelType, class TOutputPixelType>
DRRFilter<TInputPixelType, TOutputPixelType>::~DRRFilter()
{
  this->m_CurrentDRROutputIndex = 0;
  this->SetNumberOfIndependentOutputs(1);
  this->SetInput(NULL);
  this->SetITF(NULL);
  this->SetTransform(NULL);
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "\nora::DRRFilter ------------------------------------------------------------\n\n";
  indent = indent.GetNextIndent();
  os << indent << "NumberOfIndependentOutputs: " << this->m_NumberOfIndependentOutputs << "\n";
  os << indent << "CurrentDRROutputIndex: " << this->m_CurrentDRROutputIndex << "\n";
  os << indent << "Input: " << this->m_Input.GetPointer() << "\n";
  os << indent << "Transform: " << this->m_Transform.GetPointer() << "\n";
  os << indent << "ITF: " << this->m_ITF.GetPointer() << "\n";
  os << indent << "Masks: N=" << this->m_Masks.size() << "\n";
  os << indent << "Geometries: N=" << this->m_Geometries.size() << "\n";
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::SetInput(
		InputImagePointer input)
{
  if (input == this->GetInput())
    return;
  this->m_Input = NULL;
  this->Superclass::SetInput(NULL); // throws modified
  if (!input)
  	return;
  this->Superclass::SetInput(input); // throws modified
  this->m_Input = input;
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::SetProjectionGeometry(int index, GeometryPointer geometry)
{
  if(index > this->m_NumberOfIndependentOutputs || index < 0)
  	return;
  this->m_Geometries[index] = NULL;
  this->m_Geometries[index] = geometry;
  this->Modified();
}

template<class TInputPixelType, class TOutputPixelType>
typename DRRFilter<TInputPixelType, TOutputPixelType>::GeometryPointer
DRRFilter<TInputPixelType, TOutputPixelType>::GetProjectionGeometry(
		int index) const
{
  if (index >= this->m_NumberOfIndependentOutputs || index < 0)
  	return NULL;
  return this->m_Geometries[index];
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::GenerateOutputInformation()
{
  if (this->m_NumberOfIndependentOutputs <= 0 || !this->DRRCanBeComputed())
    return;
  this->EnsureCurrentImageIsCorrect(this->m_Geometries[this->m_CurrentDRROutputIndex]); // fix image size and further props
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::GenerateInputRequestedRegion()
{
  this->Superclass::GenerateInputRequestedRegion();

  // get pointer to the input
  if (!this->GetInput())
    return;
  InputImagePointer inputPtr = const_cast<InputImageType*> (this->GetInput());

  // request complete input image
  inputPtr->SetRequestedRegionToLargestPossibleRegion();
}

template<class TInputPixelType, class TOutputPixelType>
unsigned long DRRFilter<TInputPixelType, TOutputPixelType>::GetMTime(void) const
{
  unsigned long latest = this->Superclass::GetMTime();
	if (this->m_Transform)
	{
		if (latest < this->m_Transform->GetMTime())
			latest = this->m_Transform->GetMTime();
	}
	if (this->m_Input)
	{
		if (latest < this->m_Input->GetMTime())
			latest = this->m_Input->GetMTime();
	}
  if (this->m_ITF)
  {
    if (latest < this->m_ITF->GetMTime())
      latest = this->m_ITF->GetMTime();
  }
  if(this->m_Geometries[this->m_CurrentDRROutputIndex])
  {
    if (latest < this->m_Geometries[this->m_CurrentDRROutputIndex]->GetMTime())
      latest = this->m_Geometries[this->m_CurrentDRROutputIndex]->GetMTime();
  }
  if(this->m_Masks[this->m_CurrentDRROutputIndex])
  {
    if (latest < this->m_Masks[this->m_CurrentDRROutputIndex]->GetMTime())
      latest = this->m_Masks[this->m_CurrentDRROutputIndex]->GetMTime();
  }
  return latest;
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::SetTransform(
		TransformType *transform)
{
  if (transform == this->m_Transform)
    return;
  this->m_Transform = transform;
  this->Modified();
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::SetNumberOfIndependentOutputs(
		int numOutputs)
{
  if (numOutputs <= 0)
    return;

  if (numOutputs != this->m_NumberOfIndependentOutputs)
  {
    bool resizeOutputsBefore = (numOutputs > this->m_NumberOfIndependentOutputs);
    if (resizeOutputsBefore)
    {
      this->SetNumberOfOutputs(numOutputs);
      this->SetNumberOfRequiredOutputs(numOutputs);
    }
    // adjust the size of outputs (preserve existing images):
    int currNoOutputs = this->m_NumberOfIndependentOutputs;
    while (currNoOutputs > numOutputs)
    {
      // remove unnecessary filter output:
      int idx = currNoOutputs - 1;
      OutputImagePointer output = this->GetOutput(idx);
      this->RemoveOutput(output);
      this->m_Geometries[idx] = NULL;
      this->m_Geometries.pop_back();
      this->m_Masks[idx] = NULL;
      this->m_Masks.pop_back();
      currNoOutputs--;
    }
    while (currNoOutputs < numOutputs)
    {
      // add required filter output:
      int idx = currNoOutputs;
      OutputImagePointer output =
          static_cast<OutputImageType *>(this->MakeOutput(idx).GetPointer());
      this->SetNthOutput(idx, output.GetPointer());
      this->m_Geometries.push_back(NULL); //no projection set by default
      this->m_Masks.push_back(NULL); // no mask by default
      currNoOutputs++;
    }
    if (!resizeOutputsBefore)
    {
      this->SetNumberOfOutputs(numOutputs);
      this->SetNumberOfRequiredOutputs(numOutputs);
    }
    this->m_NumberOfIndependentOutputs = numOutputs;
    if (this->m_CurrentDRROutputIndex >= numOutputs)
      this->SetCurrentDRROutputIndex(0);
    this->Modified(); // image size fixed during EnsureCurrentImageIsCorrect()
  }
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::RemoveDRROutput(std::size_t index)
{
  if(index >= (unsigned int)this->m_NumberOfIndependentOutputs)
    return;
  this->m_Masks[index] = NULL;
  this->m_Geometries[index] = NULL;
  this->m_Geometries.erase(this->m_Geometries.begin()+index);
  this->m_Masks.erase(this->m_Masks.begin()+index);
  this->m_NumberOfIndependentOutputs--;
  this->Modified();
}


template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::SetCurrentDRROutputIndex(
		int currIndex)
{
  if (currIndex < 0 || currIndex >= this->m_NumberOfIndependentOutputs)
    return;

  if (this->m_CurrentDRROutputIndex != currIndex)
  {
    this->m_CurrentDRROutputIndex = currIndex;
    this->Modified(); // image size fixed during EnsureCurrentImageIsCorrect()
  }
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::SetDRRMask(
		int index, MaskImagePointer mask)
{
	if(index > this->m_NumberOfIndependentOutputs || index < 0)
		return;
	this->m_Masks[index] = NULL;
	this->m_Masks[index] = mask;
	this->Modified();
}

template<class TInputPixelType, class TOutputPixelType>
typename DRRFilter<TInputPixelType, TOutputPixelType>::MaskImagePointer DRRFilter<TInputPixelType, TOutputPixelType>::GetDRRMask(int index) const
{
	if(index >= this->m_NumberOfIndependentOutputs || index < 0)
		return NULL;
	return this->m_Masks[index];
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::EnlargeOutputRequestedRegion(
		itk::DataObject *data)
{
  this->Superclass::EnlargeOutputRequestedRegion(data);
  data->SetRequestedRegionToLargestPossibleRegion(); // largest possible!
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::GenerateOutputRequestedRegion(
		itk::DataObject *data)
{
  for (int x = 0; x < this->m_NumberOfIndependentOutputs; x++)
  {
    OutputImagePointer output = this->GetOutput(x);
    if (output)
    {
      output->SetRequestedRegionToLargestPossibleRegion();
      output->SetBufferedRegion(output->GetLargestPossibleRegion());
    }
  }
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::GenerateData()
{
	this->Superclass::GenerateData(); // let the superclass coordinate ...
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::ThreadedGenerateData(
		const OutputImageRegionType& outputRegionForThread, int threadId)
{
  // multi-threaded CPU processing code here (sub-classes) ...
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::BeforeThreadedGenerateData()
{
	// CPU pre-processing code here (sub-classes) ...
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::AfterThreadedGenerateData()
{
	// CPU post-processing code here (sub-classes) ...
}

template<class TInputPixelType, class TOutputPixelType>
void DRRFilter<TInputPixelType, TOutputPixelType>::EnsureCurrentImageIsCorrect(typename DRRFilter<TInputPixelType, TOutputPixelType>::GeometryPointer geometry)
{
  if (this->m_CurrentDRROutputIndex < 0 ||
  		this->m_CurrentDRROutputIndex >= this->m_NumberOfIndependentOutputs)
    return;

  OutputImagePointer output = this->GetOutput(this->m_CurrentDRROutputIndex);

  // ensure that the ouput image size is correct:
  typename OutputImageType::RegionType reg = output->GetLargestPossibleRegion();
  if (reg.GetSize()[0] != (unsigned int)geometry->GetDetectorSize()[0] ||
  		reg.GetSize()[1] != (unsigned int)geometry->GetDetectorSize()[1])
  {
  	typename OutputImageType::SizeType regSz;
  	regSz[0] = geometry->GetDetectorSize()[0];
  	regSz[1] = geometry->GetDetectorSize()[1];
  	regSz[2] = 1; // 3rd dimension is irrelevant
  	typename OutputImageType::IndexType regIdx;
  	regIdx.Fill(0);
  	reg.SetSize(regSz);
  	reg.SetIndex(regIdx);
  	output->SetRegions(reg);
  	output->Allocate();
  }
  // ensure that the output image spacing is correct:
  if (output->GetSpacing()[0] != geometry->GetDetectorPixelSpacing()[0] ||
  		output->GetSpacing()[1] != geometry->GetDetectorPixelSpacing()[1])
  {
  	typename OutputImageType::SpacingType sp;
  	sp[0] = geometry->GetDetectorPixelSpacing()[0];
  	sp[1] = geometry->GetDetectorPixelSpacing()[1];
  	sp[2] = 1.0; // 3rd dimension is irrelevant
  	output->SetSpacing(sp);
  }
  // properties which change with volume orientation / spatial transformation:
  // - ensure that the output image origin is correct:
  if (output->GetOrigin()[0] != geometry->GetDetectorOrigin()[0] ||
      output->GetOrigin()[1] != geometry->GetDetectorOrigin()[1] ||
      output->GetOrigin()[2] != geometry->GetDetectorOrigin()[2])
  {
    typename OutputImageType::PointType oorig;
    oorig[0] = geometry->GetDetectorOrigin()[0];
    oorig[1] = geometry->GetDetectorOrigin()[1];
    oorig[2] = geometry->GetDetectorOrigin()[2];
  	output->SetOrigin(oorig);
  }
  // - ensure that the image direction is OK:
  typename OutputImageType::DirectionType orient = output->GetDirection();
  VectorType v1;
  v1[0] = orient[0][0];
  v1[1] = orient[1][0];
  v1[2] = orient[2][0];
  VectorType v2;
  v2[0] = orient[0][1];
  v2[1] = orient[1][1];
  v2[2] = orient[2][1];

  if (v1[0] != geometry->GetDetectorRowOrientation()[0] ||
      v1[1] != geometry->GetDetectorRowOrientation()[1] ||
      v1[2] != geometry->GetDetectorRowOrientation()[2] ||
      v2[0] != geometry->GetDetectorColumnOrientation()[0] ||
      v2[1] != geometry->GetDetectorColumnOrientation()[1] ||
      v2[2] != geometry->GetDetectorColumnOrientation()[2])
  {
  	orient[0][0] = geometry->GetDetectorRowOrientation()[0]; // v1
  	orient[1][0] = geometry->GetDetectorRowOrientation()[1];
  	orient[2][0] = geometry->GetDetectorRowOrientation()[2];
  	v1[0] = orient[0][0];
  	v1[1] = orient[1][0];
  	v1[2] = orient[2][0];
  	orient[0][1] = geometry->GetDetectorColumnOrientation()[0]; // v2
  	orient[1][1] = geometry->GetDetectorColumnOrientation()[1];
  	orient[2][1] = geometry->GetDetectorColumnOrientation()[2];
    v2[0] = orient[0][1];
    v2[1] = orient[1][1];
    v2[2] = orient[2][1];
  	VectorType v3;
  	v3[0] = v1[1] * v2[2] - v1[2] * v2[1];
  	v3[1] = v1[2] * v2[0] - v1[0] * v2[2];
  	v3[2] = v1[0] * v2[1] - v1[1] * v2[0];
  	orient[0][2] = v3[0]; // v1 x v2
  	orient[1][2] = v3[1];
  	orient[2][2] = v3[2];
  	output->SetDirection(orient);
  }
}

template<class TInputPixelType, class TOutputPixelType>
int DRRFilter<TInputPixelType, TOutputPixelType>::SplitRequestedRegion(int i,
		int num, OutputImageRegionType& splitRegion)
{
	// NOTE: The implementation is more or less equal to the itk::ImageSource
	// implementation, but operates on the current output!

  // Get the CURRENT output pointer:
  OutputImageType *outputPtr = this->GetOutput(this->m_CurrentDRROutputIndex);
  const typename OutputImageType::SizeType &requestedRegionSize =
  		outputPtr->GetRequestedRegion().GetSize();

  int splitAxis;
  typename OutputImageType::IndexType splitIndex;
  typename OutputImageType::SizeType splitSize;

  // Initialize the splitRegion to the output requested region
  splitRegion = outputPtr->GetRequestedRegion();
  splitIndex = splitRegion.GetIndex();
  splitSize = splitRegion.GetSize();

  // split on the outermost dimension available
  splitAxis = outputPtr->GetImageDimension() - 1;
  while (requestedRegionSize[splitAxis] == 1)
  {
    --splitAxis;
    if (splitAxis < 0) // cannot split
      return 1;
  }

  // determine the actual number of pieces that will be generated
  typename OutputImageType::SizeType::SizeValueType range = requestedRegionSize[splitAxis];
  int valuesPerThread = itk::Math::Ceil<int>(range / (double)num);
  int maxThreadIdUsed = itk::Math::Ceil<int>(range / (double)valuesPerThread) - 1;

  // Split the region
  if (i < maxThreadIdUsed)
  {
    splitIndex[splitAxis] += i * valuesPerThread;
    splitSize[splitAxis] = valuesPerThread;
  }
  if (i == maxThreadIdUsed)
  {
    splitIndex[splitAxis] += i * valuesPerThread;
    // last thread needs to process the "rest" dimension being split
    splitSize[splitAxis] = splitSize[splitAxis] - i * valuesPerThread;
  }

  // set the split region ivars
  splitRegion.SetIndex(splitIndex);
  splitRegion.SetSize(splitSize);

  return (maxThreadIdUsed + 1);
}

}

#endif /* ORADRRFILTER_TXX_ */
