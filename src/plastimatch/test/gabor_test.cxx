/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>

#include "gabor.h"

int 
main (int argc, char *argv[])
{
#if defined (commentout)
    if (argc != 2) {
        printf ("Usage: gabor_test image\n");
        exit (-1);
    }
    FloatImageType::Pointer image = itk_image_load_float (argv[1], 0);
#endif

#if defined (commentout)
    /* Anti-functional itk gabor program */
    plm_long dim[3] = { 11, 11, 11 };
    float origin[3] = { -2.5, -2.5, -2.5 };
    float spacing[3] = { .5, .5, .5 };
    Plm_image_header pih (dim, origin, spacing);
    FloatImageType::Pointer g_img = itk_gabor_create (&pih);

    itk_image_save (g_img, "tmp.mha");
#endif

    

    return 0;
}
