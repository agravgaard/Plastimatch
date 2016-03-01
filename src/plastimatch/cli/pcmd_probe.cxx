/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdlib.h>
#include "itkLinearInterpolateImageFunction.h"
#include "itkVectorLinearInterpolateImageFunction.h"
#include "itkContinuousIndex.h"

#include "itk_image_load.h"
#include "itk_image_type.h"
#include "pcmd_probe.h"
#include "plm_clp.h"
#include "plm_file_format.h"
#include "string_util.h"

class Probe_parms {
public:
    std::string input_fn;
    std::string index_string;
    std::string location_string;

public:
    Probe_parms () {
    }
};

static void
probe_img_main (Probe_parms *parms)
{
    FloatImageType::Pointer img = itk_image_load_float (
	parms->input_fn, 0);
    FloatImageType::RegionType rg = img->GetLargestPossibleRegion ();
    
    typedef itk::LinearInterpolateImageFunction < FloatImageType, 
	float > InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    interpolator->SetInputImage (img);

    std::vector<float> index_list = parse_float3_string (
	parms->index_string);
    for (unsigned int i = 0; i < index_list.size() / 3; i++) {
	itk::ContinuousIndex<float, 3> cindex;
	cindex[0] = index_list[i*3+0];
	cindex[1] = index_list[i*3+1];
	cindex[2] = index_list[i*3+2];

	FloatPoint3DType point;
	img->TransformContinuousIndexToPhysicalPoint (cindex, point);
	printf ("%4d: %7.2f, %7.2f, %7.2f; %7.2f, %7.2f, %7.2f; ", 
	    i, cindex[0], cindex[1], cindex[2], 
	    point[0], point[1], point[2]);
	if (cindex[0] < 0 || cindex[0] >= (int) rg.GetSize(0)
	    || cindex[1] < 0 || cindex[1] >= (int) rg.GetSize(1)
	    || cindex[2] < 0 || cindex[2] >= (int) rg.GetSize(2))
	{
	    printf ("N/A\n");
	} else {
	    InterpolatorType::OutputType pixel_value 
		= interpolator->EvaluateAtContinuousIndex (cindex);
	    printf ("%f\n", pixel_value);
	}
    }

    std::vector<float> location_list = parse_float3_string (
	parms->location_string);
    for (unsigned int i = 0; i < location_list.size() / 3; i++) {
	FloatPoint3DType point;
	point[0] = location_list[i*3+0];
	point[1] = location_list[i*3+1];
	point[2] = location_list[i*3+2];

	itk::ContinuousIndex<float, 3> cindex;
	img->TransformPhysicalPointToContinuousIndex (point, cindex);
	printf ("%4d: %7.2f, %7.2f, %7.2f; %7.2f, %7.2f, %7.2f; ", 
	    (int) (index_list.size() / 3) + i, 
	    cindex[0], cindex[1], cindex[2], 
	    point[0], point[1], point[2]);
	if (cindex[0] < 0 || cindex[0] >= (int) rg.GetSize(0)
	    || cindex[1] < 0 || cindex[1] >= (int) rg.GetSize(1)
	    || cindex[2] < 0 || cindex[2] >= (int) rg.GetSize(2))
	{
	    printf ("N/A\n");
	} else {
	    InterpolatorType::OutputType pixel_value 
		= interpolator->EvaluateAtContinuousIndex (cindex);
	    printf ("%f\n", pixel_value);
	}
    }
}

