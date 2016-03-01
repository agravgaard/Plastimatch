/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

#if PLM_DCM_USE_GDCM1
#include "gdcm1_dose.h"
#include "gdcm1_rtss.h"
#endif
#include "cxt_extract.h"
#include "cxt_io.h"
#include "dir_list.h"
#include "file_util.h"
#include "itk_image_save.h"
#include "itk_image_type.h"
#include "itk_resample.h"
#include "logfile.h"
#include "path_util.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_warp.h"
#include "pointset.h"
#include "print_and_exit.h"
#include "rasterizer.h"
#include "rt_study.h"
#include "rt_study_metadata.h"
#include "rtss.h"
#include "rtss_contour.h"
#include "rtss_roi.h"
#include "segmentation.h"
#include "ss_list_io.h"
#include "ss_img_extract.h"
#include "ss_img_stats.h"
#include "string_util.h"
#include "warp_parms.h"
#include "xio_structures.h"

class Segmentation_private {
public:
    Plm_image::Pointer m_labelmap; /* Structure set lossy bitmap form */
    Plm_image::Pointer m_ss_img;   /* Structure set in lossless bitmap form */
    Rtss::Pointer m_cxt;        /* Structure set in polyline form */

    bool m_rtss_valid;
    bool m_ss_img_valid;

public:
    Segmentation_private () {
        m_rtss_valid = false;
        m_ss_img_valid = false;
    }
    ~Segmentation_private () {
    }
};

static std::string
compose_prefix_fn (
    const std::string& output_prefix, 
    const std::string& structure_name,
    const char* extension
)
{
    return string_format ("%s/%s.%s", 
        output_prefix.c_str(), 
        structure_name.c_str(), 
        extension);
}

Segmentation::Segmentation ()
{
    this->d_ptr = new Segmentation_private;
}

Segmentation::~Segmentation ()
{
    clear ();
    delete this->d_ptr;
}

void
Segmentation::clear ()
{
    d_ptr->m_cxt.reset();
    d_ptr->m_ss_img.reset();
    d_ptr->m_labelmap.reset();
    d_ptr->m_rtss_valid = false;
    d_ptr->m_ss_img_valid = false;
}

void
Segmentation::load (const char *ss_img, const char *ss_list)
{
    /* Load ss_img */
    if (d_ptr->m_ss_img) {
        d_ptr->m_ss_img.reset();
    }
    if (ss_img && file_exists (ss_img)) {
        d_ptr->m_ss_img = plm_image_load_native (ss_img);
    }

    /* Load ss_list */
    if (d_ptr->m_cxt) {
        d_ptr->m_cxt.reset();
    }
    if (ss_list && file_exists (ss_list)) {
        lprintf ("Trying to load ss_list: %s\n", ss_list);
        d_ptr->m_cxt.reset (ss_list_load (0, ss_list));
    }

    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->free_all_polylines ();
    }
    d_ptr->m_rtss_valid = false;
    d_ptr->m_ss_img_valid = true;
}

void
Segmentation::load_prefix (const std::string& prefix_dir)
{
    this->load_prefix (prefix_dir.c_str());
}

