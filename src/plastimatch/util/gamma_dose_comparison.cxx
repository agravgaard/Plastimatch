/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRegion.h"

#include "gamma_dose_comparison.h"
#include "itk_resample.h"
#include "itk_threshold.h"
#include "logfile.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_math.h"
#include "string_util.h"

//21 = 20 +1, to make a room for >2.0 histogram
#define MAX_NUM_HISTOGRAM_BIN 21

/*! \enum Gamma_output_mode Selector for output image type (gamma, or binary pass/fail)
*/
enum Gamma_labelmap_mode {
    NONE,  /*!< disable output binary uchar labelmap for gamma */
    PASS,  /*!< output binary (1/0) image of type uchar, 1 if gamma<1 */ 
    FAIL   /*!< output binary (1/0) image of type uchar, 1 if gamma>1 */ 
};

class Gamma_dose_comparison_private {
public:
    Gamma_dose_comparison_private ()
    {
        labelmap_out = 0;

        have_gamma_image = false;
        gamma_image = Plm_image::New();

        dta_tolerance = 3.0;
        dose_difference_tolerance = 0.03;
        gamma_max = 2.0;
        mode = NONE;

        have_reference_dose = false;
        reference_dose = 0.f;

        dose_max = 0.f;

        have_analysis_thresh = false;
        analysis_thresh = 0.f;
        analysis_num_vox = 0;
        analysis_num_pass = 0;

        str_gamma_report ="";
        b_local_gamma = false;
        b_compute_full_region = false;
        f_inherent_resample_mm = -1.0;

        voxels_in_mask = 0;
        voxels_in_image = 0;

        b_resample_nn = false;				
        b_interp_search = false;

        b_ref_only_threshold = false;

        for (int i = 0; i < MAX_NUM_HISTOGRAM_BIN; i++)	{
            arr_gamma_histo[i] = 0;
        }

        progress_callback = 0;
    }
public:
    Plm_image::Pointer img_in1; /*!< input dose image 1 for gamma analysis*/
    Plm_image::Pointer img_in2; /*!< input dose image 2 for gamma analysis*/
    Plm_image::Pointer img_mask; /*!< input mask image for gamma analysis*/
    Plm_image *labelmap_out; /*!< output uchar type labelmap, voxel value = 1/0 for pass/fail */

    /* Gamma image is float type image, voxel value = calculated gamma value */
    bool have_gamma_image;
    Plm_image::Pointer gamma_image;

    /* distance-to-agreement (DTA) criterion, input parameter */ 
    float dta_tolerance;
    /* dose-difference criterion, in percent.  Either as percent of 
       prescription (as requested by user), or as percent of 
       computed dose_max */
    float dose_difference_tolerance;
    /* maximum gamma to calculate */
    float gamma_max;
    Gamma_labelmap_mode mode; /*!< output mode selector for 3D Slicer plugin*/

    /* reference dose value, used for gamma analysis and analysis 
       thresholding.  */
    bool have_reference_dose;
    float reference_dose;

    /* maximum dose (max voxel value) in the reference dose, 
       set by find_reference_max_dose() */
    float dose_max;

    /* analysis thresholding, limits statistics to dose values above 
       the threshold */
    bool have_analysis_thresh;
    float analysis_thresh;
    plm_long analysis_num_vox;
    plm_long analysis_num_pass;


    /*extended features by YK*/
    std::string str_gamma_report; //used option information, gamma histogram
    bool b_local_gamma;
    bool b_compute_full_region;
    float f_inherent_resample_mm;
    bool b_ref_only_threshold;//this option is needed for a situation where a ref region is below threshold and comp region is above threshold.

    plm_long voxels_in_mask;
    plm_long voxels_in_image;

    bool b_resample_nn;

    bool b_interp_search;
    //0~ max (default: 2.0), bin size: 0.1
    int arr_gamma_histo[MAX_NUM_HISTOGRAM_BIN];

    /* callback routine for updating progress bar in SlicerRT */
    void (*progress_callback) (float);

public:
    void do_mask_threshold ();
    void find_reference_max_dose ();
    void do_gamma_analysis ();
    void do_gamma_threshold ();
    void compose_report();//fill str_gamma_report;
};

Gamma_dose_comparison::Gamma_dose_comparison () {
    d_ptr = new Gamma_dose_comparison_private;
}

Gamma_dose_comparison::~Gamma_dose_comparison () {
    delete d_ptr;
}

void 
Gamma_dose_comparison::set_reference_image (const char* image_fn)
{
    d_ptr->img_in1 = Plm_image::New (image_fn);
}

void 
Gamma_dose_comparison::set_reference_image (const FloatImageType::Pointer image)
{
    d_ptr->img_in1 = Plm_image::New (image);
}

