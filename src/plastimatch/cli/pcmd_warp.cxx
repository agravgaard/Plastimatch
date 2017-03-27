/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <algorithm>
#include <stdio.h>
#include <time.h>

#include "pcmd_warp.h"
#include "plm_clp.h"
#include "plm_file_format.h"
#include "print_and_exit.h"
#include "rt_study.h"
#include "rt_study_warp.h"
#include "string_util.h"
#include "warp_parms.h"

static void
usage_fn (dlib::Plm_clp* parser, int argc, char *argv[])
{
    printf ("Usage: plastimatch %s [options]\n", argv[1]);
    parser->print_options (std::cout);
    std::cout << std::endl;
}

static void
parse_fn (
    Warp_parms* parms, 
    dlib::Plm_clp* parser, 
    int argc, 
    char* argv[]
)
{
    /* Add --help, --version */
    parser->add_default_options ();

    /* Input files */
    parser->add_long_option ("", "input", 
        "input directory or filename; "
        "can be an image, structure set file (cxt or dicom-rt), "
        "dose file (dicom-rt, monte-carlo or xio), "
        "dicom directory, or xio directory", 1, "");
    parser->add_long_option ("", "xf", 
        "input transform used to warp image(s)", 1, "");
    parser->add_long_option ("", "referenced-ct", 
        "dicom directory used to set UIDs and metadata", 1, "");
    parser->add_long_option ("", "input-cxt", 
        "input a cxt file", 1, "");
    parser->add_long_option ("", "input-prefix", 
        "input a directory of structure set images (one image per file)", 
        1, "");
    parser->add_long_option ("", "input-ss-img", 
        "input a structure set image file", 1, "");
    parser->add_long_option ("", "input-ss-list", 
        "input a structure set list file containing names and colors", 1, "");
    parser->add_long_option ("", "input-dose-img", 
        "input a dose volume", 1, "");
    parser->add_long_option ("", "input-dose-xio", 
        "input an xio dose volume", 1, "");
    parser->add_long_option ("", "input-dose-ast", 
        "input an astroid dose volume", 1, "");
    parser->add_long_option ("", "input-dose-mc", 
        "input an monte carlo volume", 1, "");
    parser->add_long_option ("", "dif", 
        "dif file (used by dij warper)", 1, "");

    /* Output files */
    parser->add_long_option ("", "output-img", 
        "output image; can be mha, mhd, nii, nrrd, or other format "
        "supported by ITK", 1, "");
    parser->add_long_option ("", "output-cxt", 
        "output a cxt-format structure set file", 1, "");
    parser->add_long_option ("", "output-dicom", 
        "create a directory containing dicom and dicom-rt files", 1, "");
    parser->add_long_option ("", "output-dij", 
        "create a dij matrix file", 1, "");
    parser->add_long_option ("", "output-dose-img", 
        "create a dose image volume", 1, "");
    parser->add_long_option ("", "output-labelmap", 
        "create a structure set image with each voxel labeled as "
        "a single structure", 1, "");
    parser->add_long_option ("", "output-colormap", 
        "create a colormap file that can be used with 3d slicer", 1, "");
    parser->add_long_option ("", "output-pointset", 
        "create a pointset file that can be used with 3d slicer", 1, "");
    parser->add_long_option ("", "output-prefix", 
        "create a directory with a separate image for each structure", 1, "");
    parser->add_long_option ("", "output-prefix-fcsv", 
        "create a directory with a separate fcsv pointset file for "
        "each structure", 1, "");
    parser->add_long_option ("", "output-ss-img", 
        "create a structure set image which allows overlapping structures", 
        1, "");
    parser->add_long_option ("", "output-ss-list", 
        "create a structure set list file containing names and colors", 
        1, "");
    parser->add_long_option ("", "output-vf", 
        "create a vector field from the input xf", 1, "");
    parser->add_long_option ("", "output-xio", 
        "create a directory containing xio-format files", 1, "");

    /* Output options */
    parser->add_long_option ("", "output-type", 
        "type of output image, one of {uchar, short, float, ...}", 1, "");
    parser->add_long_option ("", "prefix-format", 
        "file format of rasterized structures, either \"mha\" or \"nrrd\"",
        1, "mha");
    parser->add_long_option ("", "dicom-with-uids", 
        "set to false to remove uids from created dicom filenames, "
        "default is true", 1, "true");
    parser->add_long_option ("", "dij-dose-volumes",
        "set to true to output nrrd files corresponding to Dij matrix "
        " beamlets, default is false", 1, "true");

    /* Algorithm options */
    parser->add_long_option ("", "algorithm", 
        "algorithm to use for warping, either \"itk\" or \"native\", "
        "default is native", 1, "native");
    parser->add_long_option ("", "force-resample",
        "resample the transformed image even when transform is linear", 0);
    parser->add_long_option ("", "dose-scale", 
        "scale the dose by this value", 1, "");
    parser->add_long_option ("", "interpolation", 
        "interpolation to use when resampling, either \"nn\" for "
        "nearest neighbors or \"linear\" for tri-linear, default is linear", 
        1, "linear");
    parser->add_long_option ("", "default-value", 
        "value to set for pixels with unknown value, default is 0", 1, "");
    parser->add_long_option ("", "prune-empty", 
        "delete empty structures from output", 0);
    parser->add_long_option ("", "simplify-perc", 
        "delete <arg> percent of the vertices from output polylines", 1, "0");
    parser->add_long_option ("", "xor-contours", 
        "overlapping contours should be xor'd instead of or'd", 0);

    /* Geometry options */
    parser->add_long_option ("F", "fixed", 
        "fixed image (match output size to this image)", 1, "");
    parser->add_long_option ("", "origin", 
        "location of first image voxel in mm \"x y z\"", 1, "");
    parser->add_long_option ("", "dim", 
        "size of output image in voxels \"x [y z]\"", 1, "");
    parser->add_long_option ("", "spacing", 
        "voxel spacing in mm \"x [y z]\"", 1, "");
    parser->add_long_option ("", "direction-cosines", 
        "oriention of x, y, and z axes; Specify either preset value,"
        " {identity,rotated-{1,2,3},sheared},"
        " or 9 digit matrix string \"a b c d e f g h i\"", 1, "");

    /* Metadata options */
    parser->add_long_option ("", "metadata",
        "patient metadata (you may use this option multiple times), "
        "option written as \"XXXX,YYYY=string\"", 1, "");
    parser->add_long_option ("", "modality",
        "modality metadata: such as {CT, MR, PT}, default is CT", 1, "CT");
    parser->add_long_option ("", "patient-id",
        "patient id metadata: string", 1);
    parser->add_long_option ("", "patient-name",
        "patient name metadata: string", 1);
    parser->add_long_option ("", "patient-pos",
        "patient position metadata: one of {hfs,hfp,ffs,ffp}", 1, "hfs");
    parser->add_long_option ("", "study-description",
        "study description: string", 1);
    parser->add_long_option ("", "series-description",
        "series description for image metadata: string", 1);
    parser->add_long_option ("", "dose-series-description",
        "series description for dose metadata: string", 1);
    parser->add_long_option ("", "rtss-series-description",
        "series description for structure metadata: string", 1);
    parser->add_long_option ("", "series-number",
        "series number for image metadata: integer", 1);
    parser->add_long_option ("", "dose-series-number",
        "series number for dose metadata: integer", 1);
    parser->add_long_option ("", "rtss-series-number",
        "series number for structure metadata: integer", 1);

    /* Parse options */
    parser->parse (argc,argv);

    /* Handle --help, --version */
    parser->check_default_options ();

    /* Check that an input file was given */
    if (!parser->option ("input") 
        && !parser->option("input-cxt")
        && !parser->option("input-prefix")
        && !parser->option("input-ss-img")
        && !parser->option("input-ss-list")
        && !parser->option("input-dose-img")
        && !parser->option("input-dose-xio")
        && !parser->option("input-dose-ast")
        && !parser->option("input-dose-mc"))
    {
        throw (dlib::error ("Error.  Please specify an input file "
                "using one of the --input options"));
    }

    /* Check that no extraneous options were given */
    if (parser->number_of_arguments() != 0) {
        std::string extra_arg = (*parser)[0];
        throw (dlib::error ("Error.  Unknown option " + extra_arg));
    }

    /* Input files */
    parms->input_fn = parser->get_string("input").c_str();
    parms->xf_in_fn = parser->get_string("xf").c_str();
    parms->referenced_dicom_dir = parser->get_string("referenced-ct").c_str();
    parms->input_cxt_fn = parser->get_string("input-cxt").c_str();
    parms->input_prefix = parser->get_string("input-prefix").c_str();
    parms->input_ss_img_fn = parser->get_string("input-ss-img").c_str();
    parms->input_ss_list_fn = parser->get_string("input-ss-list").c_str();
    parms->input_dose_img_fn = parser->get_string("input-dose-img").c_str();
    parms->input_dose_xio_fn = parser->get_string("input-dose-xio").c_str();
    parms->input_dose_ast_fn = parser->get_string("input-dose-ast").c_str();
    parms->input_dose_mc_fn = parser->get_string("input-dose-mc").c_str();

    /* Dij input files */
    parms->dif_in_fn = parser->get_string("dif").c_str();

    /* Output files */
    parms->output_img_fn = parser->get_string("output-img").c_str();
    parms->output_cxt_fn = parser->get_string("output-cxt").c_str();
    parms->output_dicom = parser->get_string("output-dicom").c_str();
    parms->output_dij_fn = parser->get_string("output-dij").c_str();
    parms->output_dose_img_fn = parser->get_string("output-dose-img").c_str();
    parms->output_labelmap_fn = parser->get_string("output-labelmap").c_str();
    parms->output_colormap_fn = parser->get_string("output-colormap").c_str();
    parms->output_pointset_fn = parser->get_string("output-pointset").c_str();
    parms->output_prefix = parser->get_string("output-prefix").c_str();
    parms->output_prefix_fcsv 
        = parser->get_string("output-prefix-fcsv").c_str();
    parms->output_ss_img_fn = parser->get_string("output-ss-img");
    parms->output_ss_list_fn = parser->get_string("output-ss-list");
    parms->output_vf_fn = parser->get_string("output-vf").c_str();
    parms->output_xio_dirname = parser->get_string("output-xio").c_str();
    
    /* Output options */
    if (parser->option("output-type")) {
        std::string arg = parser->get_string ("output-type");
        parms->output_type = plm_image_type_parse (arg.c_str());
        if (parms->output_type == PLM_IMG_TYPE_UNDEFINED) {
            throw (dlib::error ("Error. Unknown --output-type argument: " 
                    + parser->get_string("output-type")));
        }
    }
    if (parser->option("prefix-format")) {
        parms->prefix_format = parser->get_string ("prefix-format");
    } else {
        parms->prefix_format = "mha";
    }
    if (parser->option("dicom-with-uids")) {
        parms->dicom_with_uids = string_value_true (
            parser->get_string ("dicom-with-uids"));
    }
    if (parser->option("dij-dose-volumes")) {
        parms->output_dij_dose_volumes = string_value_true (
            parser->get_string ("dij-dose-volumes"));
    }

    /* Algorithm options */
    std::string arg = parser->get_string ("algorithm");
    if (arg == "itk") {
        parms->use_itk = 1;
    }
    else if (arg == "native") {
        parms->use_itk = 0;
    }
    else {
        throw (dlib::error ("Error. Unknown --algorithm argument: " + arg));
    }

    if (parser->option("force-resample")) {
        parms->force_resample = true;
    }
    if (parser->option("default-value")) {
        parms->default_val = parser->get_float("default-value");
    }

    parms->have_dose_scale = false;
    if (parser->option("dose-scale")) {
        parms->have_dose_scale = true;
        parms->dose_scale = parser->get_float("dose-scale");
    }

    arg = parser->get_string ("interpolation");
    if (arg == "nn") {
        parms->interp_lin = 0;
    }
    else if (arg == "linear") {
        parms->interp_lin = 1;
    }
    else {
        throw (dlib::error ("Error. Unknown --interpolation argument: " 
                + arg));
    }

    if (parser->option("prune-empty")) {
        parms->prune_empty = 1;
    }
    parms->simplify_perc = parser->get_float("simplify-perc");

    if (parser->option("xor-contours")) {
        parms->xor_contours = true;
    }

    /* Geometry options */
    if (parser->option ("dim")) {
        parms->m_have_dim = true;
        parser->assign_plm_long_13 (parms->m_dim, "dim");
    }
    if (parser->option ("origin")) {
        parms->m_have_origin = true;
        parser->assign_float_13 (parms->m_origin, "origin");
    }
    if (parser->option ("spacing")) {
        parms->m_have_spacing = true;
        parser->assign_float_13 (parms->m_spacing, "spacing");
    }
    parms->fixed_img_fn = parser->get_string("fixed").c_str();

    /* Direction cosines */
    if (parser->option ("direction-cosines")) {
        parms->m_have_direction_cosines = true;
        std::string arg = parser->get_string("direction-cosines");
        if (!parms->m_dc.set_from_string (arg)) {
            throw (dlib::error ("Error parsing --direction-cosines "
                    "(should have nine numbers)\n"));
        }
    }

    /* Metadata options */
    for (unsigned int i = 0; i < parser->option("metadata").count(); i++) {
        parms->m_study_metadata.push_back (
            parser->option("metadata").argument(0,i));
    }
    if (parser->option ("modality")) {
        std::string arg = parser->get_string ("modality");
        std::string metadata_string = "0008,0060=" + arg;
        parms->m_image_metadata.push_back (metadata_string);
    }
    if (parser->option ("patient-name")) {
        std::string arg = parser->get_string ("patient-name");
        std::string metadata_string = "0010,0010=" + arg;
        parms->m_study_metadata.push_back (metadata_string);
    }
    if (parser->option ("patient-id")) {
        std::string arg = parser->get_string ("patient-id");
        std::string metadata_string = "0010,0020=" + arg;
        parms->m_study_metadata.push_back (metadata_string);
    }
    if (parser->option ("patient-pos")) {
        std::string arg = parser->get_string ("patient-pos");
        std::transform (arg.begin(), arg.end(), arg.begin(), 
            (int(*)(int)) toupper);
        std::string metadata_string = "0018,5100=" + arg;
        parms->m_study_metadata.push_back (metadata_string);
    }
    if (parser->option ("study-description")) {
        std::string arg = parser->get_string ("study-description");
        std::string metadata_string = "0008,1030=" + arg;
        parms->m_study_metadata.push_back (metadata_string);
    }
    if (parser->option ("series-description")) {
        std::string arg = parser->get_string ("series-description");
        std::string metadata_string = "0008,103e=" + arg;
        parms->m_image_metadata.push_back (metadata_string);
    }
    if (parser->option ("dose-series-description")) {
        std::string arg = parser->get_string ("dose-series-description");
        std::string metadata_string = "0008,103e=" + arg;
        parms->m_dose_metadata.push_back (metadata_string);
    }
    if (parser->option ("rtss-series-description")) {
        std::string arg = parser->get_string ("rtss-series-description");
        std::string metadata_string = "0008,103e=" + arg;
        parms->m_rtstruct_metadata.push_back (metadata_string);
    }
    if (parser->option ("series-number")) {
        std::string arg = parser->get_string ("series-number");
        std::string metadata_string = "0020,0011=" + arg;
        parms->m_image_metadata.push_back (metadata_string);
    }
    if (parser->option ("dose-series-number")) {
        std::string arg = parser->get_string ("dose-series-number");
        std::string metadata_string = "0020,0011=" + arg;
        parms->m_dose_metadata.push_back (metadata_string);
    }
    if (parser->option ("rtss-series-number")) {
        std::string arg = parser->get_string ("rtss-series-number");
        std::string metadata_string = "0020,0011=" + arg;
        parms->m_rtstruct_metadata.push_back (metadata_string);
    }
}

void
do_command_warp (int argc, char* argv[])
{
    Warp_parms parms;
    Plm_file_format file_type;
    Rt_study rt_study;

    /* Parse command line parameters */
    plm_clp_parse (&parms, &parse_fn, &usage_fn, argc, argv, 1);

    /* Dij matrices are a special case */
    if (parms.output_dij_fn != "") {
        if (parms.dif_in_fn != "")
        {
            warp_dij_main (&parms);
            return;
        } else {
            print_and_exit ("Sorry, you need to specify --dif for dij warping.\n");
        }
    }

    /* What is the input file type? */
    file_type = plm_file_format_deduce (parms.input_fn.c_str());

    /* Pointsets are a special case */
    if (file_type == PLM_FILE_FMT_POINTSET) {
        warp_pointset_main (&parms);
        return;
    }

    /* Process warp */
    rt_study_warp (&rt_study, file_type, &parms);

    printf ("Finished!\n");
}
