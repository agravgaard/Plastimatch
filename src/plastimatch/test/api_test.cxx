/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "itk_image.h"
#include "rt_study.h"

template<class T> typename T::Pointer 
make_image (float value)
{
    typename T::Pointer image = T::New();
    typename T::IndexType start;
    typename T::SizeType size;
    typename T::RegionType region;
    typename T::PointType origin;
    typename T::SpacingType sp;
    typename T::PixelType fill_value = (typename T::PixelType) value;

    for (int d = 0; d < 3; d++) {
        start[d] = 0;
        size[d] = 100;
        origin[d] = 0;
        sp[d] = 1;
    }
    region.SetSize (size);
    region.SetIndex (start);
    image->SetRegions (region);
    image->SetOrigin (origin);
    image->SetSpacing (sp);
    image->Allocate ();
    image->FillBuffer (fill_value);

    return image;
}

int
main (int argc, char *argv[])
{
    /* Make some synthetic data */
    ShortImageType::Pointer image = make_image<ShortImageType> (100);
    FloatImageType::Pointer dose = make_image<FloatImageType> (50);
    UCharImageType::Pointer body = make_image<UCharImageType> (0);
    UCharImageType::Pointer tumor = make_image<UCharImageType> (0);
    for (unsigned int s = 20; s < 80; s++) {
        for (unsigned int r = 20; r < 80; r++) {
            for (unsigned int c = 20; c < 80; c++) {
                UCharImageType::IndexType idx;
                idx[0] = s; idx[1] = r; idx[2] = c;
                body->SetPixel(idx, 1);
                if (s > 40 && s < 60 && r > 40 && r < 60 && c > 40 && c < 60) {
                    tumor->SetPixel(idx, 1);
                }
            }
        }
    }
 
    /* Add it into the Rt_study structure */
    Rt_study::Pointer rt_study = Rt_study::New ();
    rt_study->set_image (image);
    rt_study->add_structure (body, "Body", "255\\0\\0");
    rt_study->add_structure (tumor);
    rt_study->set_dose (dose);

    /* Write dicom output to a directory */
    rt_study->save_dicom ("api_test_1");
    
    return 0;
}

