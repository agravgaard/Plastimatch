/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"

#include "file_util.h"
#include "itk_image_load.h"
#include "itk_image_save.h"
#include "itk_image_type.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_warp.h"
#include "print_and_exit.h"
#include "rt_study.h"
#include "rt_study_metadata.h"
#include "rt_study_warp.h"
#include "rtss.h"
#include "segmentation.h"
#include "simplify_points.h"
#include "string_util.h"
#include "warp_parms.h"
#include "volume.h"
#include "xform.h"
#include "xio_dose.h"

static void
load_input_files (Rt_study *rt_study, Plm_file_format file_type, 
    Warp_parms *parms)
{
    if (parms->input_fn != "") {
        rt_study->load (parms->input_fn.c_str(), file_type);
    }

    if (parms->input_cxt_fn != "") {
        rt_study->load_cxt (parms->input_cxt_fn.c_str());
    }

    if (parms->input_prefix != "") {
        rt_study->load_prefix (parms->input_prefix.c_str());
    }

    if (parms->input_ss_img_fn != "") {
        if (!file_exists (parms->input_ss_img_fn)) {
            print_and_exit ("Error: cannot open file %s for read\n",
                parms->input_ss_img_fn.c_str());
        }
        rt_study->load_ss_img (
            parms->input_ss_img_fn.c_str(), 
            parms->input_ss_list_fn.c_str());
    }

    if (parms->input_dose_img_fn != "") {
        rt_study->load_dose_img (parms->input_dose_img_fn.c_str());
    }

    if (parms->input_dose_xio_fn != "") {
        rt_study->load_dose_xio (parms->input_dose_xio_fn.c_str());
    }

    if (parms->input_dose_ast_fn != "") {
        rt_study->load_dose_astroid (parms->input_dose_ast_fn.c_str());
    }

    if (parms->input_dose_mc_fn != "") {
        rt_study->load_dose_mc (parms->input_dose_mc_fn.c_str());
    }

    if (!rt_study->have_image()
        && !rt_study->have_segmentation()
        && !rt_study->have_dose())
    {
        print_and_exit ("Sorry, could not load input as any known type.\n");
    }
}

static void
save_ss_img (
    Rt_study *rt_study, 
    const Xform *xf, 
    Plm_image_header *pih, 
    Warp_parms *parms
)
{
    Segmentation::Pointer seg = rt_study->get_segmentation();

    /* labelmap */
    if (parms->output_labelmap_fn != "") {
        lprintf ("save_ss_img: save_labelmap\n");
        seg->save_labelmap (parms->output_labelmap_fn.c_str());
    }

    /* ss_img */
    if (parms->output_ss_img_fn != "") {
        lprintf ("save_ss_img: save_ss_image\n");
        seg->save_ss_image (parms->output_ss_img_fn);
    }

    /* list of structure names */
    if (parms->output_ss_list_fn != "") {
        lprintf ("save_ss_img: save_ss_list\n");
        seg->save_ss_list (parms->output_ss_list_fn);
    }

    /* prefix images */
    if (parms->output_prefix != "") {
        lprintf ("save_ss_img: save_prefix\n");
        seg->save_prefix (parms->output_prefix, parms->prefix_format);
    }

    /* prefix fcsv files */
    if (parms->output_prefix_fcsv != "") {
        lprintf ("save_ss_img: save_prefix_fcsv\n");
        lprintf ("save_ss_img: save_prefix_fcsv (%s)\n",
            parms->output_prefix_fcsv.c_str());
        seg->save_prefix_fcsv (parms->output_prefix_fcsv.c_str());
    }

    /* 3D Slicer color table */
    if (parms->output_colormap_fn != "") {
        lprintf ("save_ss_img: save_colormap\n");
        seg->save_colormap (parms->output_colormap_fn.c_str());
    }

    /* cxt */
    if (parms->output_cxt_fn != "") {
        lprintf ("save_ss_img: save_cxt\n");
        seg->save_cxt (rt_study->get_rt_study_metadata (),
            parms->output_cxt_fn.c_str(), false);
    }

    /* xio */
    if (parms->output_xio_dirname != "") {
        lprintf ("save_ss_img: save_xio (dirname = %s)\n", 
            parms->output_xio_dirname.c_str());
        seg->save_xio (
            rt_study->get_rt_study_metadata (),
            rt_study->get_xio_ct_transform (),
            parms->output_xio_version,
            parms->output_xio_dirname);
    }
}