void
Segmentation::load_prefix (const char *prefix_dir)
{
    /* Clear out any existing structures */
    this->clear ();

    /* Load the list of files in the directory */
    Dir_list dl;
    dl.load (prefix_dir);

    /* Make a quick pass through the directory to find the number of 
       files.  This is used to size the ss_img. */
    int max_structures = 0;
    for (int i = 0; i < dl.num_entries; i++) {
        /* Look at filename, make sure it is an mha or nrrd file */
        const char *entry = dl.entries[i];
	if (!extension_is (entry, ".mha") 
            && !extension_is (entry, ".nrrd"))
        {
            continue;
        }
        max_structures++;
    }
    int out_vec_len = 1 + (max_structures - 1) / 8;
    if (out_vec_len < 2) out_vec_len = 2;

    /* Make a second pass that actually loads the files */
    bool first = true;
    int bit = 0;
    UCharVecImageType::Pointer ss_img;
    Plm_image_header ss_img_pih;
    for (int i = 0; i < dl.num_entries; i++) {
        /* Look at filename, make sure it is an mha or nrrd file */
        const char *entry = dl.entries[i];
	if (!extension_is (entry, ".mha") 
            && !extension_is (entry, ".nrrd"))
        {
            continue;
        }

        /* Grab the structure name from the filename */
        char *structure_name = strdup (entry);
        strip_extension (structure_name);
        lprintf ("Loading structure: %s\n", structure_name);

        /* Load the file */
        std::string input_fn = string_format ("%s/%s", prefix_dir, entry);
        Plm_image img (input_fn, PLM_IMG_TYPE_ITK_UCHAR);
        Plm_image_header pih (img);

        if (first) {
            this->initialize_ss_image (pih, out_vec_len);

            ss_img = d_ptr->m_ss_img->itk_uchar_vec ();
            Plm_image_header::clone (&ss_img_pih, &pih);

            first = false;
        } else {
            if (!Plm_image_header::compare (&pih, &ss_img_pih)) {
                print_and_exit ("Image size mismatch when loading prefix_dir");
            }
        }

        /* Add name to ss_list */
        d_ptr->m_cxt->add_structure (
            structure_name, "", 
            d_ptr->m_cxt->num_structures + 1,
            bit);
        free (structure_name);

        /* GCS FIX: This code is replicated in ss_img_extract */
        unsigned int uchar_no = bit / 8;
        unsigned int bit_no = bit % 8;
        unsigned char bit_mask = 1 << bit_no;
        if (uchar_no > ss_img->GetVectorLength()) {
            print_and_exit ("Error.  Ss_img vector is too small.");
        }

        /* Set up iterators for looping through images */
        typedef itk::ImageRegionConstIterator< UCharImageType > 
            UCharIteratorType;
        typedef itk::ImageRegionIterator< UCharVecImageType > 
            UCharVecIteratorType;
        UCharImageType::Pointer uchar_img = img.itk_uchar();
        UCharIteratorType uchar_img_it (uchar_img, 
            uchar_img->GetLargestPossibleRegion());
        UCharVecIteratorType ss_img_it (ss_img, 
            ss_img->GetLargestPossibleRegion());

        /* Loop through voxels, or'ing them into ss_img */
        /* GCS FIX: This is inefficient, due to undesirable construct 
           and destruct of itk::VariableLengthVector of each pixel */
        for (
            uchar_img_it.GoToBegin(), ss_img_it.GoToBegin();
            !uchar_img_it.IsAtEnd();
            ++uchar_img_it, ++ss_img_it
        ) {
            unsigned char u = uchar_img_it.Get ();
            if (!u) continue;

            itk::VariableLengthVector<unsigned char> v 
                = ss_img_it.Get ();
            v[uchar_no] |= bit_mask;
            ss_img_it.Set (v);
        }

        /* Move to next bit */
        bit++;
    }

    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->free_all_polylines ();
    }
    d_ptr->m_rtss_valid = false;
    d_ptr->m_ss_img_valid = true;
}

