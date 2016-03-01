//
#ifndef ORAFLEXMAPCORRECTION_TXX_
#define ORAFLEXMAPCORRECTION_TXX_

#include "oraFlexMapCorrection.h"

namespace ora
{

template<class TPixelType, class TMaskPixelType>
FlexMapCorrection<TPixelType, TMaskPixelType>::FlexMapCorrection()
{
  m_LinacProjProps = NULL;
  m_CorrectedProjProps = GenericProjectionPropertiesType::New();
}

template<class TPixelType, class TMaskPixelType>
FlexMapCorrection<TPixelType, TMaskPixelType>::~FlexMapCorrection()
{
  m_LinacProjProps = NULL;
  m_CorrectedProjProps = NULL;
}

template<class TPixelType, class TMaskPixelType>
void FlexMapCorrection<TPixelType, TMaskPixelType>::PrintSelf(std::ostream& os,
    itk::Indent indent) const
{
  os << indent << "LINAC Projection Properties: "
      << m_LinacProjProps.GetPointer() << "\n";
  os << indent << "Corrected Projection Properties: "
      << m_CorrectedProjProps.GetPointer() << "\n";
}

}

#endif /* ORAFLEXMAPCORRECTION_TXX_ */
