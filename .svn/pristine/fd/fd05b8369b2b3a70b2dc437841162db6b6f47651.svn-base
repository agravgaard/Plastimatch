/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include "itkGaborImageSource.h"
#include "itkGaborKernelFunction.h"
#include "itkImage.h"

#include "itk_gabor.h"
#include "itk_image_save.h"

void 
itk_gabor (FloatImageType::Pointer image)
{
    typedef itk::GaborImageSource< FloatImageType > GaborSourceType;
    GaborSourceType::Pointer GaborKernelImage = GaborSourceType::New();

    GaborKernelImage->Update();
    FloatImageType::Pointer img = GaborKernelImage->GetOutput();

    itk_image_save (img, "tmp.mha");
}

FloatImageType::Pointer
itk_gabor_create (const Plm_image_header *pih)
{
    typedef itk::GaborImageSource< FloatImageType > GaborSourceType;
    GaborSourceType::Pointer gabor = GaborSourceType::New();

    //gabor->SetSize (pih->GetSize());
    //gabor->SetSpacing (pih->m_spacing);
    //gabor->SetOrigin (pih->m_origin);
    
#if defined (commentout)
#endif
    FloatImageType::PointType origin;
    origin.Fill (15);
    gabor->SetOrigin (origin);
    FloatImageType::SpacingType spacing;
    spacing.Fill (0.25);
    gabor->SetSpacing (spacing);

    gabor->Update();
    FloatImageType::Pointer img = gabor->GetOutput();
    return img;
}
