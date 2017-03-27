#ifndef ORAINTENSITYTRANSFERFUNCTION_TXX_
#define ORAINTENSITYTRANSFERFUNCTION_TXX_

#include "oraIntensityTransferFunction.h"
#include <vector>
#include <algorithm>

namespace ora
{

template<typename TPixel> inline TPixel
IntensityTransferFunction::MapInValue(TPixel in)
{
  return static_cast<TPixel>(GetOutputIntensity(static_cast<double>(in)));
}

template<typename TPixel> inline void
IntensityTransferFunction::MapInValue(TPixel in, TPixel &out)
{
  out = static_cast<TPixel>(GetOutputIntensity(static_cast<double>(in)));
  return;
}

template<typename TPixel> inline void
IntensityTransferFunction::MapInValues(int count, TPixel *ins, TPixel *outs)
{
  for(int i = 0; i < count; i++)
    outs[i] = static_cast<TPixel>(GetOutputIntensity(static_cast<double>(ins[i])));
  return;
}

}

#endif /* ORAINTENSITYTRANSFERFUNCTION_TXX_ */
