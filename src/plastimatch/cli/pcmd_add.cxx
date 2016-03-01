/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <list>
#include "itkAddImageFilter.h"

#include "itk_image_type.h"
#include "itk_scale.h"
#include "plm_clp.h"
#include "plm_file_format.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "xform.h"

class Add_parms {
public:
    std::string output_fn;
    bool average;
    std::vector<float> weight_vector;
    std::list<std::string> input_fns;
public:
    Add_parms () {
        average = false;
    }
};

static void
scale_image (Plm_image::Pointer& img, float weight)
{
    img->set_itk (itk_scale (img->itk_float(), weight));
}

static void
scale_vf (Xform *xf, float weight)
{
    xf->set_itk_vf (itk_scale (xf->get_itk_vf(), weight));
}

void
add_vf_main (Add_parms *parms)
{
    typedef itk::AddImageFilter< 
        DeformationFieldType, DeformationFieldType, DeformationFieldType 
        > AddFilterType;

    AddFilterType::Pointer addition = AddFilterType::New();

    /* Load the first input image */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Xform sum;
    sum.load (*it);
    ++it;

    /* Weigh the first input image */
    int widx = 0;
    if (parms->weight_vector.size() > 0) {
        scale_vf (&sum, parms->weight_vector[widx]);
        ++widx;
    }

    /* Loop through remaining images */
    while (it != parms->input_fns.end()) {
        /* Load the images */
        Xform tmp;
        tmp.load (*it);

        /* Weigh it */
        if (parms->weight_vector.size() > 0) {
            scale_vf (&tmp, parms->weight_vector[widx]);
            ++widx;
        }

        /* Add it to running sum */
	addition->SetInput1 (sum.get_itk_vf());
	addition->SetInput2 (tmp.get_itk_vf());
	addition->Update();
	sum.set_itk_vf (addition->GetOutput ());
        ++it;
    }

    /* Take average */
    if (parms->average) {
        float avg = 1.0f / parms->input_fns.size();
        scale_vf (&sum, avg);
    }

    /* Save the sum image */
    sum.save (parms->output_fn);
}

void
add_vol_main (Add_parms *parms)
{
    typedef itk::AddImageFilter< 
        FloatImageType, FloatImageType, FloatImageType > AddFilterType;

    AddFilterType::Pointer addition = AddFilterType::New();

    /* Load the first input image */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Plm_image::Pointer sum = plm_image_load (*it, PLM_IMG_TYPE_ITK_FLOAT);
    ++it;

    /* Weigh the first input image */
    int widx = 0;
    if (parms->weight_vector.size() > 0) {
        scale_image (sum, parms->weight_vector[widx]);
        ++widx;
    }

    /* Loop through remaining images */
    while (it != parms->input_fns.end()) {
        /* Load the images */
        Plm_image::Pointer tmp = plm_image_load (*it, PLM_IMG_TYPE_ITK_FLOAT);

        /* Weigh it */
        if (parms->weight_vector.size() > 0) {
            scale_image (tmp, parms->weight_vector[widx]);
            ++widx;
        }

        /* Add it to running sum */
	addition->SetInput1 (sum->itk_float());
	addition->SetInput2 (tmp->itk_float());
	addition->Update();
	sum->m_itk_float = addition->GetOutput ();
        ++it;
    }

    /* Take average */
    if (parms->average) {
        float avg = 1.0f / parms->input_fns.size();
        scale_image (sum, avg);
    }

    /* Save the sum image */
    sum->convert_to_original_type ();
    sum->save_image (parms->output_fn);
}

void
add_main (Add_parms *parms)
{
    /* Make sure we got the same number of input files and weights */
    if (parms->weight_vector.size() > 0 
        && parms->weight_vector.size() != parms->input_fns.size())
    {
        print_and_exit (
            "Error, you specified %d input files and %d weights\n",
            parms->input_fns.size(),
            parms->weight_vector.size());
    }
    
    /* What is the input file type? */
    std::list<std::string>::iterator it = parms->input_fns.begin();
    Plm_file_format file_format = plm_file_format_deduce (*it);

    switch (file_format) {
    case PLM_FILE_FMT_VF:
        add_vf_main (parms);
        break;
    default:
        add_vol_main (parms);
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
    Add_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Output files */
    parser->add_long_option ("", "output", "output image", 1, "");

    /* Weight vector */
    parser->add_long_option ("", "weight", 
        "specify a vector of weights; the images are multiplied "
        "by the weight prior to adding their values", 
        1, "");

    /* Average option */
    parser->add_long_option ("", "average", 
        "produce an output file which is the average of the input files "
        "(if no weights are specified), or multiply the weights by 1/n",
        0);

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
                "file to add."));
    }

    /* Copy input filenames to parms struct */
    for (unsigned long i = 0; i < parser->number_of_arguments(); i++) {
        parms->input_fns.push_back ((*parser)[i]);
    }

    /* Average */
    if (parser->option ("average")) {
        parms->average = true;
    }

    /* Output files */
    parms->output_fn = parser->get_string("output");
    if (parser->option ("weight")) {
        parser->assign_float_vec (&parms->weight_vector, "weight");
    }
}

void
do_command_add (int argc, char *argv[])
{
    Add_parms parms;

    /* Check if we're doing add or average */
    if (!strcmp (argv[1], "add")) {
        parms.average = false;
    } else {
        parms.average = true;
    }

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    add_main (&parms);
}