void 
Gamma_dose_comparison::set_reference_image (const Plm_image::Pointer& image)
{
    d_ptr->img_in1 = image;
}

void 
Gamma_dose_comparison::set_compare_image (const char* image_fn)
{
    d_ptr->img_in2 = Plm_image::New (image_fn);
}

void 
Gamma_dose_comparison::set_compare_image (const Plm_image::Pointer& image)
{
    d_ptr->img_in2 = image;
}

void 
Gamma_dose_comparison::set_compare_image (const FloatImageType::Pointer image)
{
    d_ptr->img_in2 = Plm_image::New (image);
}

void 
Gamma_dose_comparison::set_mask_image (const std::string& image_fn)
{
    d_ptr->img_mask = Plm_image::New (image_fn);
}

void 
Gamma_dose_comparison::set_mask_image (const char* image_fn)
{
    d_ptr->img_mask = Plm_image::New (image_fn);
}

void 
Gamma_dose_comparison::set_mask_image (const Plm_image::Pointer& image)
{
    d_ptr->img_mask = image;
}

void 
Gamma_dose_comparison::set_mask_image (const UCharImageType::Pointer image)
{
    d_ptr->img_mask = Plm_image::New (image);
}

float
Gamma_dose_comparison::get_spatial_tolerance ()
{
    return d_ptr->dta_tolerance;
}

void 
Gamma_dose_comparison::set_spatial_tolerance (float spatial_tol)
{
    d_ptr->dta_tolerance = spatial_tol;
}

float
Gamma_dose_comparison::get_dose_difference_tolerance ()
{
    return d_ptr->dose_difference_tolerance;
}

void 
Gamma_dose_comparison::set_dose_difference_tolerance (float dose_tol)
{
    d_ptr->dose_difference_tolerance = dose_tol;
}

void 
Gamma_dose_comparison::set_reference_dose (float dose)
{
    d_ptr->reference_dose = dose;
    d_ptr->have_reference_dose = true;
}

void 
Gamma_dose_comparison::set_analysis_threshold (float thresh)
{
    d_ptr->have_analysis_thresh = true;
    d_ptr->analysis_thresh = thresh; //0.1 = 10%
}

void 
Gamma_dose_comparison::set_gamma_max (float gamma_max)
{
    d_ptr->gamma_max = gamma_max;
}

void 
Gamma_dose_comparison::set_progress_callback (
    void (*progress_callback)(float))
{
    d_ptr->progress_callback = progress_callback;
}

void 
Gamma_dose_comparison::run ()
{
    if (!d_ptr->have_reference_dose) {
        d_ptr->find_reference_max_dose ();
        d_ptr->reference_dose = d_ptr->dose_max;
    }
    d_ptr->have_gamma_image = true;

    //Edited by YK
    // if the reference image is too sparse, resample it with smaller 
    // spacing. (e.g. 1 mm)
    if (d_ptr->f_inherent_resample_mm > 0.0){		
        float spacing[3];
        spacing[0] = d_ptr->f_inherent_resample_mm;
        spacing[1] = d_ptr->f_inherent_resample_mm;
        spacing[2] = d_ptr->f_inherent_resample_mm;
        resample_image_with_fixed_spacing(d_ptr->img_in1, spacing);
    }	

    // Resample mask 
    if (d_ptr->img_mask) {
        // Set values to either 1 or 0
        d_ptr->do_mask_threshold ();
        // Resample as float
        resample_image_to_reference (d_ptr->img_in1, d_ptr->img_mask);
        // Threshold float image at value of 0.5
        d_ptr->img_mask->set_itk (
            itk_threshold_above (d_ptr->img_mask->itk_float(), 0.5));
    }

    resample_image_to_reference (d_ptr->img_in1, d_ptr->img_in2);
    lprintf("Gamma calculation is under progress...\n");

    //YKdebug
    //d_ptr->img_in1->save_image("<image1_path>");
    //d_ptr->img_in2->save_image("<image2_path>");
    //YKdebug
	
    d_ptr->do_gamma_analysis ();

    //compose a report string
    d_ptr->compose_report();
}

Plm_image::Pointer
Gamma_dose_comparison::get_gamma_image ()
{
    if (!d_ptr->have_gamma_image) {
        this->run();
    }
    return d_ptr->gamma_image;
}

FloatImageType::Pointer
Gamma_dose_comparison::get_gamma_image_itk ()
{
    return get_gamma_image()->itk_float();
}

Plm_image*
Gamma_dose_comparison::get_pass_image ()
{
    if (!d_ptr->have_gamma_image) {
        this->run();
    }
    d_ptr->mode = PASS;
    d_ptr->do_gamma_threshold ();
    return d_ptr->labelmap_out;
}

