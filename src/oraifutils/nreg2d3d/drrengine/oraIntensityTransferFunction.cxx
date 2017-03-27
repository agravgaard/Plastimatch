//
#include "oraIntensityTransferFunction.h"
#include <vector>
#include <algorithm>

namespace ora
{
IntensityTransferFunction::IntensityTransferFunction()
  : itk::Object()
{
   m_LastSortTimeStamp = 0;
}

IntensityTransferFunction::~IntensityTransferFunction()
{
  ;
}

void
IntensityTransferFunction::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Vector::const_iterator itIn = m_SupportingPoints.begin();
  do
  {
    os << indent << "Input threshold: " << (*itIn).inValue;
    os << " mapped to output threshold: " << (*itIn).outValue;
    os << std::endl; 
    ++itIn;
  } while (itIn != m_SupportingPoints.end());
}

void
IntensityTransferFunction::AddSupportingPoint(double in, double out)
{
  SupportingPointStruct point;
  point.inValue = in;
  point.outValue = out;
  m_SupportingPoints.push_back(point);
  this->Modified();
}

unsigned int
IntensityTransferFunction::GetNumberOfSupportingPoints() const
{
  return m_SupportingPoints.size();
}

void
IntensityTransferFunction::RemoveSupportingPoint(int i)
{
  if(i > (int)GetNumberOfSupportingPoints() || i < 0)
    return;
  Vector::iterator pos = m_SupportingPoints.begin() + i;
  m_SupportingPoints.erase(pos);
  this->Modified();
}

void
IntensityTransferFunction::RemoveAllSupportingPoints()
{
  m_SupportingPoints.clear();
  this->Modified();
}

const IntensityTransferFunction::Vector*
IntensityTransferFunction::GetSupportingPoints()
{
  return &m_SupportingPoints;
}

bool
IntensityTransferFunction::SupportingPointsSmallerThan(const SupportingPointStruct &sp1,
		const SupportingPointStruct &sp2)
{
    // check if supporting point sp1 invalue is smaller than
	// sp2 invalue
    return sp1.inValue < sp2.inValue;
}

double
IntensityTransferFunction::GetOutputIntensity(double in)
{
  if(m_SupportingPoints.empty())
    return 0;
  //check if the array needs sorting
  if(this->GetMTime() > m_LastSortTimeStamp)
  {
    m_LastSortTimeStamp = this->GetMTime();
    std::stable_sort(m_SupportingPoints.begin(), m_SupportingPoints.end(), SupportingPointsSmallerThan);
  }
  //don't check input that is not in range
  if(in < m_SupportingPoints[0].inValue || in >= m_SupportingPoints[(GetNumberOfSupportingPoints() - 1)].inValue)
    return 0;
  //search for the right thresholds and return transfered intensity
  for(unsigned int i = 1; i < m_SupportingPoints.size(); i++)
  {
    if(m_SupportingPoints[i].inValue > in)
    {
      double differenceThresholds = m_SupportingPoints[i].inValue - m_SupportingPoints[i - 1].inValue;
      double differenceIn = in - m_SupportingPoints[i - 1].inValue;
      return m_SupportingPoints[i - 1].outValue +
		  ((m_SupportingPoints[i].outValue - m_SupportingPoints[i - 1].outValue) *
				  (differenceIn/differenceThresholds));
    }
  }
  //if intensity equals intensity of last threshold
  if(in == m_SupportingPoints[(GetNumberOfSupportingPoints() - 1)].inValue)
    return m_SupportingPoints[(GetNumberOfSupportingPoints() - 1)].outValue;
  return 0;
}



}