void
Segmentation::add_structure (
    UCharImageType::Pointer itk_image, 
    const char *structure_name,
    const char *structure_color)
{
    Plm_image_header pih (itk_image);

    /* Allocate image if this is the first structure */
    if (!d_ptr->m_ss_img) {
        this->initialize_ss_image (pih, 2);
    }

    else {
        /* Make sure image size is the same */
        Plm_image_header ss_img_pih (d_ptr->m_ss_img);
        if (!Plm_image_header::compare (&pih, &ss_img_pih)) {
            print_and_exit ("Image size mismatch when adding structure");
        }
    }

    /* Figure out basic structure info */
    if (!structure_name) {
        structure_name = "";
    }
    if (!structure_color) {
        structure_color = "";
    }
    int bit = d_ptr->m_cxt->num_structures; /* GCS FIX: I hope this is ok */

    /* Add structure to rtss */
    d_ptr->m_cxt->add_structure (
        structure_name, structure_color,
        d_ptr->m_cxt->num_structures + 1,
        bit);

#if defined (commentout)
    /* Expand vector length if needed */
    UCharVecImageType::Pointer ss_img = d_ptr->m_ss_img->itk_uchar_vec ();
    if (uchar_no > ss_img->GetVectorLength()) {
        this->broaden_ss_image (uchar_no);
    }

    /* Set up iterators for looping through images */
    typedef itk::ImageRegionConstIterator< UCharImageType > 
        UCharIteratorType;
    typedef itk::ImageRegionIterator< UCharVecImageType > 
        UCharVecIteratorType;
    UCharIteratorType uchar_img_it (itk_image, 
        itk_image->GetLargestPossibleRegion());
    UCharVecIteratorType ss_img_it (ss_img, 
        ss_img->GetLargestPossibleRegion());

    /* Loop through voxels, or'ing them into ss_img */
    /* GCS FIX: This is inefficient, due to undesirable construct 
       and destruct of itk::VariableLengthVector of each pixel */
    for (uchar_img_it.GoToBegin(), ss_img_it.GoToBegin();
        !uchar_img_it.IsAtEnd();
        ++uchar_img_it, ++ss_img_it
    ) {
        unsigned char u = uchar_img_it.Get ();
        if (!u) continue;

        itk::VariableLengthVector<unsigned char> v 
            = ss_img_it.Get ();
        v[uchar_no] |= bit_mask;
        ss_img_it.Set (v);
    }
#endif

    /* Set bit within ss_img */
    this->set_structure_image (itk_image, bit);

    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->free_all_polylines ();
    }
    d_ptr->m_rtss_valid = false;
    d_ptr->m_ss_img_valid = true;
}

void
Segmentation::load_cxt (const std::string& input_fn, Rt_study_metadata *rsm)
{
    d_ptr->m_cxt = Rtss::New();
    cxt_load (d_ptr->m_cxt.get(), rsm, input_fn.c_str());

    d_ptr->m_rtss_valid = true;
    d_ptr->m_ss_img_valid = false;
}

void
Segmentation::load_gdcm_rtss (const char *input_fn, Rt_study_metadata *rsm)
{
#if PLM_DCM_USE_GDCM1
    d_ptr->m_cxt = Rtss::New();
    gdcm_rtss_load (d_ptr->m_cxt.get(), rsm, input_fn);

    d_ptr->m_rtss_valid = true;
    d_ptr->m_ss_img_valid = false;
#endif
}

void
Segmentation::load_xio (const Xio_studyset& studyset)
{
    d_ptr->m_cxt = Rtss::New();
    lprintf ("calling xio_structures_load\n");
    xio_structures_load (d_ptr->m_cxt.get(), studyset);

    d_ptr->m_rtss_valid = true;
    d_ptr->m_ss_img_valid = false;
}

size_t
Segmentation::get_num_structures ()
{
    if (d_ptr->m_cxt) {
        return d_ptr->m_cxt->num_structures;
    }
    return 0;
}

std::string
Segmentation::get_structure_name (size_t index)
{
    if (d_ptr->m_cxt) {
        return d_ptr->m_cxt->get_structure_name (index);
    }
    return 0;
}

void 
Segmentation::set_structure_name (size_t index, const std::string& name)
{
    if (!d_ptr->m_cxt) {
        return;
    }
    d_ptr->m_cxt->set_structure_name (index, name);
}