UCharImageType::Pointer
Gamma_dose_comparison::get_pass_image_itk ()
{
    return get_pass_image()->itk_uchar();
}

Plm_image*
Gamma_dose_comparison::get_fail_image ()
{
    if (!d_ptr->have_gamma_image) {
        this->run();
    }
    d_ptr->mode = FAIL;
    d_ptr->do_gamma_threshold ();
    return d_ptr->labelmap_out;
}

UCharImageType::Pointer
Gamma_dose_comparison::get_fail_image_itk ()
{
    return get_fail_image()->itk_uchar();
}

float
Gamma_dose_comparison::get_pass_fraction ()
{
    if (d_ptr->analysis_num_vox < 1) {
        return 0.f;
    }
    return d_ptr->analysis_num_pass / (float) d_ptr->analysis_num_vox;
}

int
Gamma_dose_comparison::get_analysis_num_vox()
{    
    return (int)d_ptr->analysis_num_vox;
}

int
Gamma_dose_comparison::get_passed_num_vox()
{
    return (int)d_ptr->analysis_num_pass;
}

float Gamma_dose_comparison::get_reference_dose()
{
    return (float)d_ptr->reference_dose;
}


void 
Gamma_dose_comparison::resample_image_to_reference (
    const Plm_image::Pointer& image_reference,
    Plm_image::Pointer& image_moving)
{
    Plm_image_header pih;
    pih.set_from_plm_image (image_reference);

    itk::Image<float, 3>::Pointer resampledMovingImage = resample_image (
        image_moving->itk_float(),
        &pih,
        0.f,
        !this->is_resample_nn()
    );

    image_moving->set_itk(resampledMovingImage);
}

void 
Gamma_dose_comparison::resample_image_with_fixed_spacing (
    Plm_image::Pointer& input_img, float spacing[3])
{	
    Plm_image_header pih;
    pih.set_from_plm_image (input_img);	

    ///* Auto-adjust, keep same image extent */
    float extent[3];
    pih.get_image_extent (extent);
	
    plm_long new_dim[3];
    for (int d = 0; d < 3; d++) {
	new_dim[d] = 1 + FLOOR_PLM_LONG (extent[d] / spacing[d]);
    }
    pih.set_spacing(spacing);
    pih.set_dim (new_dim);	

    itk::Image<float, 3>::Pointer resampledImage = resample_image (
        input_img->itk_float(),
        &pih,
        0.f,
        !this->is_resample_nn()
    );
    input_img->set_itk(resampledImage);
}

/* -------------------------------------------------------------------------
   Private functions
   ------------------------------------------------------------------------- */
void 
Gamma_dose_comparison_private::find_reference_max_dose ()
{
    FloatImageType::Pointer itk_1 = img_in1->itk_float();
    typedef itk::ImageRegionIteratorWithIndex< 
        FloatImageType > FloatIteratorType;
    typedef itk::ImageRegion<3> FloatRegionType;
    
    FloatRegionType all_of_img1 = itk_1->GetLargestPossibleRegion();
    FloatIteratorType itk_1_iterator (itk_1, all_of_img1);
    float maxlevel1=-1e20;
    for (itk_1_iterator.GoToBegin(); 
         !itk_1_iterator.IsAtEnd(); 
         ++itk_1_iterator)
    {
        float level1 = itk_1_iterator.Get();
        if (level1 > maxlevel1) maxlevel1 = level1;         
    } 
    this->dose_max = maxlevel1;

    lprintf ("Gamma dose max is %f\n", this->dose_max);
}