static void
warp_and_save_ss (
    Rt_study *rt_study,  
    const Xform::Pointer& xf, 
    Plm_image_header *pih, 
    Warp_parms *parms)
{
    if (!rt_study->have_segmentation()) {
        return;
    }

    Segmentation::Pointer seg = rt_study->get_segmentation();

    /* If we have need to create image outputs, or if we have to 
       warp something, then we need to rasterize the volume */
    /* GCS FIX: If there is an input m_ss_img, we still do this 
       because we might need the labelmap */
    if (parms->output_labelmap_fn != ""
        || parms->output_ss_img_fn != ""
        || parms->xf_in_fn != ""
        || parms->output_prefix != "")
    {

        /* In the following cases, we should use the default 
           rasterization geometry (i.e. cxt->rast_xxx):
           (a) Warping
           (b) No known output geometry
           (c) Output geometry doesn't match slice locations

           GCS FIX: Only case (a) is handled.

           In the other cases we can directly rasterize to the output 
           geometry.
        */
        Plm_image_header pih;
        Rtss *cxt = seg->get_structure_set_raw ();
        if (parms->xf_in_fn != "") {
            pih.set_from_gpuit (cxt->rast_dim, cxt->rast_offset, 
                cxt->rast_spacing, 0);
        } else {
            pih.set_from_gpuit (cxt->m_dim, cxt->m_offset, cxt->m_spacing, 0);
        }
        lprintf ("Warp_and_save_ss: seg->rasterize\n");
        seg->rasterize (&pih,
            parms->output_labelmap_fn != "",
            parms->xor_contours);
    }

    /* Do the warp */
    if (parms->xf_in_fn != "") {
        lprintf ("Warp_and_save_ss: seg->warp\n");
        seg->warp (xf, pih, parms);
    }

    /* If we are warping, re-extract polylines into cxt */
    /* GCS FIX: This is only necessary if we are outputting polylines. 
       Otherwise it is wasting users time. */
    if (parms->xf_in_fn != "") {
        lprintf ("Warp_and_save_ss: seg->cxt_re_extract\n");
        seg->cxt_re_extract ();
    }

    /* If we need to reduce the number of points (aka if simplify-perc 
       was set), purge the excessive points...*/
    if (parms->simplify_perc > 0. && parms->simplify_perc < 100.) {
        lprintf ("Warp_and_save_ss: do_simplify\n");
        do_simplify(rt_study, parms->simplify_perc);
    }

    /* Save non-dicom formats, such as mha, cxt, xio */
    lprintf ("Warp_and_save_ss: save_ss_img\n");
    save_ss_img (rt_study, xf.get(), pih, parms);
}

