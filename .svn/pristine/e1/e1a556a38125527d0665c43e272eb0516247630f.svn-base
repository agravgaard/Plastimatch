#include <stdio.h>
#include <limits>
#include "itkCastImageFilter.h"
#include "itkImageFileWriter.h"

#include "file_util.h"
#include "itk_image_save.h"
#include "logfile.h"
#include "path_util.h"
#include "plm_image.h"
#include "rt_study_metadata.h"
#include "string_util.h"

typedef itk::Image < float, 3 > FloatImageType;
typedef itk::Image < unsigned char, 3 > UCharImageType;

template<class T> 
void
my_itk_image_save (T image, const char* fname)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ImageFileWriter< ImageType >  WriterType;

    logfile_printf ("Trying to write image to %s\n", fname);

//    printf ("Hello!\n");

    typename WriterType::Pointer writer = WriterType::New();
    writer->SetInput (image);
    writer->SetFileName (fname);
    make_parent_directories (fname);

//    printf ("Maybe gonna update...\n");
    if (extension_is (fname, "nrrd")) {
	writer->SetUseCompression (true);
    }
    try {
	writer->Update();
    }
    catch (itk::ExceptionObject& excp) {
	printf ("ITK exception writing image file.\n");
	std::cout << excp << std::endl;
    }
//    printf ("Done ??\n");
}


int main 
(
    int argc,
    char* argv[]
)
{
    FloatImageType::Pointer image = FloatImageType::New();
    FloatImageType::RegionType rg;

    FloatImageType::IndexType start;
    FloatImageType::SizeType  size;

    size[0]  = 200;  // size along X
    size[1]  = 200;  // size along Y
    size[2]  = 200;  // size along Z

    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y
    start[2] =   0;  // first index on Z

    FloatImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( start );
    image->SetRegions( region );
    image->Allocate();

    FloatImageType::PixelType  initialValue = 0;
    image->FillBuffer( initialValue );

    typedef itk::CastImageFilter <
	FloatImageType, UCharImageType > ClampCastFilterType;
    ClampCastFilterType::Pointer caster = ClampCastFilterType::New();

    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    UCharImageType::Pointer tmp = caster->GetOutput();

//    std::cout << tmp;

//    itk_image_save (image, "foo.mha");
//    itk_image_save (tmp, "foo.mha");
//    itk_image_save_float (tmp, "foo.mha");
    my_itk_image_save (tmp, "foo.mha");

    Plm_image::Pointer pli = Plm_image::New (image);
    Rt_study_metadata rsm;
    rsm.set_patient_name ("Hello^World");
    rsm.set_patient_id ("ANON^Hello");

    pli->save_short_dicom ("itk_dicom", &rsm);
}