UCharImageType::Pointer
Segmentation::get_structure_image (int index)
{
    if (!d_ptr->m_ss_img) {
        print_and_exit (
            "Error extracting unknown structure image (no ssi %d)\n", index);
    }

    if (!d_ptr->m_cxt) {
        print_and_exit (
            "Error extracting unknown structure image (no cxt %d)\n", index);
    }

    Rtss_roi *curr_structure = d_ptr->m_cxt->slist[index];
    int bit = curr_structure->bit;

    if (bit == -1) {
        print_and_exit (
            "Error extracting unknown structure image (no bit %d)\n", index);
    }
    UCharImageType::Pointer prefix_img 
        = ss_img_extract_bit (d_ptr->m_ss_img, bit);

    return prefix_img;
}

void
Segmentation::save_colormap (const std::string& colormap_fn)
{
    ss_list_save_colormap (d_ptr->m_cxt.get(), colormap_fn.c_str());
}

void
Segmentation::save_cxt (
    const Rt_study_metadata::Pointer& rsm, 
    const std::string& cxt_fn, 
    bool prune_empty
)
{
    cxt_save (d_ptr->m_cxt.get(), rsm, cxt_fn.c_str(), prune_empty);
}

void
Segmentation::save_gdcm_rtss (
    const char *output_dir, 
    const Rt_study_metadata::Pointer& rsm
)
{
    std::string fn;

    /* Perform destructive keyholization of the cxt.  This is necessary 
       because DICOM-RT requires that structures with holes be defined 
       using a single structure */
    d_ptr->m_cxt->keyholize ();

    /* Some systems (GE ADW) do not allow special characters in 
       structure names.  */
    d_ptr->m_cxt->adjust_structure_names ();

    if (rsm) {
        this->apply_dicom_dir (rsm);
    }

    fn = string_format ("%s/%s", output_dir, "rtss.dcm");

#if PLM_DCM_USE_GDCM1
    gdcm_rtss_save (d_ptr->m_cxt.get(), rsm, fn.c_str());
#else
    /* GDCM 2 not implemented -- you're out of luck. */
#endif
}

void
Segmentation::save_fcsv (
    const Rtss_roi *curr_structure, 
    const std::string& fn
)
{
    Labeled_pointset pointset;

    for (size_t j = 0; j < curr_structure->num_contours; j++) {
        Rtss_contour *curr_polyline = curr_structure->pslist[j];
        for (int k = 0; k < curr_polyline->num_vertices; k++) {
            pointset.insert_lps ("", curr_polyline->x[k],
                curr_polyline->y[k], curr_polyline->z[k]);
        }
    }

    pointset.save_fcsv (fn);
}

void
Segmentation::save_prefix_fcsv (const std::string& output_prefix)
{
    if (!d_ptr->m_cxt) {
        print_and_exit (
            "Error: save_prefix_fcsv() tried to save a RTSS without a CXT\n");
    }

    for (size_t i = 0; i < d_ptr->m_cxt->num_structures; i++)
    {
        Rtss_roi *curr_structure = d_ptr->m_cxt->slist[i];

        std::string fn = 
            compose_prefix_fn (output_prefix, curr_structure->name, "fcsv");
        save_fcsv (curr_structure, fn);
    }
}

void
Segmentation::save_ss_image (const std::string& ss_img_fn)
{
    if (!d_ptr->m_ss_img) {
        print_and_exit (
            "Error: save_ss_image() tried to write a non-existant ss_img");
    }
    if (d_ptr->m_ss_img->m_type == PLM_IMG_TYPE_GPUIT_UCHAR_VEC
        || d_ptr->m_ss_img->m_type == PLM_IMG_TYPE_ITK_UCHAR_VEC) 
    {
        /* Image type must be uchar vector */
        d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_UCHAR_VEC);
    }
    else {
        /* Image type must be uint32_t */
        d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_ULONG);
    }

    d_ptr->m_ss_img->save_image (ss_img_fn);
}

void
Segmentation::save_labelmap (const std::string& labelmap_fn)
{
    d_ptr->m_labelmap->save_image (labelmap_fn);
}

