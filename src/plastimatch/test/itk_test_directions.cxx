#include <stdio.h>
#include <limits>
#include "itkCastImageFilter.h"
#include "itkImageFileWriter.h"

#include "file_util.h"
#include "itk_image_save.h"
#include "logfile.h"
#include "path_util.h"
#include "plm_image.h"
#include "rt_study.h"
#include "string_util.h"
#include "volume.h"

typedef itk::Image < float, 3 > FloatImageType;

int main 
(
    int argc,
    char* argv[]
)
{
    FloatImageType::Pointer image = FloatImageType::New();
    FloatImageType::RegionType rg;
    FloatImageType::SizeType size;
    FloatImageType::PointType og;
    FloatImageType::SpacingType sp;
    FloatImageType::DirectionType itk_dc;

    size[0]  = 2;
    size[1]  = 2;
    size[2]  = 2;

    og[0] =   0;
    og[1] =   0;
    og[2] =   0;

    sp[0] =   1;
    sp[1] =   10;
    sp[2] =   100;

    itk_dc[0][0] = 1;
    itk_dc[0][1] = 0;
    itk_dc[0][2] = 0;
    itk_dc[1][0] = 0;
    itk_dc[1][1] = .7071;
    itk_dc[1][2] = -.7071;
    itk_dc[2][0] = 0;
    itk_dc[2][1] = .7071;
    itk_dc[2][2] = .7071;

    FloatImageType::RegionType region;
    region.SetSize (size);
    image->SetRegions (region);
    image->SetOrigin (og);
    image->SetSpacing (sp);
    image->SetDirection (itk_dc);
    image->Allocate();

    FloatImageType::PixelType  initialValue = 0;
    image->FillBuffer( initialValue );

    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > IteratorType;
    IteratorType it (image, image->GetLargestPossibleRegion());
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        FloatImageType::IndexType idx = it.GetIndex ();
        FloatPoint3DType phys;
        image->TransformIndexToPhysicalPoint (idx, phys);
        std::cout << idx << " " << phys << "\n";
    }

    Plm_image::Pointer pli = Plm_image::New (image);
    Volume::Pointer v = pli->get_volume_float();
    plm_long ijk[3];
    float proj_ijk[3];
    float xyz[3];
    LOOP_Z (ijk, xyz, v) {
        LOOP_Y (ijk, xyz, v) {
            LOOP_X (ijk, xyz, v) {

                proj_ijk[0] = PROJECT_X (xyz, v->proj);
                proj_ijk[1] = PROJECT_Y (xyz, v->proj);
                proj_ijk[2] = PROJECT_Z (xyz, v->proj);

                printf ("[%d %d %d] %f %f %f -> [%f %f %f]\n", 
                    (int) ijk[0], (int) ijk[1], (int) ijk[2], 
                    xyz[0], xyz[1], xyz[2], 
                    proj_ijk[0], proj_ijk[1], proj_ijk[2]);
            }
        }
    }

    pli->save_image ("itk_test_directions.mha");
    pli->save_image ("itk_test_directions.nrrd");

    Rt_study rt;
    rt.set_image (pli);
    rt.save_dicom ("itk_test_directions_dicom");
}
