#include <stdio.h>
#include "gamma_dose_comparison.h"

int main (int argc, char* argv[])
{
    Gamma_dose_comparison gdc;

    gdc.set_reference_image ("image1.nrrd");
    gdc.set_compare_image ("image2.nrrd");
    gdc.run();

    FloatImageType::Pointer g = gdc.get_gamma_image_itk();
}
