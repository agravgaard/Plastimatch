/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include "pcmd_maximum.h"
#include <list>
#include "itkMaximumImageFilter.h"

#include "itk_image_type.h"
#include "itk_scale.h"
#include "plm_clp.h"
#include "plm_file_format.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "xform.h"

class Maximum_parms {
public:
    std::string output_fn;
    std::list<std::string> input_fns;
public:
    Maximum_parms () {}
};

void
maximum_vol_main (Maximum_parms *parms)
{
    typedef itk::MaximumImageFilter< 
        FloatImageType, FloatImageType, FloatImageType > MaximumFilterType;

    MaximumFilterType::Pointer maxfilter = MaximumFilterType::New();

    /* Load the first input image */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Plm_image::Pointer max = plm_image_load (*it, PLM_IMG_TYPE_ITK_FLOAT);
    ++it;

    /* Loop through remaining images */
    while (it != parms->input_fns.end()) {
        /* Load the images */
        Plm_image::Pointer tmp = plm_image_load (*it, PLM_IMG_TYPE_ITK_FLOAT);

        /* Add it to running max */
        maxfilter->SetInput1 (max->itk_float());
        maxfilter->SetInput2 (tmp->itk_float());
        maxfilter->Update();
        max->m_itk_float = maxfilter->GetOutput ();
        ++it;
    }

    /* Save the max image */
    max->convert_to_original_type ();
    max->save_image (parms->output_fn);
}

void
maximum_main (Maximum_parms *parms)
{
    
    /* What is the input file type? */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Plm_file_format file_format = plm_file_format_deduce (*it);

    if (file_format == PLM_FILE_FMT_IMG)
        maximum_vol_main (parms);
    else
        print_and_exit (
            "Sorry, can only compute input images, given type %s\n",
            plm_file_format_string (file_format));
}

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options] input_image [input_image ...]\n", 
        argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Maximum_parms* parms, 
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
                "file to maximize (makes no sense)."));
    }

    /* Copy input filenames to parms struct */
    for (unsigned long i = 0; i < parser->number_of_arguments(); i++) {
        parms->input_fns.push_back ((*parser)[i]);
    }

    /* Output files */
    parms->output_fn = parser->get_string("output");
}

void
do_command_maximum (int argc, char *argv[])
{
    Maximum_parms parms;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    maximum_main (&parms);
}