void
Segmentation::save_prefix (const std::string &output_prefix,
    const std::string& extension)
{
    if (!d_ptr->m_ss_img) {
        return;
    }

    if (!d_ptr->m_cxt) {
        printf ("WTF???\n");
    }

    for (size_t i = 0; i < d_ptr->m_cxt->num_structures; i++)
    {
        std::string fn;
        Rtss_roi *curr_structure = d_ptr->m_cxt->slist[i];
        int bit = curr_structure->bit;

        if (bit == -1) continue;
        UCharImageType::Pointer prefix_img 
            = ss_img_extract_bit (d_ptr->m_ss_img, bit);

        fn = string_format ("%s/%s.%s", 
            output_prefix.c_str(),
            curr_structure->name.c_str(),
            extension.c_str());
        itk_image_save (prefix_img, fn.c_str());
    }
}

void
Segmentation::save_prefix (const char *output_prefix)
{
    this->save_prefix (std::string (output_prefix));
}

void
Segmentation::save_ss_list (const std::string& ss_list_fn)
{
    ss_list_save (d_ptr->m_cxt.get(), ss_list_fn.c_str());
}

void
Segmentation::save_xio (
    const Rt_study_metadata::Pointer& rsm,
    Xio_ct_transform *xio_transform, 
    Xio_version xio_version, 
    const std::string &output_dir
)
{
    xio_structures_save (rsm, d_ptr->m_cxt.get(), xio_transform,
        xio_version, output_dir.c_str());
}

UInt32ImageType::Pointer
Segmentation::get_ss_img_uint32 (void)
{
    if (!d_ptr->m_ss_img) {
        print_and_exit ("Sorry, can't get_ss_img()\n");
    }
    d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_ULONG);
    return d_ptr->m_ss_img->m_itk_uint32;
}

UCharVecImageType::Pointer
Segmentation::get_ss_img_uchar_vec (void)
{
    if (!d_ptr->m_ss_img) {
        print_and_exit ("Sorry, can't get_ss_img()\n");
    }
    d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_UCHAR_VEC);
    return d_ptr->m_ss_img->m_itk_uchar_vec;
}

void
Segmentation::apply_dicom_dir (const Rt_study_metadata::Pointer& rsm)
{
    if (!d_ptr->m_cxt) {
        return;
    }

    if (!rsm || !rsm->slice_list_complete()) {
        return;
    }

    d_ptr->m_cxt->apply_slice_index (rsm);
}

void
Segmentation::convert_ss_img_to_cxt (void)
{
    /* Only convert if ss_img found */
    if (!d_ptr->m_ss_img) {
        return;
    }

    /* Allocate memory for cxt */
    bool use_existing_bits;
    if (d_ptr->m_cxt) {
        use_existing_bits = true;
    }
    else {
        d_ptr->m_cxt = Rtss::New();
        use_existing_bits = false;
    }

    /* Copy geometry from ss_img to cxt */
    d_ptr->m_cxt->set_geometry (d_ptr->m_ss_img);

    if (d_ptr->m_ss_img->m_type == PLM_IMG_TYPE_GPUIT_UCHAR_VEC
        || d_ptr->m_ss_img->m_type == PLM_IMG_TYPE_ITK_UCHAR_VEC) 
    {
        /* Image type must be uchar vector */
        d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_UCHAR_VEC);

        /* Do extraction */
        lprintf ("Doing extraction\n");
        ::cxt_extract (d_ptr->m_cxt.get(), d_ptr->m_ss_img->m_itk_uchar_vec, 
            -1, use_existing_bits);
    }
    else {
        /* Image type must be uint32_t */
        d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_ULONG);

        /* Do extraction */
        lprintf ("Doing extraction\n");
        ::cxt_extract (d_ptr->m_cxt.get(), d_ptr->m_ss_img->m_itk_uint32, -1, 
            use_existing_bits);
    }

    d_ptr->m_rtss_valid = true;
}

