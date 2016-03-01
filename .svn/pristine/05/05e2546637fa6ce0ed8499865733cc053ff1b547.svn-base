/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "itkImage.h"
#include "itkImageDuplicator.h"

#include "itk_image_clone.h"
#include "itk_image_type.h"

/* -----------------------------------------------------------------------
   Casting image types
   ----------------------------------------------------------------------- */
template<class T> 
T
itk_image_clone (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ImageDuplicator < ImageType > DuplicatorType;

    typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage (image);
    duplicator->Update();
    return duplicator->GetOutput();
}


/* Explicit instantiations */
template PLMBASE_API FloatImageType::Pointer itk_image_clone (FloatImageType::Pointer);