void 
Gamma_dose_comparison_private::do_gamma_analysis ()
{ 
    float spacing_in[3], origin_in[3];
    plm_long dim_in[3];
    Plm_image_header pih;
    float gamma;

    FloatImageType::Pointer itk_1 = img_in1->itk_float();
    FloatImageType::Pointer itk_2 = img_in2->itk_float();
    UCharImageType::Pointer mask_img;
    if (img_mask) {
        mask_img = img_mask->itk_uchar();
    }

    pih.set_from_itk_image (itk_1);
    pih.get_dim (dim_in);
    pih.get_origin (origin_in);
    pih.get_spacing (spacing_in);

    // Create ITK image for gamma output, "pass", "fail" and combined 
    FloatImageType::SizeType sz;
    FloatImageType::IndexType st;
    FloatImageType::RegionType rg;
    FloatImageType::PointType og;
    FloatImageType::SpacingType sp;
    FloatImageType::DirectionType dc;
    for (int d1 = 0; d1 < 3; d1++) {
        st[d1] = 0;
        sz[d1] = dim_in[d1];
        sp[d1] = spacing_in[d1];
        og[d1] = origin_in[d1];
    }
    rg.SetSize (sz);
    rg.SetIndex (st);
    dc = pih.GetDirection();

    FloatImageType::Pointer gamma_img = FloatImageType::New();
    UCharImageType::Pointer gamma_labelmap = UCharImageType::New();

    gamma_img->SetRegions (rg);
    gamma_img->SetOrigin (og);
    gamma_img->SetSpacing (sp);
    gamma_img->SetDirection (dc);
    gamma_img->Allocate();

    gamma_labelmap->SetRegions (rg);
    gamma_labelmap->SetOrigin (og);
    gamma_labelmap->SetSpacing (sp);
    gamma_labelmap->SetDirection (dc);
    gamma_labelmap->Allocate();

    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > UCharIteratorType;
    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > FloatIteratorType;
    typedef itk::ImageRegion<3> FloatRegionType;
    
    FloatRegionType all_of_img1 = itk_1->GetLargestPossibleRegion();
    FloatRegionType all_of_img2 = itk_2->GetLargestPossibleRegion();
    FloatRegionType subset_of_img2, sub2set_of_img2;


    FloatIteratorType itk_1_iterator (itk_1, all_of_img1);
    FloatIteratorType gamma_img_iterator (gamma_img, gamma_img->GetLargestPossibleRegion());
    UCharIteratorType mask_img_iterator;
    if (mask_img) {
        mask_img_iterator = UCharIteratorType (mask_img, mask_img->GetLargestPossibleRegion());
    }

    FloatImageType::IndexType k1, k2, k3; //k1: img1_pos (index), k2: img2_pos(moving, index), k3: img2_pos (moving, subregion2 for interp-search)
    FloatImageType::IndexType sub1_start, sub2_start;
    FloatImageType::OffsetType offset, sub2_offset;
    FloatImageType::SizeType region_size, sub2_region_size;
    FloatPoint3DType phys;

    //int reg_pixsize; 
    float level1, level2, level3, dr2, dd2, gg;
    float f0,f1,f2,f3;
    float fixedlevel2;//corresponding position pixel value of level1 to apply threshold
	

    ////interpolated voxel position (float) for interp-search
    //float sub2_interp_index[3];	
    //float dose_diff1, dose_diff2, dose_diff3;
    //float p_factor;//interpolation factor for interp-search
	
    // vox-to-mm-to-gamma conversion factors
    // strictly, these should come from IMAGE2, not 1
    f0 = spacing_in[0]/this->dta_tolerance; f0=f0*f0;
    f1 = spacing_in[1]/this->dta_tolerance; f1=f1*f1;
    f2 = spacing_in[2]/this->dta_tolerance; f2=f2*f2;

    // if this is 2D, f(dim) is forced to be 0 not to contribute to
    // gamma value below:
    if (dim_in[0] == 1)
        f0 = 0.0;
    if (dim_in[1] == 1)
        f1 = 0.0;
    if (dim_in[2] == 1)
        f2 = 0.0;

    // dose_difference_tolerance: e.g.: 0.03
    // hence, dose_tol [Gy]
    // To add local-gamma feature, this should be modified
    // float dose_tol = this->reference_dose * this->dose_difference_tolerance;
    f3 = 1. / this->dose_difference_tolerance;
    f3 = f3*f3;
    
    // compute search region size
    float gmax_dist = this->dta_tolerance * this->gamma_max;
    offset[0] = (int) ceil (gmax_dist /fabs(spacing_in[0]));
    offset[1] = (int) ceil (gmax_dist /fabs(spacing_in[1]));
    offset[2] = (int) ceil (gmax_dist /fabs(spacing_in[2]));	

    // for interp-search
    sub2_offset[0] = 1.0;
    sub2_offset[1] = 1.0;
    sub2_offset[2] = 1.0;

    // for 2D image this might not be necessary if itk image can deal 
    // with boundary condition
    for (int i = 0; i < 3; i++)	{
        if (dim_in[i] == 1)	{
            offset[i] = 0.0;
            sub2_offset[i] = 0.0;
        }
    }

    //analysis_threshold in Gy
    float analysis_threshold_in_Gy = this->analysis_thresh * this->reference_dose;
    //default: if no option of analysis threshold is used: analysis_threshold = 0.0
    lprintf("analysis threshold = %3.2f Gy \n", analysis_threshold_in_Gy);

    gamma_img_iterator.GoToBegin();
    if (mask_img) {
        mask_img_iterator.GoToBegin();
    }
	
    //This value is -1.0 in OmniproIMRT
    float NoProcessGammaValue = 0.0f;

    int idx_histo = 0;
    int num_histo_bin = MAX_NUM_HISTOGRAM_BIN-1; //last slot is reserved for > max_gamma cases	
    plm_long voxels_processed = 0;

    voxels_in_image = pih.get_num_voxels ();
    voxels_in_mask = 0;

    //For interp-search option
    //Convert all the components to scaled ones
    //From now on, scaled gamma coordinate is used.
    float gamma_comp_r1[4]; // 0,1,2,3 --> X, Y, Z, dose with scaling, r1 = ref point
    float gamma_comp_r2[4]; // 0,1,2,3 --> X, Y, Z, dose with scaling r2 = current searching point
    float gamma_comp_r3[4]; //r3: sub2-region moving point		
    float gamma_comp_rn[4]; //rn: normal_vector point (will give the smallest gamma value in gamma space)

    float ref_dose_general;//in local gamma option, this should be a local dose, not a reference dose.

    float sum_up = 0.0;
    float sum_down = 0.0;

    int i = 0;

    for (itk_1_iterator.GoToBegin(); 
         !itk_1_iterator.IsAtEnd(); 
         ++itk_1_iterator)
    {
        voxels_processed++;
        // skip masked out voxels
        // (mask may be interpolated so we use a value of 0.5 for threshold)
        if (mask_img) {
            unsigned char mask_value = mask_img_iterator.Get();
            ++mask_img_iterator;
            // if voxel not inside mask, gamma value is 0.0 (passed)
            if (mask_value == 0) {
                //YK: is 0.0 Safe? how about -1.0?
                gamma_img_iterator.Set (NoProcessGammaValue);
                ++gamma_img_iterator;
                continue;
            }
        }
        voxels_in_mask ++;

        //calculate gamma for this voxel of input image
        level1 = itk_1_iterator.Get();

        if (b_local_gamma)
            ref_dose_general = level1;
        else //global, default
            ref_dose_general = this->reference_dose;	//by default, this is coming from ref dose (max dose if not defined)
                       
        k1 = itk_1_iterator.GetIndex();
        itk_1->TransformIndexToPhysicalPoint(k1, phys);
        itk_2->TransformPhysicalPointToIndex(phys, k2);

        
        if (b_ref_only_threshold) //don't care the corresponding pixel value
            fixedlevel2 = -1.0;            
        else
            fixedlevel2 = itk_2->GetPixel(k2);//just for comparison to corresponding value
            

        //if this option is on, computation will be much faster because dose comparison will not be perforemd.
        if (!this->b_compute_full_region){
            if (this->have_analysis_thresh){
                //if (level1 < analysis_threshold_in_Gy){
                if (level1 < analysis_threshold_in_Gy && fixedlevel2 < analysis_threshold_in_Gy){
                    gamma_img_iterator.Set (NoProcessGammaValue);//YK: is 0.0 Safe? how about -1.0?
                    ++gamma_img_iterator;
                    continue;//skip the rest of the loop. This point will not be counted in analysis
                }			
            }
        }		

        /*k1=itk_1_iterator.GetIndex();
        itk_1->TransformIndexToPhysicalPoint( k1, phys );
        itk_2->TransformPhysicalPointToIndex( phys, k2 );		*/

        //k2 is the voxel index of the k1's physical (mm) position in img2
    
        // set subset_of_img2 to the following region:  
        // k1[0]-region_size < k2[0] < k1[0]+region_size, same for y,z
        // assume (approx) same pix spacing in img1 and img2
        // crop the region by the entire image to be safe		
        
        sub1_start = k2 - offset;
        subset_of_img2.SetIndex(sub1_start);
        region_size[0] = 2 * offset[0] + 1;
        region_size[1] = 2 * offset[1] + 1;
        region_size[2] = 2 * offset[2] + 1; //seems to be fine even in 2D case

        subset_of_img2.SetSize (region_size);
        subset_of_img2.Crop (all_of_img2);

        FloatIteratorType itk_2_iterator (itk_2, subset_of_img2);

        // calculate gamma, take a minimum of ... over the subset_of_img2
        gamma = 1e20;

        for (itk_2_iterator.GoToBegin(); 
             !itk_2_iterator.IsAtEnd(); 
             ++itk_2_iterator)
        {
            //original method: w/o interp-search option

            //k2: moving index from sub1_start
            k2 = itk_2_iterator.GetIndex();
            level2 = itk_2_iterator.Get();
			
            dr2 = (k2[0]-k1[0])*(k2[0]-k1[0])*f0 +
                (k2[1]-k1[1])*(k2[1]-k1[1])*f1 +
                (k2[2]-k1[2])*(k2[2]-k1[2])*f2 ;
            //dd2: (dose diff./D)^2						
            //dd2 = ((level1 - level2) / reference_dose) * ((level1 - level2) / reference_dose) * f3; // (if local gamma is on, {[(d1-d2)/d1]*100 (%) / tol_dose(%)}^2)            								
            dd2 = ((level1 - level2) / ref_dose_general) * ((level1 - level2) / ref_dose_general) * f3; // (if local gamma is on, {[(d1-d2)/d1]*100 (%) / tol_dose(%)}^2)            								

            gg = dr2 + dd2;
            // in this subregion, only minimum value is take.
            if (gg < gamma) gamma=gg;			

            if (!this->b_interp_search)
                continue;

            ///////////////////interp-search begin////////////////
            //if some option is on (e.g. interp-search)
            sub2_start = k2 - sub2_offset;

            //for 3x3x3 region of interp-search (in 3D)
            sub2_region_size[0] = 2 * sub2_offset[0] + 1;
            sub2_region_size[1] = 2 * sub2_offset[1] + 1;
            sub2_region_size[2] = 2 * sub2_offset[2] + 1;

            sub2set_of_img2.SetIndex(sub2_start);
            sub2set_of_img2.SetSize(sub2_region_size);
            sub2set_of_img2.Crop(all_of_img2);

            FloatIteratorType sub2_iterator(itk_2, sub2set_of_img2);

            //scaling to go to the gamma calculation space. all components (x,y,z,d) are now in sampe weighting / space
            gamma_comp_r1[0] = k1[0] * sqrt(f0);
            gamma_comp_r1[1] = k1[1] * sqrt(f1);
            gamma_comp_r1[2] = k1[2] * sqrt(f2);
            gamma_comp_r1[3] = level1 / ref_dose_general * sqrt(f3);

            gamma_comp_r2[0] = k2[0] * sqrt(f0);
            gamma_comp_r2[1] = k2[1] * sqrt(f1);
            gamma_comp_r2[2] = k2[2] * sqrt(f2);
            gamma_comp_r2[3] = level2 / ref_dose_general * sqrt(f3);

            //maximum 3x3x3 iterations
            for (sub2_iterator.GoToBegin();
                 !sub2_iterator.IsAtEnd();
                 ++sub2_iterator)
            {
                k3 = sub2_iterator.GetIndex(); //voxel position (pix)
                level3 = sub2_iterator.Get(); //Dose in Gy

                gamma_comp_r3[0] = k3[0] * sqrt(f0);
                gamma_comp_r3[1] = k3[1] * sqrt(f1);
                gamma_comp_r3[2] = k3[2] * sqrt(f2);
                gamma_comp_r3[3] = level3 / ref_dose_general * sqrt(f3);

                //in 4D space, find the normal vector and its crossection point on the line (r2-r3)
                // vector equation for the line: rx = r2 + t(r3-r2)
                //VECTOR PRODUCT[(rx - r1),(r2-r3)] = 0 

                //if t is determined, then we can get the point for gamma calculation.
                sum_up = 0.0;
                sum_down = 0.0;

                for (i = 0; i < 4; i++)	{
                    sum_up += (gamma_comp_r2[i] - gamma_comp_r3[i])*(gamma_comp_r2[i] - gamma_comp_r1[i]);
                    sum_down += (gamma_comp_r2[i] - gamma_comp_r3[i])*(gamma_comp_r2[i] - gamma_comp_r3[i]);
                }

                float scalar_t = sum_up / sum_down;

                //Only interpolation is allowed. exterapolation should not be used.
                if (scalar_t > 0.0 && scalar_t < 1.0){
                    for (i = 0; i < 4; i++)	{
                        gamma_comp_rn[i] = gamma_comp_r2[i] + scalar_t * (gamma_comp_r3[i] - gamma_comp_r2[i]);
                    }

                    gg = (gamma_comp_r1[0] - gamma_comp_rn[0])*(gamma_comp_r1[0] - gamma_comp_rn[0]) +
                        (gamma_comp_r1[1] - gamma_comp_rn[1])*(gamma_comp_r1[1] - gamma_comp_rn[1]) +
                        (gamma_comp_r1[2] - gamma_comp_rn[2])*(gamma_comp_r1[2] - gamma_comp_rn[2]) +
                        (gamma_comp_r1[3] - gamma_comp_rn[3])*(gamma_comp_r1[3] - gamma_comp_rn[3]);
                    // in this subregion, only minimum value is take.
                    if (gg < gamma) gamma = gg;
                }//only scalar_t is in btw. 0 and 1 (interpolation)
            }// end of sub2_iteration for interp-search
        }
        gamma = sqrt(gamma);

        //gamma_max: e.g. 2.0
        if (gamma > this->gamma_max) {
            gamma = this->gamma_max;
        }
        gamma_img_iterator.Set (gamma);
        ++gamma_img_iterator;

        /*determine gamma histogram bin */		
        idx_histo = floor(gamma / gamma_max * (double)num_histo_bin);
	
        /* Get statistics */
        if (this->have_analysis_thresh) {
            //if (level1 > analysis_threshold_in_Gy) {            
            /* include this voxel */
            /*fixedlevel2 = -1 if include hig dose Comp is off*/
            if (!(level1 < analysis_threshold_in_Gy && fixedlevel2 < analysis_threshold_in_Gy)){
                this->analysis_num_vox ++;
                if (gamma <= 1) {
                    this->analysis_num_pass ++;
                }
                arr_gamma_histo[idx_histo]++;
            }
        }
        //if no analysis threshold was used: every dose point will be counted as analysis point
        else{
            this->analysis_num_vox++;
            if (gamma <= 1) {
                this->analysis_num_pass++;
            }
            arr_gamma_histo[idx_histo]++;
        }

        if (progress_callback && voxels_processed % 1000 == 0) {
            progress_callback (
                (float) voxels_processed / (float) this->voxels_in_image);
        }
    }

    /* One last time, just to be sure */
    if (progress_callback) {
        progress_callback (1.0);
    }

    this->gamma_image->set_itk (gamma_img);	
}

