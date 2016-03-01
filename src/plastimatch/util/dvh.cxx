/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <time.h>
#include "itkSubtractImageFilter.h"
#include "itkImageRegionIterator.h"

#include "dvh.h"
#include "dvh_p.h"
#include "file_util.h"
#include "itk_resample.h"
#include "make_string.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "rt_study.h"
#include "rtss_roi.h"
#include "rtss.h"
#include "segmentation.h"

Dvh_private::Dvh_private () {
    this->dose_units = Dvh::default_dose_units ();
    this->normalization = Dvh::default_normalization ();
    this->histogram_type = Dvh::default_histogram_type ();
    this->num_bins = Dvh::default_histogram_num_bins ();
    this->bin_width = Dvh::default_histogram_bin_width ();
}

Dvh_private::~Dvh_private () {
}

Dvh::Dvh () {
    this->d_ptr = new Dvh_private;
}

Dvh::~Dvh () {
    delete this->d_ptr;
}

void
Dvh::set_structure_set_image (
    const char* ss_image_fn, const char *ss_list_fn)
{
    d_ptr->rtss = new Segmentation;
    d_ptr->rtss->load (ss_image_fn, ss_list_fn);
}

void 
Dvh::set_dose_image (const char* image_fn)
{
    d_ptr->dose = plm_image_load_native (image_fn);
}

void 
Dvh::set_dose_units (enum Dvh_units units)
{
    d_ptr->dose_units = units;
}

void 
Dvh::set_dvh_parameters (enum Dvh_normalization normalization,
    enum Histogram_type histogram_type, int num_bins, float bin_width)
{
    d_ptr->normalization = normalization;
    d_ptr->histogram_type = histogram_type;
    d_ptr->num_bins = num_bins;
    d_ptr->bin_width = bin_width;
}