void
Segmentation::convert_to_uchar_vec (void)
{
    if (!d_ptr->m_ss_img) {
        print_and_exit (
            "Error: convert_to_uchar_vec() requires an image");
    }
    d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_UCHAR_VEC);
}

void
Segmentation::cxt_extract (void)
{
    if (d_ptr->m_ss_img && !d_ptr->m_rtss_valid) {
        this->convert_ss_img_to_cxt ();
    }
}

void
Segmentation::cxt_re_extract (void)
{
    d_ptr->m_cxt->free_all_polylines ();
    if (d_ptr->m_ss_img->m_type == PLM_IMG_TYPE_GPUIT_UCHAR_VEC
        || d_ptr->m_ss_img->m_type == PLM_IMG_TYPE_ITK_UCHAR_VEC) 
    {
        d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_UCHAR_VEC);
        ::cxt_extract (d_ptr->m_cxt.get(), d_ptr->m_ss_img->m_itk_uchar_vec, 
            d_ptr->m_cxt->num_structures, true);
    }
    else {
        d_ptr->m_ss_img->convert (PLM_IMG_TYPE_ITK_ULONG);
        ::cxt_extract (d_ptr->m_cxt.get(), d_ptr->m_ss_img->m_itk_uint32, 
            d_ptr->m_cxt->num_structures, true);
    }

    d_ptr->m_rtss_valid = true;
}

void
Segmentation::prune_empty (void)
{
    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->prune_empty ();
    }
}

void
Segmentation::rasterize (
    Plm_image_header *pih,
    bool want_labelmap,
    bool xor_overlapping
)
{
    /* Rasterize structure sets */
    Rasterizer rasterizer;

#if (PLM_CONFIG_USE_SS_IMAGE_VEC)
    printf ("Setting use_ss_img_vec to true!\n");
    bool use_ss_img_vec = true;
#else
    bool use_ss_img_vec = false;
#endif

    printf ("Rasterizing...\n");
    rasterizer.rasterize (d_ptr->m_cxt.get(), pih, false, want_labelmap, true,
        use_ss_img_vec, xor_overlapping);

    /* Convert rasterized structure sets from vol to plm_image */
    printf ("Converting...\n");
    if (want_labelmap) {
        d_ptr->m_labelmap = Plm_image::New();
        d_ptr->m_labelmap->set_volume (rasterizer.labelmap_vol);
        rasterizer.labelmap_vol = 0;
    }
    d_ptr->m_ss_img = Plm_image::New();

    if (use_ss_img_vec) {
        d_ptr->m_ss_img->set_itk (rasterizer.m_ss_img->m_itk_uchar_vec);
    }
    else {
        Volume::Pointer v = rasterizer.m_ss_img->get_volume();
        d_ptr->m_ss_img->set_volume (v);
    }
    lprintf ("Finished rasterization.\n");

    d_ptr->m_ss_img_valid = true;
}

void
Segmentation::set_geometry (const Plm_image_header *pih)
{
    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->set_geometry (pih);
    }
}

void
Segmentation::find_rasterization_geometry (Plm_image_header *pih)
{
    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->find_rasterization_geometry (pih);
    }
}

Segmentation::Pointer 
Segmentation::warp_nondestructive (
    const Xform::Pointer& xf, 
    Plm_image_header *pih, 
    bool use_itk) const
{
    Segmentation::Pointer rtss_warped = Segmentation::New ();

    rtss_warped->d_ptr->m_cxt = Rtss::New (
        Rtss::clone_empty (0, d_ptr->m_cxt.get()));
    rtss_warped->d_ptr->m_rtss_valid = false;

    if (d_ptr->m_labelmap) {
        printf ("Warping labelmap.\n");
        Plm_image::Pointer tmp = Plm_image::New();
        plm_warp (tmp, 0, xf, pih, d_ptr->m_labelmap, 0, use_itk, 0);
        rtss_warped->d_ptr->m_labelmap = tmp;
        rtss_warped->d_ptr->m_labelmap->convert (PLM_IMG_TYPE_ITK_ULONG);
    }

    if (d_ptr->m_ss_img) {
        printf ("Warping ss_img.\n");
        Plm_image::Pointer tmp = Plm_image::New();
        plm_warp (tmp, 0, xf, pih, d_ptr->m_ss_img, 0, use_itk, 0);
        rtss_warped->d_ptr->m_ss_img = tmp;
    }

    return rtss_warped;
}