void 
Gamma_dose_comparison_private::do_gamma_threshold ()
{ 
    FloatImageType::Pointer ref_img = img_in1->itk_float();
    FloatImageType::Pointer gamma_img = this->gamma_image->itk_float();

    /* Create labelmap image if not already created */
    if (!labelmap_out) {
        labelmap_out = new Plm_image;
        UCharImageType::Pointer gamma_labelmap = UCharImageType::New();
        itk_image_header_copy (gamma_labelmap, gamma_img);
        gamma_labelmap->Allocate();
        labelmap_out = new Plm_image (gamma_labelmap);
    }
    UCharImageType::Pointer gamma_labelmap = labelmap_out->itk_uchar();

    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > 
        UCharIteratorType;
    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > 
        FloatIteratorType;
    typedef itk::ImageRegion<3> FloatRegionType;
    
    FloatIteratorType ref_it (gamma_img, 
        ref_img->GetLargestPossibleRegion());
    FloatIteratorType gam_it (gamma_img, 
        gamma_img->GetLargestPossibleRegion());
    UCharIteratorType lab_it (gamma_labelmap,
        gamma_labelmap->GetLargestPossibleRegion());

    /* Loop through gamma image, compare against threshold */
    for (ref_it.GoToBegin(), gam_it.GoToBegin(), lab_it.GoToBegin(); 
         !ref_it.IsAtEnd(); 
         ++ref_it, ++gam_it, ++lab_it)
    {
        float ref_dose = ref_it.Get();
        float gamma = gam_it.Get();
        switch (mode) {
        case PASS:
            if ((gamma >=0) && (gamma <= 1) && ref_dose > 0) {
                lab_it.Set (1);
            } else {
                lab_it.Set (0);
            }
            break;
        case FAIL:
            if (gamma > 1) {
                lab_it.Set (1);
            } else {
                lab_it.Set (0);
            }
            break;
        case NONE:
        default:
            lab_it.Set (0);
            break;
        }
    }
}