void
Dvh::run ()
{
    int *hist;
    int *struct_vox;
    int bin;
    float dose_spacing[3];
    float ss_spacing[3];
    float ss_ori[3];
    plm_long ss_dim[3];

    FloatImageType::Pointer dose_img = d_ptr->dose->itk_float ();
#if (PLM_CONFIG_USE_SS_IMAGE_VEC)
    UCharVecImageType::Pointer ss_img = d_ptr->rtss->get_ss_img_uchar_vec ();
#else
    UInt32ImageType::Pointer ss_img = d_ptr->rtss->get_ss_img_uint32 ();
#endif

    /* GCS HACK: This should go into rtss.cxx */
    Rtss *ss_list;
    if (d_ptr->rtss->have_structure_set()) {
        ss_list = d_ptr->rtss->get_structure_set_raw();
    } else {
        ss_list = new Rtss;
#if (PLM_CONFIG_USE_SS_IMAGE_VEC)
        int num_structures = ss_img->GetVectorLength() * 8;
#else
        int num_structures = 32;
#endif
        for (int i = 0; i < num_structures; i++) {
            ss_list->add_structure ("Unknown Structure", 
                "255 255 0", i+1, i);
        }
    }

    /* Create histogram */
    std::cout << "Creating Histogram..." << std::endl;
    printf ("Your Histogram will have %d bins and will be %f Gy large\n",
        d_ptr->num_bins, d_ptr->bin_width);
    hist = (int*) malloc (sizeof(int) * ss_list->num_structures 
        * d_ptr->num_bins);
    memset (hist, 0, sizeof(int) * ss_list->num_structures 
        * d_ptr->num_bins);
    struct_vox = (int*) malloc (sizeof(int) * ss_list->num_structures);
    memset (struct_vox, 0, sizeof(int) * ss_list->num_structures);
   
    /* Is voxel size the same? */
    std::cout << "checking voxel size..." << std::endl;
    dose_spacing[0]=dose_img->GetSpacing()[0];
    dose_spacing[1]=dose_img->GetSpacing()[1];
    dose_spacing[2]=dose_img->GetSpacing()[2];
    std::cout << dose_spacing[0] << " " 
        << dose_spacing[1] << " " << dose_spacing[2] << "\n";
    ss_spacing[0]=ss_img->GetSpacing()[0];
    ss_spacing[1]=ss_img->GetSpacing()[1];
    ss_spacing[2]=ss_img->GetSpacing()[2];
    std::cout << ss_spacing[0] << " " 
        << ss_spacing[1] << " " << ss_spacing[2] << "\n";

    if (dose_spacing[0] != ss_spacing[0] 
        || dose_spacing[1] != ss_spacing[1] 
        || dose_spacing[2] != ss_spacing[2])
    {
        std::cout << "dose voxel " 
            << dose_spacing[0] << " " 
            << dose_spacing[1] << " " 
            << dose_spacing[2] << std::endl;
        std::cout << "ss voxel " 
            << ss_spacing[0] << " " 
            << ss_spacing[1] << " " 
            << ss_spacing[2] << std::endl;
        std::cout << "Resampling" << std::endl;

        /*resample volume*/
        ss_ori[0]=ss_img->GetOrigin()[0];
        ss_ori[1]=ss_img->GetOrigin()[1];
        ss_ori[2]=ss_img->GetOrigin()[2];
        ss_dim[0]=ss_img->GetLargestPossibleRegion().GetSize()[0];
        ss_dim[1]=ss_img->GetLargestPossibleRegion().GetSize()[1];
        ss_dim[2]=ss_img->GetLargestPossibleRegion().GetSize()[2];

        /* GCS FIX: Direction cosines */
        Plm_image_header pih (ss_dim, ss_ori, ss_spacing, 0);
        FloatImageType::Pointer resampled 
            = resample_image (dose_img, &pih, 0, 1);
        dose_img=resampled;

    } else {
        std::cout 
            << "Dose and ss-img have the same size. Resample not necessary.\n";
    }

    /* Declare iterators */
    typedef itk::ImageRegionConstIterator < FloatImageType > 
        FloatIteratorType;
    FloatIteratorType it_d (dose_img, dose_img->GetRequestedRegion ());
#if (PLM_CONFIG_USE_SS_IMAGE_VEC)
    typedef itk::ImageRegionConstIterator < UCharVecImageType > 
        UCharVecIteratorType;
    UCharVecIteratorType it_s (ss_img, ss_img->GetRequestedRegion ());
#else
    typedef itk::ImageRegionConstIterator < UInt32ImageType > 
        UInt32IteratorType;
    UInt32IteratorType it_s (ss_img, ss_img->GetRequestedRegion ());
#endif

    /* Loop through dose & ss images */
    for (it_d.GoToBegin(), it_s.GoToBegin(); 
         !it_d.IsAtEnd(); 
         ++it_d, ++it_s)
    {
        float d = it_d.Get();
#if (PLM_CONFIG_USE_SS_IMAGE_VEC)
	itk::VariableLengthVector<unsigned char> s = it_s.Get();
#else
        uint32_t s = it_s.Get();
#endif

        /* Convert from cGy to Gy */
        if (d_ptr->dose_units == DVH_UNITS_CGY) {
            d = d / 100;
        }

        /* Compute the bin */
        bin = (int) floor ((d+(0.5*d_ptr->bin_width)) / d_ptr->bin_width);
        if (bin < 0) {
            bin = 0;
        } else if (bin > (d_ptr->num_bins-1)) {
            bin = d_ptr->num_bins - 1;
        }

        for (size_t sno = 0; sno < ss_list->num_structures; sno++) {
            Rtss_roi *curr_structure = ss_list->slist[sno];
                    
            /* Is this pixel in the current structure? */
#if (PLM_CONFIG_USE_SS_IMAGE_VEC)
            int curr_bit = curr_structure->bit;
            unsigned int uchar_no = curr_bit / 8;
            unsigned int bit_no = curr_bit % 8;
            unsigned char bit_mask = 1 << bit_no;
            if (uchar_no > ss_img->GetVectorLength()) {
                print_and_exit (
                    "Error: bit %d was requested from image of %d bits\n", 
                    curr_bit, ss_img->GetVectorLength() * 8);
            }
            bool in_struct = s[uchar_no] & bit_mask;
#else            
            uint32_t in_struct = s & (1 << curr_structure->bit);
#endif

            /* If so, update histogram & structure size */
            if (in_struct) {
                struct_vox[sno] ++;
                hist[bin*ss_list->num_structures + sno] ++;
            }
        }
    }

    /* Convert histogram to cumulative histogram */
    if (d_ptr->histogram_type == DVH_CUMULATIVE_HISTOGRAM) {
        for (size_t sno = 0; sno < ss_list->num_structures; sno++) {
            int cum = 0;
            for (bin = d_ptr->num_bins - 1; bin >= 0; bin--) {
                cum = cum + hist[bin*ss_list->num_structures + sno];
                hist[bin*ss_list->num_structures + sno] = cum;
            }
        }
    }

    /* Create output string */
    d_ptr->output_string = "Dose (Gy)";
    for (size_t sno = 0; sno < ss_list->num_structures; sno++) {
        Rtss_roi *curr_structure = ss_list->slist[sno];
        d_ptr->output_string += ",";
        d_ptr->output_string += curr_structure->name.c_str();
    }
    d_ptr->output_string += "\n";
    for (plm_long bin = 0; bin < d_ptr->num_bins; bin++) {
        d_ptr->output_string += make_string (bin * d_ptr->bin_width);
        for (size_t sno = 0; sno < ss_list->num_structures; sno++) {
            int val = hist[bin*ss_list->num_structures + sno];
            d_ptr->output_string += ",";
            if (struct_vox[sno] == 0) {
                d_ptr->output_string += "0";
            }
            else if (d_ptr->normalization == DVH_NORMALIZATION_PCT) {
                float fval = ((float) val) / struct_vox[sno];
                d_ptr->output_string += make_string (fval);
            }
            else {
                d_ptr->output_string += make_string (val);
            }
        }
        d_ptr->output_string += "\n";
    }
}

void
Dvh::save_csv (const char* csv_fn)
{
    FILE *fp = plm_fopen (csv_fn, "w");
    fprintf (fp, "%s", d_ptr->output_string.c_str());
    fclose (fp);
}
