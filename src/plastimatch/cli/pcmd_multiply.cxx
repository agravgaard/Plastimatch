/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include "pcmd_multiply.h"
#include <list>
#include "itkMultiplyImageFilter.h"

#include "itk_image_type.h"
#include "itk_scale.h"
#include "plm_clp.h"
#include "plm_file_format.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "xform.h"

class Multiply_parms {
public:
    std::string output_fn;
    std::list<std::string> input_fns;
public:
    Multiply_parms () {}
};

void
multiply_vf_main (Multiply_parms *parms)
{
    typedef itk::MultiplyImageFilter< 
        DeformationFieldType, DeformationFieldType, DeformationFieldType 
        > MultiplyFilterType;

    MultiplyFilterType::Pointer mulfilter = MultiplyFilterType::New();

    /* Load the first input image */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Xform mult;
    mult.load (*it);
    ++it;

    /* Loop through remaining images */
    while (it != parms->input_fns.end()) {
        /* Load the images */
        Xform tmp;
        tmp.load (*it);

        /* Add it to running mult */
	mulfilter->SetInput1 (mult.get_itk_vf());
	mulfilter->SetInput2 (tmp.get_itk_vf());
	mulfilter->Update();
	mult.set_itk_vf (mulfilter->GetOutput ());
        ++it;
    }

    /* Save the mult image */
    mult.save (parms->output_fn);
}

void
multiply_vol_main (Multiply_parms *parms)
{
    typedef itk::MultiplyImageFilter< 
        FloatImageType, FloatImageType, FloatImageType > MultiplyFilterType;

    MultiplyFilterType::Pointer mulfilter = MultiplyFilterType::New();

    /* Load the first input image */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Plm_image::Pointer mult = plm_image_load (*it, PLM_IMG_TYPE_ITK_FLOAT);
    ++it;

    /* Loop through remaining images */
    while (it != parms->input_fns.end()) {
        /* Load the images */
        Plm_image::Pointer tmp = plm_image_load (*it, PLM_IMG_TYPE_ITK_FLOAT);

        /* Add it to running mult */
	mulfilter->SetInput1 (mult->itk_float());
	mulfilter->SetInput2 (tmp->itk_float());
	mulfilter->Update();
	mult->m_itk_float = mulfilter->GetOutput ();
        ++it;
    }

    /* Save the mult image */
    mult->convert_to_original_type ();
    mult->save_image (parms->output_fn);
}

void
multiply_main (Multiply_parms *parms)
{
    
    /* What is the input file type? */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Plm_file_format file_format = plm_file_format_deduce (*it);

    switch (file_format) {
    case PLM_FILE_FMT_VF:
        multiply_vf_main (parms);
        break;
    default:
        multiply_vol_main (parms);
        break;
    }
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] input_file [input_file ...]\n", 
        argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Multiply_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", "output image", 1, "");

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an output file was given */
    if (!parser->option ("output")) {
	throw (dlib::error ("Error.  Please specify an output file "
		"using the --output option"));
    }

    /* Check that no extraneous options were given */
    if (parser->number_of_arguments() == 0) {
	throw (dlib::error ("Error.  You must specify at least one "
                "file to multiply."));
    }

    /* Copy input filenames to parms struct */
    for (unsigned long i = 0; i < parser->number_of_arguments(); i++) {
        parms->input_fns.push_back ((*parser)[i]);
    }

    /* Output files */
    parms->output_fn = parser->get_string("output");
}

void
do_command_multiply (int argc, char *argv[])
{
    Multiply_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    multiply_main (&parms);
}
