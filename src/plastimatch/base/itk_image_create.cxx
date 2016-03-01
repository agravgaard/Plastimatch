/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "itkImage.h"

#include "itk_image_type.h"
#include "itk_image_create.h"
#include "plm_image_header.h"

/* -----------------------------------------------------------------------
   Create a new image
   ----------------------------------------------------------------------- */
template<class T> 
typename itk::Image<T,3>::Pointer
itk_image_create (const Plm_image_header& pih)
{
    typename itk::Image<T,3>::Pointer img = itk::Image<T,3>::New ();
    img->SetOrigin (pih.GetOrigin());
    img->SetSpacing (pih.GetSpacing());
    img->SetDirection (pih.GetDirection());
    img->SetRegions (pih.GetRegion());
    img->Allocate ();
    img->FillBuffer (static_cast<T>(0));

    return img;
}

template<>
DeformationFieldType::Pointer
itk_image_create<FloatVector3DType> (const Plm_image_header& pih)
{
    DeformationFieldType::Pointer img = DeformationFieldType::New ();
    img->SetOrigin (pih.GetOrigin());
    img->SetSpacing (pih.GetSpacing());
    img->SetDirection (pih.GetDirection());
    img->SetRegions (pih.GetRegion());
    img->Allocate ();
    FloatVector3DType v;
    v.Fill (0);
    img->FillBuffer (v);
    return img;
}

/* Explicit instantiations */
template PLMBASE_API itk::Image<unsigned char,3>::Pointer itk_image_create<unsigned char> (const Plm_image_header& pih);
template PLMBASE_API itk::Image<float,3>::Pointer itk_image_create<float> (const Plm_image_header& pih);
//template PLMBASE_API itk::Image<FloatVector3DType,3>::Pointer itk_image_create<FloatVector3DType> (const Plm_image_header& pih);
