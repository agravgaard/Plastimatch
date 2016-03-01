//
#ifndef _ORAONETOONEINTERPOLATEIMAGEFUNCTION_TXX_
#define _ORAONETOONEINTERPOLATEIMAGEFUNCTION_TXX_

#include "oraOneToOneInterpolateImageFunction.h"

namespace ora
{

template<class TInputImage, class TCoordRep>
OneToOneInterpolateImageFunction<TInputImage, TCoordRep>::OneToOneInterpolateImageFunction() :
  Superclass()
{

}

template<class TInputImage, class TCoordRep>
OneToOneInterpolateImageFunction<TInputImage, TCoordRep>::~OneToOneInterpolateImageFunction()
{

}

template<class TInputImage, class TCoordRep>
void OneToOneInterpolateImageFunction<TInputImage, TCoordRep>::PrintSelf(
    std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

template<class TInputImage, class TCoordRep>
typename OneToOneInterpolateImageFunction<TInputImage, TCoordRep>::OutputType OneToOneInterpolateImageFunction<
    TInputImage, TCoordRep>::EvaluateAtContinuousIndex(
    const ContinuousIndexType &index) const
{
  IndexType idx;
  // NOTE: casting leads to precision problems when converting:
  // point -> continuous index -> index
  // need rounding!
  idx.CopyWithRound(index);
  return static_cast<OutputType> (this->GetInputImage()->GetPixel(idx));
}

}

#endif // _ORAONETOONEINTERPOLATEIMAGEFUNCTION_TXX_