void
rt_study_warp (Rt_study *rt_study, Plm_file_format file_type, Warp_parms *parms)
{
    DeformationFieldType::Pointer vf = DeformationFieldType::New();
    Xform::Pointer xform = Xform::New ();
    Plm_image_header pih;

    /* Load referenced DICOM directory */
    if (parms->referenced_dicom_dir != "") {
        lprintf ("Loading RDD\n");
        rt_study->load_rdd (parms->referenced_dicom_dir.c_str());
        lprintf ("Loading RDD complete\n");
    }

    /* Set user-supplied metadata also prior to loading files,
       because the user supplied patient position is needed to load XiO data */
    rt_study->set_study_metadata (parms->m_study_metadata);

    /* Load input file(s) */
    load_input_files (rt_study, file_type, parms);

    /* Set user-supplied metadata (overrides metadata in input files) */
    rt_study->set_study_metadata (parms->m_study_metadata);
    rt_study->set_image_metadata (parms->m_image_metadata);
    rt_study->set_dose_metadata (parms->m_dose_metadata);
    rt_study->set_rtstruct_metadata (parms->m_rtstruct_metadata);

    /* Load transform */
    if (parms->xf_in_fn != "") {
        lprintf ("Loading xform (%s)\n", parms->xf_in_fn.c_str());
        xform = xform_load (parms->xf_in_fn);
    }

    /* Try to guess the proper dimensions and spacing for output image */
    Xform_type xform_type = xform->get_type ();
    if (parms->fixed_img_fn != "") {
        /* use the spacing of user-supplied fixed image */
        lprintf ("Setting PIH from FIXED\n");
        FloatImageType::Pointer fixed = itk_image_load_float (
            parms->fixed_img_fn, 0);
        pih.set_from_itk_image (fixed);
    } else if (xform_type == XFORM_ITK_VECTOR_FIELD) {
        /* use the spacing from input vector field */
        lprintf ("Setting PIH from VF\n");
        pih.set_from_itk_image (xform->get_itk_vf());
    } else if (xform_type == XFORM_GPUIT_BSPLINE) {
        /* use the spacing from input bxf file */
        lprintf ("Setting PIH from XFORM\n");
        pih.set_from_gpuit_bspline (xform->get_gpuit_bsp());
    } else if (rt_study->get_rt_study_metadata()->slice_list_complete()) {
        /* use spacing from referenced CT */
        lprintf ("Setting PIH from RDD\n");
        Plm_image_header::clone (&pih, 
            rt_study->get_rt_study_metadata()->get_image_header());
    } else if (rt_study->have_image()) {
        /* use the spacing of the input image */
        lprintf ("Setting PIH from M_IMG\n");
        pih.set_from_plm_image (rt_study->get_image().get());
    } else if (rt_study->have_segmentation()
        && rt_study->get_segmentation()->have_ss_img())
    {
        /* use the spacing of the input image */
        lprintf ("Setting PIH from M_SS_IMG\n");
        pih.set_from_plm_image (rt_study->get_segmentation()->get_ss_img());
    }
    else if (rt_study->have_segmentation() &&
        /* use the spacing of the structure set */
        rt_study->get_segmentation()->have_structure_set() && 
        rt_study->get_segmentation()->get_structure_set()->have_geometry) {
        pih.set_from_gpuit (
            rt_study->get_segmentation()->get_structure_set()->m_dim, 
            rt_study->get_segmentation()->get_structure_set()->m_offset, 
            rt_study->get_segmentation()->get_structure_set()->m_spacing, 
            0);
    } else if (rt_study->has_dose()) {
        /* use the spacing of dose */
        lprintf ("Setting PIH from DOSE\n");
        pih.set_from_plm_image (rt_study->get_dose());
    } else if (rt_study->have_segmentation()
        && rt_study->get_segmentation()->have_structure_set())
    {
        /* we have structure set, but without geometry.  use 
           heuristics to find a good geometry for rasterization */
        rt_study->get_segmentation()->find_rasterization_geometry (&pih);
    } else {
        /* use some generic default parameters */
        plm_long dim[3] = { 500, 500, 500 };
        float origin[3] = { -249.5, -249.5, -249.5 };
        float spacing[3] = { 1., 1., 1. };
        pih.set_from_gpuit (dim, origin, spacing, 0);
    }

    if (parms->m_have_dim) {
        pih.set_dim (parms->m_dim);
    }
    if (parms->m_have_origin) {
        pih.set_origin (parms->m_origin);
    }
    if (parms->m_have_spacing) {
        pih.set_spacing (parms->m_spacing);
    }
    if (parms->m_have_direction_cosines) {
        /* GCS FIX: This will do illogical things unless the user 
           sets the origin properly */
        pih.set_direction_cosines (parms->m_dc);
    }

    lprintf ("PIH is:\n");
    pih.print ();

    /* Check if output image geometry matches input image geometry.
       If it doesn't we need to resample to get the output geometry. 
       We need to supply an identity xform if none was supplied, 
       so that the warp function can do the resample. */
    bool pih_changed = false;
    if (rt_study->get_image()) {
        Plm_image_header pih_input_image (rt_study->get_image());
        if (!Plm_image_header::compare (&pih_input_image, &pih)) {
            pih_changed = true;
            if (parms->xf_in_fn == "") {
                TranslationTransformType::Pointer trn
                    = TranslationTransformType::New();
                xform->set_trn(trn);
            }
        }
    }

    /* Warp the image and create vf */
    if (rt_study->have_image()
        && (parms->xf_in_fn != ""
            || pih_changed)
        && (parms->output_img_fn != ""
            || parms->output_vf_fn != ""
            || parms->output_dicom != ""))
    {
        Plm_image::Pointer im_out = Plm_image::New();
        lprintf ("Rt_study_warp: Warping m_img\n");
        plm_warp (im_out, &vf, xform, &pih, rt_study->get_image(), 
            parms->default_val, parms->use_itk, parms->interp_lin);
        rt_study->set_image (im_out);
    }

    /* Save output image */
    if (parms->output_img_fn != "" && rt_study->have_image()) {
        lprintf ("Rt_study_warp: Saving m_img (%s)\n",
            parms->output_img_fn.c_str());
        rt_study->get_image()->convert_and_save (
            parms->output_img_fn.c_str(), 
            parms->output_type);
    }

    /* Warp the dose image */
    if (rt_study->has_dose()
        && parms->xf_in_fn != ""
        && (parms->output_dose_img_fn != ""
            || parms->output_xio_dirname != ""
            || parms->output_dicom != ""))
    {
        lprintf ("Rt_study_warp: Warping dose\n");
        Plm_image::Pointer im_out = Plm_image::New();
        plm_warp (im_out, 0, xform, &pih, rt_study->get_dose(), 0, 
            parms->use_itk, 1);
        rt_study->set_dose (im_out);
    }

    /* Scale the dose image */
    if (rt_study->has_dose() && parms->have_dose_scale) {
        rt_study->get_dose_volume_float()->scale_inplace (parms->dose_scale);
    }

    /* Save output dose image */
    if (parms->output_dose_img_fn != "" && rt_study->has_dose())
    {
        lprintf ("Rt_study_warp: Saving dose image (%s)\n", 
            parms->output_dose_img_fn.c_str());
        rt_study->save_dose (
            parms->output_dose_img_fn.c_str(), 
            parms->output_type);
    }

    /* Save output XiO dose */
    if (parms->output_xio_dirname != ""
        && rt_study->get_xio_dose_filename() != ""
        && rt_study->has_dose())
    {
        lprintf ("Rt_study_warp: Saving xio dose.\n");
        std::string fn = string_format ("%s/%s", 
            parms->output_xio_dirname.c_str(), "dose");
        xio_dose_save (
            rt_study->get_dose(),
            rt_study->get_study_metadata(), 
            rt_study->get_xio_ct_transform(),
            fn.c_str(), 
            rt_study->get_xio_dose_filename().c_str());
    }

    /* Save output vector field */
    if (parms->xf_in_fn != "" 
        && parms->output_vf_fn != "")
    {
        lprintf ("Rt_study_warp: Saving vf.\n");
        itk_image_save (vf, parms->output_vf_fn.c_str());
    }

    /* Preprocess structure sets */
    if (rt_study->have_segmentation()) {
        Segmentation::Pointer seg = rt_study->get_segmentation();

        /* Convert ss_img to cxt */
        lprintf ("Rt_study_warp: Convert ss_img to cxt.\n");
        seg->convert_ss_img_to_cxt ();

        /* Delete empty structures */
        if (parms->prune_empty) {
            lprintf ("Rt_study_warp: Prune empty structures.\n");
            seg->prune_empty ();
        }

        /* Set the DICOM reference info -- this sets the internal geometry 
           of the ss_image so we rasterize on the same slices as the CT? */
        lprintf ("Rt_study_warp: Apply dicom_dir.\n");
        seg->apply_dicom_dir (rt_study->get_rt_study_metadata());
        
        /* Set the output geometry */
        lprintf ("Rt_study_warp: Set geometry from PIH.\n");
        seg->set_geometry (&pih);

        /* Set rasterization geometry */
        lprintf ("Rt_study_warp: Set rasterization geometry.\n");
        seg->get_structure_set()->set_rasterization_geometry ();
    }

    /* Warp and save structure set (except dicom) */
    lprintf ("Rt_study_warp: warp and save ss.\n");
    warp_and_save_ss (rt_study, xform, &pih, parms);

    /* Save dicom */
    if (parms->output_dicom != "") {
        lprintf ("Rt_study_warp: Save dicom.\n");
        rt_study->save_dicom (parms->output_dicom.c_str(),
            parms->dicom_with_uids);
    }
}
