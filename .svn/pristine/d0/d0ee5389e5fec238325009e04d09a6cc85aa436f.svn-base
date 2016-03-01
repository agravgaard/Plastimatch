/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

#include "dice_statistics.h"
#include "hausdorff_statistics.h"
#include "pcmd_resample.h"

void print_usage (void)
{
    printf ("Usage: dice_stats file1 file2\n");
}

/* For differing resolutions, resamples image_2 to image_1 */
void check_resolution (
    UCharImageType::Pointer *image_1,
    UCharImageType::Pointer *image_2
)
{
    if (
        (*image_1)->GetLargestPossibleRegion().GetSize() !=
        (*image_2)->GetLargestPossibleRegion().GetSize()
       )
    {
        Plm_image_header pih;
        Resample_parms parms;

        parms.interp_lin = false;
        pih.set_from_itk_image (*image_1);
        *image_2 = resample_image (
                    *image_2,
                    &pih,
                    parms.default_val,
                    parms.interp_lin
        );
    }
}


int main (int argc, char* argv[])
{
    if (argc != 3) {
	print_usage();
	exit (-1);
    }

    UCharImageType::Pointer image_1 = itk_image_load_uchar(argv[1], 0);
    UCharImageType::Pointer image_2 = itk_image_load_uchar(argv[2], 0);

    check_resolution (&image_1, &image_2);

    do_dice<unsigned char> (image_1, image_2, stdout);
    do_hausdorff<unsigned char> (image_1, image_2);
    do_contour_mean_dist<unsigned char> (image_1, image_2);
    return 0;
}