void 
Gamma_dose_comparison_private::do_mask_threshold ()
{ 
    UCharImageType::Pointer mask_img = img_mask->itk_uchar();

    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > 
        UCharIteratorType;
    UCharIteratorType mask_it (mask_img, 
        mask_img->GetLargestPossibleRegion());

    /* Loop through mask image, set to either 0 or 1 */
    for (mask_it.GoToBegin(); !mask_it.IsAtEnd(); ++mask_it)
    {
        unsigned char mask_val = mask_it.Get();
        mask_it.Set (mask_val > 0 ? 1 : 0);
    }
}

void Gamma_dose_comparison_private::compose_report()
{
    str_gamma_report ="";

    int iStrSize = 128;
    char itemStr[128];
    std::string item;

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%3.2f\n", "pass_rate(%)", analysis_num_pass / (float)analysis_num_vox*100.0);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%d\n", "interp_search", this->b_interp_search);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%d\n", "local_gamma_on", this->b_local_gamma);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%3.2f\n","analysis_threshold", this->analysis_thresh);	
    str_gamma_report= str_gamma_report+std::string(itemStr);
	
    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%d\n","compute_full_region", this->b_compute_full_region);
    str_gamma_report= str_gamma_report+std::string(itemStr);    

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%d\n", "resample-nn", this->b_resample_nn);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%3.2f\n","gamma_max", this->gamma_max);
    str_gamma_report= str_gamma_report+std::string(itemStr);
	
    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%3.2f\n","inherent_resample(mm)", this->f_inherent_resample_mm);
    str_gamma_report= str_gamma_report+std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%d\n", "ref-only-threshold", this->b_ref_only_threshold);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%3.2f\n","reference_dose_Gy", this->reference_dose);
    str_gamma_report= str_gamma_report+std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%3.2f\n","dose_difference_tolerance", this->dose_difference_tolerance);
    str_gamma_report= str_gamma_report+std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%3.2f\n","dta_tolerance", this->dta_tolerance);
    str_gamma_report= str_gamma_report+std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf( itemStr, "%s\t%3.2f\n","dose_max", this->dose_max);
    str_gamma_report= str_gamma_report+std::string(itemStr);
	
    str_gamma_report += string_format ("%s\t%d\n","voxels_in_mask", 
        (int) this->voxels_in_mask);

    str_gamma_report += string_format ("%s\t%d\n","number_of_total_voxels", 
        (int) this->voxels_in_image);

    memset(itemStr, 0, iStrSize);	
    sprintf( itemStr, "%s\t%d\n","number_of_total_voxels", (int) this->voxels_in_image);
    str_gamma_report= str_gamma_report+std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%d\n", "number_of_analysis_voxels", (int) this->analysis_num_vox);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%d\n", "number_of_pass_voxels", (int) this->analysis_num_pass);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "%s\t%3.2f\n", "pass_rate(%)", analysis_num_pass / (float)analysis_num_vox*100.0);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "[BEGIN GAMMA HISTOGRAM]\n");
    str_gamma_report = str_gamma_report + std::string(itemStr);

    //print out histogram
    int iMaxNumBin = MAX_NUM_HISTOGRAM_BIN-1;
    double bin_interval = gamma_max / (double)iMaxNumBin;
    for (int i = 0; i < iMaxNumBin; i++){
        memset(itemStr, 0, iStrSize);
        sprintf(itemStr, "%3.2f-%3.2f\t%d\n", (double)i*bin_interval, (double)(i + 1)*bin_interval, arr_gamma_histo[i]);
        str_gamma_report = str_gamma_report + std::string(itemStr);
    }

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "gamma>=%3.2f\t%d\n", (double)gamma_max, arr_gamma_histo[iMaxNumBin]);
    str_gamma_report = str_gamma_report + std::string(itemStr);

    memset(itemStr, 0, iStrSize);
    sprintf(itemStr, "[END GAMMA HISTOGRAM]\n");
    str_gamma_report = str_gamma_report + std::string(itemStr);
}

