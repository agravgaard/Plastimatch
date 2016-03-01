

#include "itkRTIImageIOFactory.h"
#include "itkRTIImageIO.h"

#include <itkCreateObjectFunction.h>
#include <itkVersion.h>


namespace itk
{


RTIImageIOFactory
::RTIImageIOFactory()
{
  this->RegisterOverride("itkImageIOBase", "itkRTIImageIO",
    "RTI Image IO", 1, CreateObjectFunction<RTIImageIO>::New());
}

RTIImageIOFactory
::~RTIImageIOFactory()
{
  ;
}

const char *RTIImageIOFactory::GetITKSourceVersion(void) const
{
  return ITK_SOURCE_VERSION;
}

const char* RTIImageIOFactory::GetDescription(void) const
{
  return "RTI ImageIO Factory enables open radART RTI images <-> ITK comm.";
}


}