void
Segmentation::warp (
    const Xform::Pointer& xf, 
    Plm_image_header *pih, 
    bool use_itk)
{
    if (d_ptr->m_labelmap) {
        printf ("Warping labelmap.\n");
        Plm_image::Pointer tmp = Plm_image::New();
        plm_warp (tmp, 0, xf, pih, d_ptr->m_labelmap, 0, use_itk, 0);
        d_ptr->m_labelmap = tmp;
        d_ptr->m_labelmap->convert (PLM_IMG_TYPE_ITK_ULONG);
    }

    if (d_ptr->m_ss_img) {
        printf ("Warping ss_img.\n");
        Plm_image::Pointer tmp = Plm_image::New();
        plm_warp (tmp, 0, xf, pih, d_ptr->m_ss_img, 0, use_itk, 0);
        d_ptr->m_ss_img = tmp;
    }

    /* The cxt polylines are now obsolete */
    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->free_all_polylines ();
    }
    d_ptr->m_rtss_valid = false;
}

void
Segmentation::warp (
    const Xform::Pointer& xf, 
    Plm_image_header *pih, 
    Warp_parms *parms)
{
    this->warp (xf, pih, parms->use_itk);
}

bool
Segmentation::have_ss_img ()
{
    return d_ptr->m_ss_img != 0;
}

void
Segmentation::set_ss_img (UCharImageType::Pointer ss_img)
{
    d_ptr->m_ss_img = Plm_image::New();
    d_ptr->m_ss_img->set_itk (ss_img);

    if (d_ptr->m_cxt) {
        d_ptr->m_cxt->free_all_polylines ();
    }
    d_ptr->m_rtss_valid = false;
    d_ptr->m_ss_img_valid = true;
}

Plm_image::Pointer
Segmentation::get_ss_img ()
{
    return d_ptr->m_ss_img;
}

bool
Segmentation::have_structure_set ()
{
    return d_ptr->m_cxt != 0;
}

Rtss::Pointer&
Segmentation::get_structure_set ()
{
    return d_ptr->m_cxt;
}

Rtss *
Segmentation::get_structure_set_raw ()
{
    return d_ptr->m_cxt.get();
}

void
Segmentation::set_structure_set (Rtss::Pointer& rtss_ss)
{
    d_ptr->m_cxt = rtss_ss;

    d_ptr->m_rtss_valid = true;
    d_ptr->m_ss_img_valid = false;
}

void
Segmentation::set_structure_set (Rtss *rtss_ss)
{
    d_ptr->m_cxt.reset (rtss_ss);

    d_ptr->m_rtss_valid = true;
    d_ptr->m_ss_img_valid = false;
}