std::string
Gamma_dose_comparison::get_report_string()
{
    return d_ptr->str_gamma_report;
}

void
Gamma_dose_comparison::set_report_string(std::string& report_str)
{
    d_ptr->str_gamma_report = report_str;
}

bool
Gamma_dose_comparison::is_local_gamma()
{
    return d_ptr->b_local_gamma;
}

void
Gamma_dose_comparison::set_local_gamma(bool bLocalGamma)
{
    d_ptr->b_local_gamma = bLocalGamma;
}

bool
Gamma_dose_comparison::is_compute_full_region()
{
    return d_ptr->b_compute_full_region;
}

void
Gamma_dose_comparison::set_compute_full_region(bool b_compute_full_region)
{
    d_ptr->b_compute_full_region = b_compute_full_region;
}

float
Gamma_dose_comparison::get_inherent_resample_mm()
{
    return d_ptr->f_inherent_resample_mm;
}

void
Gamma_dose_comparison::set_inherent_resample_mm(float inherent_spacing_mm)
{
    d_ptr->f_inherent_resample_mm = inherent_spacing_mm;
}

bool Gamma_dose_comparison::is_resample_nn()
{
    return d_ptr->b_resample_nn;
}

void Gamma_dose_comparison::set_resample_nn(bool b_resample_nn)
{
    d_ptr->b_resample_nn = b_resample_nn;
}

bool Gamma_dose_comparison::is_interp_search()
{
    return d_ptr->b_interp_search;
}

void Gamma_dose_comparison::set_interp_search(bool b_interp_search)
{
    d_ptr->b_interp_search = b_interp_search;
}

Plm_image* Gamma_dose_comparison::get_ref_image()
{
    return d_ptr->img_in1.get();
}

Plm_image* Gamma_dose_comparison::get_comp_image()
{
    return d_ptr->img_in2.get();
}

bool Gamma_dose_comparison::is_ref_only_threshold()
{
    return d_ptr->b_ref_only_threshold;
}

void Gamma_dose_comparison::set_ref_only_threshold(bool b_ref_only_threshold)
{
    d_ptr->b_ref_only_threshold = b_ref_only_threshold;
}

