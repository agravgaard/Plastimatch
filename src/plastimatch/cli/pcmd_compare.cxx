/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <time.h>
#include "itkSubtractImageFilter.h"
#include "itkImageRegionIterator.h"

#include "itk_image_type.h"
#include "mha_io.h"
#include "pcmd_compare.h"
#include "plm_file_format.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "volume.h"

void
vf_analyze (Volume* vol1, Volume* vol2)
{
    int d, i, j, k, v;
    float* img1 = (float*) vol1->img;
    float* img2 = (float*) vol2->img;
    float max_vlen2 = 0.0f;
    int max_vlen_idx_lin = 0;
    int max_vlen_idx[3] = { 0, 0, 0 };

    for (v = 0, k = 0; k < vol1->dim[2]; k++) {
	for (j = 0; j < vol1->dim[1]; j++) {
	    for (i = 0; i < vol1->dim[0]; i++, v++) {
		float* dxyz1 = &img1[3*v];
		float* dxyz2 = &img2[3*v];
		float diff[3];
		float vlen2 = 0.0f;
		for (d = 0; d < 3; d++) {
		    diff[d] = dxyz2[d] - dxyz1[d];
		    vlen2 += diff[d] * diff[d];
		}
		if (vlen2 > max_vlen2) {
		    max_vlen2 = vlen2;
		    max_vlen_idx_lin = v;
		    max_vlen_idx[0] = i;
		    max_vlen_idx[1] = j;
		    max_vlen_idx[2] = k;
		}
	    }
	}
    }

    printf ("Max diff idx:  %4d %4d %4d [%d]\n", max_vlen_idx[0], max_vlen_idx[1], max_vlen_idx[2], max_vlen_idx_lin);
    printf ("Vol 1:         %10.3f %10.3f %10.3f\n", img1[3*max_vlen_idx_lin], img1[3*max_vlen_idx_lin+1], img1[3*max_vlen_idx_lin+2]);
    printf ("Vol 2:         %10.3f %10.3f %10.3f\n", img2[3*max_vlen_idx_lin], img2[3*max_vlen_idx_lin+1], img2[3*max_vlen_idx_lin+2]);
    printf ("Vec len diff:  %10.3f\n", sqrt(max_vlen2));
}

static void
vf_compare (Compare_parms* parms)
{
    int d;
    Volume *vol1, *vol2;

    vol1 = read_mha (parms->img_in_1_fn.c_str());
    if (!vol1) {
	fprintf (stderr, 
	    "Sorry, couldn't open file \"%s\" for read.\n", 
	    parms->img_in_1_fn.c_str());
	exit (-1);
    }
    if (vol1->pix_type != PT_VF_FLOAT_INTERLEAVED) {
	fprintf (stderr, "Sorry, file \"%s\" is not an "
	    "interleaved float vector field.\n", 
	    parms->img_in_1_fn.c_str());
	fprintf (stderr, "Type = %d\n", vol1->pix_type);
	exit (-1);
    }

    vol2 = read_mha (parms->img_in_2_fn.c_str());
    if (!vol2) {
	fprintf (stderr, 
	    "Sorry, couldn't open file \"%s\" for read.\n", 
	    parms->img_in_2_fn.c_str());
	exit (-1);
    }
    if (vol2->pix_type != PT_VF_FLOAT_INTERLEAVED) {
	fprintf (stderr, "Sorry, file \"%s\" is not an "
	    "interleaved float vector field.\n", 
	    parms->img_in_2_fn.c_str());
	fprintf (stderr, "Type = %d\n", vol2->pix_type);
	exit (-1);
    }

    for (d = 0; d < 3; d++) {
	if (vol1->dim[d] != vol2->dim[d]) {
	    fprintf (stderr, "Can't compare.  "
		"Files have different dimensions.\n");
	    exit (-1);
	}
    }

    vf_analyze (vol1, vol2);

    delete vol1;
    delete vol2;
}

static void
img_compare (Compare_parms* parms)
{
    Plm_image::Pointer img1, img2;

    img1 = plm_image_load_native (parms->img_in_1_fn);
    if (!img1) {
	print_and_exit ("Error: could not open '%s' for read\n",
	    parms->img_in_1_fn.c_str());
    }
    img2 = plm_image_load_native (parms->img_in_2_fn);
    if (!img2) {
	print_and_exit ("Error: could not open '%s' for read\n",
	    parms->img_in_2_fn.c_str());
    }

    if (!Plm_image::compare_headers (img1, img2)) {
	print_and_exit ("Error: image sizes do not match\n");
    }

    FloatImageType::Pointer fi1 = img1->itk_float ();
    FloatImageType::Pointer fi2 = img2->itk_float ();

    typedef itk::SubtractImageFilter< FloatImageType, FloatImageType, 
				      FloatImageType > SubtractFilterType;
    SubtractFilterType::Pointer sub_filter = SubtractFilterType::New();

    sub_filter->SetInput1 (fi1);
    sub_filter->SetInput2 (fi2);

    try {
	sub_filter->Update();
    } catch (itk::ExceptionObject & excep) {
	std::cerr << "ITK exception caught: " << excep << std::endl;
	exit (-1);
    }
    FloatImageType::Pointer diff = sub_filter->GetOutput ();

    typedef itk::ImageRegionConstIterator < FloatImageType > FloatIteratorType;
    FloatIteratorType it (diff, diff->GetRequestedRegion ());

    int first = 1;
    float min_val = 0.0;
    float max_val = 0.0;
    int num = 0, num_dif = 0;
    double ave = 0.0;
    double mae = 0.0;
    double mse = 0.0;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
	float v = it.Get();
	if (first) {
	    min_val = v;
	    max_val = v;
	    first = 0;
	}
	if (min_val > v)     min_val = v;
	if (max_val < v)     max_val = v;
	if (v != 0.0)        num_dif ++;
	ave += v;
	mae += fabs (v);
	mse += (v * v);
	num ++;
    }

    printf ("MIN %f AVE %f MAX %f\n"
	"MAE %f MSE %f\n"
	"DIF %d NUM %d\n",
	min_val, 
	(float) (ave / num), 
	max_val, 
	(float) (mae / num), 
	(float) (mse / num), 
	num_dif, 
	num);
}

static void
compare_main (Compare_parms* parms)
{
    Plm_file_format file_type_1, file_type_2;

    /* What is the input file type? */
    file_type_1 = plm_file_format_deduce (parms->img_in_1_fn);
    file_type_2 = plm_file_format_deduce (parms->img_in_2_fn);

    if (file_type_1 == PLM_FILE_FMT_VF 
	&& file_type_2 == PLM_FILE_FMT_VF)
    {
	vf_compare (parms);
    }
    else
    {
	img_compare (parms);
    }
}

static void
compare_print_usage (void)
{
    printf ("Usage: plastimatch compare file_1 file_2\n"
	    );
    exit (-1);
}

static void
compare_parse_args (Compare_parms* parms, int argc, char* argv[])
{
    if (argc != 4) {
	compare_print_usage ();
    }
    
    parms->img_in_1_fn = argv[2];
    parms->img_in_2_fn = argv[3];
}

void
do_command_compare (int argc, char *argv[])
{
    Compare_parms parms;
    
    compare_parse_args (&parms, argc, argv);

    compare_main (&parms);
}