void
Segmentation::set_structure_image (
    UCharImageType::Pointer uchar_img, 
    unsigned int bit
)
{
    /* Figure out which bit of which byte to change */
    unsigned int uchar_no = bit / 8;
    unsigned int bit_no = bit % 8;
    unsigned char bit_mask = 1 << bit_no;

    /* Expand vector length if needed */
    UCharVecImageType::Pointer ss_img = d_ptr->m_ss_img->itk_uchar_vec ();
    if (uchar_no > ss_img->GetVectorLength()) {
        this->broaden_ss_image (uchar_no);
    }

    /* Set up iterators for looping through images */
    typedef itk::ImageRegionConstIterator< UCharImageType > 
        UCharIteratorType;
    typedef itk::ImageRegionIterator< UCharVecImageType > 
        UCharVecIteratorType;
    UCharIteratorType uchar_img_it (uchar_img, 
        uchar_img->GetLargestPossibleRegion());
    UCharVecIteratorType ss_img_it (ss_img, 
        ss_img->GetLargestPossibleRegion());

    /* Loop through voxels, or'ing them into ss_img */
    /* GCS FIX: This is inefficient, due to undesirable construct 
       and destruct of itk::VariableLengthVector of each pixel */
    for (uchar_img_it.GoToBegin(), ss_img_it.GoToBegin();
        !uchar_img_it.IsAtEnd();
        ++uchar_img_it, ++ss_img_it
    ) {
        unsigned char u = uchar_img_it.Get ();
        if (!u) continue;

        itk::VariableLengthVector<unsigned char> v 
            = ss_img_it.Get ();
        v[uchar_no] |= bit_mask;
        ss_img_it.Set (v);
    }
}

void
Segmentation::resample (float spacing[3])
{
    d_ptr->m_ss_img->set_itk (
        resample_image (d_ptr->m_ss_img->itk_uchar_vec (), spacing));
}


/* -----------------------------------------------------------------------
   Protected member functions
   ----------------------------------------------------------------------- */
void
Segmentation::initialize_ss_image (
    const Plm_image_header& pih, int vector_length)
{
    UCharVecImageType::Pointer ss_img;
    Plm_image_header ss_img_pih;

    /* Create ss_image with same resolution as first image */
    d_ptr->m_ss_img = Plm_image::New ();
    ss_img = UCharVecImageType::New ();
    itk_image_set_header (ss_img, pih);
    ss_img->SetVectorLength (vector_length);
    ss_img->Allocate ();

    /* GCS NOTE: For some reason, ss_img->FillBuffer (0) 
       doesn't do what I want. */
    itk::VariableLengthVector<unsigned char> v;
    v.SetSize (vector_length);
    v.Fill (0);
    ss_img->FillBuffer (v);

    d_ptr->m_ss_img->set_itk (ss_img);
    Plm_image_header::clone (&ss_img_pih, &pih);

    /* Create ss_list to hold strucure names */
    d_ptr->m_cxt = Rtss::New();
    d_ptr->m_cxt->set_geometry (d_ptr->m_ss_img);
}

void
Segmentation::broaden_ss_image (int new_vector_length)
{
    /* Get old image */
    UCharVecImageType::Pointer old_ss_img = d_ptr->m_ss_img->itk_uchar_vec ();
    Plm_image_header pih (old_ss_img);

    /* Create new image */
    UCharVecImageType::Pointer new_ss_img = UCharVecImageType::New ();
    itk_image_set_header (new_ss_img, pih);
    new_ss_img->SetVectorLength (new_vector_length);
    new_ss_img->Allocate ();

    /* Create "pixels" */
    itk::VariableLengthVector<unsigned char> v_old;
    itk::VariableLengthVector<unsigned char> v_new;
    int old_vector_length = old_ss_img->GetVectorLength();
    v_old.SetSize (old_vector_length);
    v_new.SetSize (new_vector_length);
    v_new.Fill (0);

    /* Loop through image */
    typedef itk::ImageRegionIterator< 
        UCharVecImageType > UCharVecIteratorType;
    UCharVecIteratorType it_old (
        old_ss_img, old_ss_img->GetLargestPossibleRegion());
    UCharVecIteratorType it_new (
        new_ss_img, new_ss_img->GetLargestPossibleRegion());
    for (it_old.GoToBegin(), it_new.GoToBegin(); 
         !it_old.IsAtEnd(); 
         ++it_old, ++it_new)
    {
        /* Copy old pixel bytes into new */
        v_old = it_old.Get();
        for (int i = 0; i < old_vector_length; i++) {
            v_new[i] = v_old[i];
        }
        it_new.Set (v_new);
    }

    /* Fixate new image */
    d_ptr->m_ss_img->set_itk (new_ss_img);
}
