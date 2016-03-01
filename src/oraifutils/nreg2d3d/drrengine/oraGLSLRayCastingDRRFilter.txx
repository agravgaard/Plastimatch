//
#ifndef ORAGLSLRAYCASTINGDRRFILTER_TXX_
#define ORAGLSLRAYCASTINGDRRFILTER_TXX_

#include "oraGLSLRayCastingDRRFilter.h"

//ITK
#include <itkConceptChecking.h>
#include <itkMatrixOffsetTransformBase.h>
#include <itkMacro.h>
#include <itkRealTimeClock.h>
//ORAIFUTILS
#include "oraDRRFilter.h"

namespace ora
{

template<class TInputPixelType, class TOutputPixelType>
GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::GLSLRayCastingDRRFilter()
	: Superclass()
{
	this->m_LastPreProcessingTime = -1;
	this->m_LastPostProcessingTime = -1;
	this->m_LastGPUPureProcessingTime = -1;
	this->m_PreProcessingClock = ClockType::New();
	this->m_PostProcessingClock = ClockType::New();
	this->m_PureProcessingClock = ClockType::New();
	this->m_GeometryTransform = TransformType::New();
	this->m_CurrentGeometry = GeometryType::New();
	this->m_MappedInput = NULL;
	this->m_LastITFCalculationTimeStamp = 0;
	this->m_OffTheFlyITFMapping = false;
	this->m_LastInputVolumeTimeStamp = 0;
	//FIXME:
}

template<class TInputPixelType, class TOutputPixelType>
GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::~GLSLRayCastingDRRFilter()
{
	this->SetInput(NULL);
	this->m_PreProcessingClock = NULL;
	this->m_PostProcessingClock = NULL;
	this->m_PureProcessingClock = NULL;
	this->m_CurrentGeometry = NULL;
	this->m_MappedInput = NULL;
	//FIXME:
}

template<class TInputPixelType, class TOutputPixelType>
void GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::SetInput(InputImagePointer input)
{
	//FIXME:
}

template<class TInputPixelType, class TOutputPixelType>
void GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::SetOffTheFlyITFMapping(bool flag)
{
	//FIXME
}

template<class TInputPixelType, class TOutputPixelType>
void GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::GenerateOutputInformation()
{
	//FIXME:
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::DRRCanBeComputed() const
{
	//FIXME:
	return false;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsGPUBased() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsCPUBased() const
{
	return false;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsCPUMultiThreaded() const
{
	return false;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingITFOnTheFly() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingITFOffTheFly() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingRigidTransformation() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingAffineTransformation() const
{
	return false;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingElasticTransformation() const
{
	return false;
}

template<class TInputPixelType, class TOutputPixelType>
bool GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingDRRMasks() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
void GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::GenerateData()
{
	//FIXME:
}

template<class TInputPixelType, class TOutputPixelType>
void GLSLRayCastingDRRFilter<TInputPixelType, TOutputPixelType>::UpdateCurrentImagingGeometry()
{

}

}

#endif /* ORAGLSLRAYCASTINGDRRFILTER_TXX_ */