static void
probe_vf_main (Probe_parms *parms)
{
    DeformationFieldType::Pointer img = itk_image_load_float_field (
	parms->input_fn);
    DeformationFieldType::RegionType rg = img->GetLargestPossibleRegion ();
    
    typedef itk::VectorLinearInterpolateImageFunction < DeformationFieldType, 
	float > InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    interpolator->SetInputImage (img);

    std::vector<float> index_list = parse_float3_string (
	parms->index_string);
    for (unsigned int i = 0; i < index_list.size() / 3; i++) {
	itk::ContinuousIndex<float, 3> cindex;
	cindex[0] = index_list[i*3+0];
	cindex[1] = index_list[i*3+1];
	cindex[2] = index_list[i*3+2];

	FloatPoint3DType point;
	img->TransformContinuousIndexToPhysicalPoint (cindex, point);
	printf ("%4d: %7.2f, %7.2f, %7.2f; %7.2f, %7.2f, %7.2f; ", 
	    i, cindex[0], cindex[1], cindex[2], 
	    point[0], point[1], point[2]);
	if (cindex[0] < 0 || cindex[0] >= (int) rg.GetSize(0)
	    || cindex[1] < 0 || cindex[1] >= (int) rg.GetSize(1)
	    || cindex[2] < 0 || cindex[2] >= (int) rg.GetSize(2))
	{
	    printf ("N/A\n");
	} else {
	    InterpolatorType::OutputType pixel_value 
		= interpolator->EvaluateAtContinuousIndex (cindex);
	    printf ("%f %f %f\n", 
		pixel_value[0], pixel_value[1], pixel_value[2]);
	}
    }

    std::vector<float> location_list = parse_float3_string (
	parms->location_string);
    for (unsigned int i = 0; i < location_list.size() / 3; i++) {
	FloatPoint3DType point;
	point[0] = location_list[i*3+0];
	point[1] = location_list[i*3+1];
	point[2] = location_list[i*3+2];

	itk::ContinuousIndex<float, 3> cindex;
	img->TransformPhysicalPointToContinuousIndex (point, cindex);
	printf ("%4d: %7.2f, %7.2f, %7.2f; %7.2f, %7.2f, %7.2f; ", 
	    (int) (index_list.size() / 3) + i, 
	    cindex[0], cindex[1], cindex[2], 
	    point[0], point[1], point[2]);
	if (cindex[0] < 0 || cindex[0] >= (int) rg.GetSize(0)
	    || cindex[1] < 0 || cindex[1] >= (int) rg.GetSize(1)
	    || cindex[2] < 0 || cindex[2] >= (int) rg.GetSize(2))
	{
	    printf ("N/A\n");
	} else {
	    InterpolatorType::OutputType pixel_value 
		= interpolator->EvaluateAtContinuousIndex (cindex);
	    printf ("%f %f %f\n", 
		pixel_value[0], pixel_value[1], pixel_value[2]);
	}
    }
}

static void
do_probe (Probe_parms *parms)
{
    switch (plm_file_format_deduce (parms->input_fn)) {
    case PLM_FILE_FMT_VF:
	probe_vf_main (parms);
	break;
    case PLM_FILE_FMT_IMG:
    default:
	probe_img_main (parms);
	break;
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    std::cout << "Usage: plastimatch probe [options] file\n";
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Probe_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Basic options */
    parser->add_long_option ("i", "index", 
	"List of voxel indices, such as \"i j k;i j k;...\"", 1, "");
    parser->add_long_option ("l", "location", 
	"List of spatial locations, such as \"i j k;i j k;...\"", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an index or location was given */
    if (!parser->have_option ("index") 
	&& !parser->have_option("location"))
    {
	throw (dlib::error ("Error.  Please specify either an index "
		"or a location option"));
    }

    /* Check that an input file was given */
    if (parser->number_of_arguments() == 0) {
	throw (dlib::error ("Error.  You must specify an input file"));
	
    } else if (parser->number_of_arguments() > 1) {
	std::string extra_arg = (*parser)[1];
	throw (dlib::error ("Error.  Unknown option " + extra_arg));
    }

    /* Copy values into output struct */
    parms->input_fn = (*parser)[0];
    parms->index_string = parser->get_string("index");
    parms->location_string = parser->get_string("location");
}

void
do_command_probe (int argc, char *argv[])
{
    Probe_parms parms;

    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    do_probe (&parms);
}
