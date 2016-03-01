/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdlib.h>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include <itkTranslationTransform.h>
#include "itkLinearInterpolateImageFunction.h"

#include "dir_list.h"
#include "logfile.h"
#include "mabs.h"
#include "mabs_atlas_selection.h"
#include "path_util.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_warp.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "registration.h"
#include "registration_parms.h"
#include "registration_data.h"
#include "rt_study.h"
#include "xform.h"

/* Utility function to compare similarity value in the list of atlases */
bool
compare_similarity_value_from_pairs(
    const std::pair<std::string, double>& first_atlas,
    const std::pair<std::string, double>& second_atlas)
{
    return (first_atlas.second >= second_atlas.second);
} 


Mabs_atlas_selection::Mabs_atlas_selection ()
{
    /* constructor */
    this->subject_id = "";
    this->atlas_selection_criteria = "nmi";
    this->selection_reg_parms_fn = "";
    this->atlas_dir = "";
    this->similarity_percent_threshold = 0.40;
    this->atlases_from_ranking = -1;
    this->number_of_atlases = -1;
    this->hist_bins = 100;
    this->percentage_nmi_random_sample = -1;
    this->min_hist_sub_value_defined = false;
    this->min_hist_sub_value=0;
    this->max_hist_sub_value_defined = false;
    this->max_hist_sub_value=0;
    this->min_hist_atl_value_defined = false;
    this->min_hist_atl_value=0;
    this->max_hist_atl_value_defined = false;
    this->max_hist_atl_value=0;
    this->max_random_atlases = 14;
    this->min_random_atlases = 6;
    this->precomputed_ranking_fn = "";
}


Mabs_atlas_selection::~Mabs_atlas_selection ()
{
    /* destructor */
}


void
Mabs_atlas_selection::run_selection()
{
    if (this->atlas_selection_criteria == "nmi" ||
        this->atlas_selection_criteria == "nmi-post" ||
        this->atlas_selection_criteria == "nmi-ratio" ||
        this->atlas_selection_criteria == "mse" ||
        this->atlas_selection_criteria == "mse-post" ||
        this->atlas_selection_criteria == "mse-ratio")
    {
        this->similarity_ranking();
    }

    else if (this->atlas_selection_criteria == "random")
    {
        this->random_ranking ();
    }

    else if (this->atlas_selection_criteria == "precomputed")
    {
        this->precomputed_ranking ();
    }

}


void
Mabs_atlas_selection::similarity_ranking()
{
    lprintf("SIMILARITY RANKING \n");

    printf ("Number of initial atlases = %d \n", this->number_of_atlases);
    
    /* Loop through images in the atlas and compute similarity value */
    std::list<std::pair<std::string, double> > atlas_and_similarity;
    std::list<std::string>::iterator atl_it;
    int i;
    for (atl_it = this->atlas_dir_list.begin(), i=0;
         atl_it != this->atlas_dir_list.end(); atl_it++, i++)
    {	
        Rt_study* rtds_atl= new Rt_study;
       	std::string path_atl = *atl_it;
        std::string atlas_id = basename (path_atl);
        std::string atlas_input_path = string_format ("%s/prealign/%s", 
            this->atlas_dir.c_str(), atlas_id.c_str());
        std::string fn = string_format ("%s/img.nrrd", atlas_input_path.c_str());
        rtds_atl->load_image (fn.c_str());
        
        this->atlas = rtds_atl->get_image();

        /* Subject compared with atlas, not subject vs itself */
        if (this->subject_id.compare(atlas_id))
        {
            lprintf("Similarity values %s - %s \n", this->subject_id.c_str(), atlas_id.c_str());
            /* Compute similarity value */
            atlas_and_similarity.push_back(
                std::make_pair(basename(atlas_id),
                this->compute_general_similarity_value())); 
        }
       
        delete rtds_atl;
    } 
  
    /* Sort atlases in basis of the similarity value */
    atlas_and_similarity.sort(compare_similarity_value_from_pairs);

    /* Copy ranked atlases (not selected) */
    this->ranked_atlases.assign(atlas_and_similarity.begin(), atlas_and_similarity.end());

    /* Find min and max similarity value */
    double min_similarity_value = atlas_and_similarity.back().second;
    double max_similarity_value = atlas_and_similarity.front().second;
    printf("Minimum similarity value = %g \n", min_similarity_value);
    printf("Maximum similarity value = %g \n", max_similarity_value);

    /* Create items to select the atlases */	
    std::list<std::pair<std::string, double> > most_similar_atlases;
    std::list<std::pair<std::string, double> >::iterator reg_it;
    printf("List of the selected atlases for subject %s: \n", this->subject_id.c_str());
    
    /* Find atlas to include in the registration - THRESHOLD - */
    if (this->atlases_from_ranking == -1) {
        for(reg_it = atlas_and_similarity.begin();
            reg_it != atlas_and_similarity.end(); reg_it++)
        {
            /* Similarity value greater than the threshold, take the atlas */
            if (reg_it->second >=
                (((max_similarity_value-min_similarity_value) * this->similarity_percent_threshold) + min_similarity_value))
    	    {
                most_similar_atlases.push_back(*reg_it);
                printf("Atlas %s having similarity value = %f \n", reg_it->first.c_str(), reg_it->second);
            }
        
            /* The list is sorted, all the next atlases have a similarity value lower than the threshold
             * exit from the cycle
             * */
            else break; 
        }
    }
    
    /* Find atlas to include in the registration - TOP - */
    if (this->atlases_from_ranking != -1) {

        /* Check if atlases_from_ranking is greater than number_of_atlases */
        if (this->atlases_from_ranking >= this->number_of_atlases)
            print_and_exit("Atlases_from_ranking is greater than number of atlases\n");

        reg_it = atlas_and_similarity.begin();
        printf("Atlas to select = %d \n", this->atlases_from_ranking);

        for (int i = 1;
            ((i <= this->atlases_from_ranking) && (i <= (int) atlas_and_similarity.size())); 
            reg_it++, i++)
        {
            printf("Atlas %s (# %d) having similarity value = %f \n", reg_it->first.c_str(), i, reg_it->second);
            most_similar_atlases.push_back(*reg_it);
        }
    }
   
    /* Prepare to exit from function */
    printf("Number of selected atlases = %d \n", (int) most_similar_atlases.size());
    this->selected_atlases = most_similar_atlases; 
}


double
Mabs_atlas_selection::compute_general_similarity_value()
{
    double score = 0;

    /* metric: NMI */
    if (this->atlas_selection_criteria == "nmi") 
    {
        score = this->compute_nmi(this->subject, this->atlas);
        lprintf("NMI value = %g \n", score);
    }

    /* metric: MSE */
    else if (this->atlas_selection_criteria == "mse") 
    {
        score = this->compute_mse(this->subject, this->atlas);
        lprintf("MSE value = %g \n", score);
    }

    /* metrics: NMI POST & MSE POST */
    else if (this->atlas_selection_criteria == "nmi-post" ||
             this->atlas_selection_criteria == "mse-post")
    {
        score = this->compute_similarity_value_post();
    }
    
    /* metrics: NMI RATIO & MSE RATIO */
    else if (this->atlas_selection_criteria == "nmi-ratio" ||
             this->atlas_selection_criteria == "mse-ratio")
    {
        score = this->compute_similarity_value_ratio();
    }

    return score;
}


double
Mabs_atlas_selection::compute_similarity_value_post()
{
    Registration reg;
    Registration_parms::Pointer regp = reg.get_registration_parms ();
    Registration_data::Pointer regd = reg.get_registration_data ();

    reg.set_command_file (this->selection_reg_parms_fn);
    reg.set_fixed_image (this->subject);
    reg.set_moving_image (this->atlas);
    Xform::Pointer xf = reg.do_registration_pure ();

    Plm_image::Pointer deformed_atlas = Plm_image::New ();
    Plm_image_header fixed_pih (regd->fixed_image);
    plm_warp (deformed_atlas, 0, xf, &fixed_pih, 
        this->atlas, regp->default_value, 0, 1);
   
    double similarity_value_post = 0;
    if (this->atlas_selection_criteria == "nmi-post") {
        similarity_value_post = this->compute_nmi (this->subject, deformed_atlas);
        lprintf("NMI post = %g \n", similarity_value_post);
    }
 
    else if (this->atlas_selection_criteria == "mse-post") {
        similarity_value_post = this->compute_mse (this->subject, deformed_atlas);
        lprintf("MSE post = %g \n", similarity_value_post);
    }
   
    return similarity_value_post;
}


double
Mabs_atlas_selection::compute_similarity_value_ratio()
{
    double similarity_value_pre = 0;

    if (this->atlas_selection_criteria == "nmi-ratio")
        similarity_value_pre = this->compute_nmi(this->subject, this->atlas);
    else if (this->atlas_selection_criteria == "mse-ratio")
        similarity_value_pre = this->compute_mse(this->subject, this->atlas);
   
    lprintf("Similarity value pre = %g \n", similarity_value_pre);

    Registration reg;
    Registration_parms::Pointer regp = reg.get_registration_parms ();
    Registration_data::Pointer regd = reg.get_registration_data ();

    reg.set_command_file (this->selection_reg_parms_fn);
    reg.set_fixed_image (this->subject);
    reg.set_moving_image (this->atlas);
    Xform::Pointer xf = reg.do_registration_pure ();

    Plm_image::Pointer deformed_atlas = Plm_image::New ();
    Plm_image_header fixed_pih (regd->fixed_image);
    plm_warp (deformed_atlas, 0, xf, &fixed_pih, 
        this->atlas, regp->default_value, 0, 1);

    double similarity_value_post = 0;

    if (this->atlas_selection_criteria == "nmi-ratio")
        similarity_value_post = this->compute_nmi (this->subject, deformed_atlas);
    else if (this->atlas_selection_criteria == "mse-ratio")
        similarity_value_post = this->compute_mse (this->subject, deformed_atlas);
   
    lprintf("Similarity value post = %g \n", similarity_value_post);

    return ((similarity_value_post/similarity_value_pre)-1) * similarity_value_post;
}


double
Mabs_atlas_selection::compute_nmi (
    const Plm_image::Pointer& img1, /* Subject */ 
    const Plm_image::Pointer& img2) /* Atlas */
{	
    /* Cost function */
    typedef float PixelComponentType;
    const unsigned int Dimension = 3;
    typedef itk::Image< PixelComponentType, Dimension > ImageType;
    typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<ImageType, ImageType > NmiMetricType;
    NmiMetricType::Pointer nmi_metric = NmiMetricType::New();
    
    /* Identity transformation */
    typedef itk::TranslationTransform< double, Dimension > TranslationTransformType;
    typedef TranslationTransformType::Pointer TranslationTransformPointer;
    TranslationTransformPointer transform = TranslationTransformType::New();
    transform->SetIdentity();
	
    /* Linear Interpolator */
    typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    
    /* Set mask if defined */
    if (this->mask)
    {
       	nmi_metric->SetFixedImageMask(this->mask);
    }

    /* Set number of random sample if defined */
    if (this->percentage_nmi_random_sample != -1) { /* User set this value */
        if (this->percentage_nmi_random_sample <= 0 ||
            this->percentage_nmi_random_sample > 1) { /* The set value is not in the range */
            printf("Percentage nmi random sample not set properly. " 
                   "User setting will be ignored and the default value will be used\n");
        }

        else { /* The set value is in the range, apply it */
            unsigned long number_of_img_voxels =
                img1->itk_short()->GetLargestPossibleRegion().GetNumberOfPixels();
            unsigned long number_spatial_samples =
                static_cast<unsigned long> (number_of_img_voxels * this->percentage_nmi_random_sample);
#if defined (ITK_USE_OPTIMIZED_REGISTRATION_METHODS) || (ITK_VERSION_MAJOR >= 4)
            nmi_metric->SetNumberOfSpatialSamples(number_spatial_samples);
#endif
        }
    }
    
    /* Set histogram interval if defined */
    if (this->min_hist_sub_value_defined &&
        this->min_hist_atl_value_defined)
    {
        NmiMetricType::MeasurementVectorType lower_bounds;
        #if ITK_VERSION_MAJOR >= 4 
        lower_bounds.SetSize(2);
        #endif
        lower_bounds.SetElement(0, this->min_hist_sub_value);
        lower_bounds.SetElement(1, this->min_hist_atl_value);
        nmi_metric->SetLowerBound(lower_bounds);
    }

    if (this->max_hist_sub_value_defined &&
        this->max_hist_atl_value_defined)
    {
        NmiMetricType::MeasurementVectorType upper_bounds;
        #if ITK_VERSION_MAJOR >= 4
        upper_bounds.SetSize(2);
        #endif
        upper_bounds.SetElement(0, this->max_hist_sub_value);
        upper_bounds.SetElement(1, this->max_hist_atl_value);
        nmi_metric->SetUpperBound(upper_bounds);
    }
    
    /* Metric settings */
    unsigned int numberOfHistogramBins = this->hist_bins;
    NmiMetricType::HistogramType::SizeType histogramSize;
    #if ITK_VERSION_MAJOR >= 4
    histogramSize.SetSize(2);
    #endif
    histogramSize.Fill(numberOfHistogramBins);
    nmi_metric->SetHistogramSize(histogramSize);
    
    nmi_metric->SetFixedImage(img1->itk_float());
    nmi_metric->SetMovingImage(img2->itk_float());
    nmi_metric->SetFixedImageRegion(img1->itk_float()->GetLargestPossibleRegion());
    
    nmi_metric->SetTransform(transform);
    nmi_metric->SetInterpolator(interpolator);
    
    nmi_metric->Initialize();

    /* NMI compute */
    return (double) nmi_metric->GetValue(transform->GetParameters());
}


double
Mabs_atlas_selection::compute_mse (
    const Plm_image::Pointer& img1, /* Subject */ 
    const Plm_image::Pointer& img2) /* Atlas */
{	
    /* Cost function */
    typedef float PixelComponentType;
    const unsigned int Dimension = 3;
    typedef itk::Image< PixelComponentType, Dimension > ImageType;
    typedef itk::MeanSquaresImageToImageMetric<ImageType, ImageType > MseMetricType;
    MseMetricType::Pointer mse_metric = MseMetricType::New();
    
    /* Identity transformation */
    typedef itk::TranslationTransform< double, Dimension > TranslationTransformType;
    typedef TranslationTransformType::Pointer TranslationTransformPointer;
    TranslationTransformPointer transform = TranslationTransformType::New();
    transform->SetIdentity();
	
    /* Linear Interpolator */
    typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    
    /* Set mask if defined */
    if (this->mask)
    {
       	mse_metric->SetFixedImageMask(this->mask);	
    }
    
    /* Metric settings */
    mse_metric->SetFixedImage(img1->itk_float());
    mse_metric->SetMovingImage(img2->itk_float());
    mse_metric->SetFixedImageRegion(img1->itk_float()->GetLargestPossibleRegion());
    
    mse_metric->SetTransform(transform);
    mse_metric->SetInterpolator(interpolator);
    
    mse_metric->Initialize();
    
    /* MSE compute */
    return (double) mse_metric->GetValue(transform->GetParameters());
}


void
Mabs_atlas_selection::random_ranking()
{
    lprintf("RANDOM RANKING \n");
    
    std::list<std::string> random_atlases;

    /* Check if atlases_from_ranking is greater than number_of_atlases */
    if (this->min_random_atlases < 1 || (this->max_random_atlases + 1) > this->number_of_atlases)
        print_and_exit("Bounds for random selection are not correct\n");

    int random_number_of_atlases = rand() %
        (this->max_random_atlases + 1 - this->min_random_atlases) + this->min_random_atlases;
    printf("Selected %d random atlases for the subject %s \n", random_number_of_atlases, this->subject_id.c_str());

    /* Select the random atlases */
    int i=0;
    while ((int) random_atlases.size() < random_number_of_atlases) {

        /* Choose a random atlas */
        int random_index = rand() % (this->number_of_atlases);
        std::list<std::string>::iterator atlases_iterator = this->atlas_dir_list.begin();
        std::advance (atlases_iterator, random_index);

        /* Check if the random atlas has been already included and if it is different from the subject */
        if (find(random_atlases.begin(), random_atlases.end(), basename(*atlases_iterator)) == random_atlases.end() &&
            basename(*atlases_iterator) != this->subject_id)
        {
            i++;
            std::string atlas = basename(*atlases_iterator);
            printf("Atlas number %d is %s \n", i, atlas.c_str());
            random_atlases.push_back(atlas);
        } 
    }
    
    /* Fill this->selected_atlas */
    std::list<std::string>::iterator atl_it;
    for (atl_it = random_atlases.begin(); 
        atl_it != random_atlases.end(); atl_it++)
    {
        this->selected_atlases.push_back(std::make_pair(*atl_it, 0.0));
    }
 
}


void
Mabs_atlas_selection::precomputed_ranking()
{
    /* Check if atlases_from_ranking is greater than number_of_atlases */
    if (this->atlases_from_ranking >= this->number_of_atlases)
        print_and_exit("Atlases_from_ranking is greater than number of atlases\n");

    /* Open the file where the precomputed ranking is stored */
    std::ifstream ranking_file (this->precomputed_ranking_fn.c_str());
    
    std::string atlases_line;

    printf("Atlases to select = %d \n", this->atlases_from_ranking);

    /* Read line by line*/
    while(std::getline(ranking_file, atlases_line))
    {
        
        std::istringstream atlases_line_stream (atlases_line);
        std::string item;
        
        /* item_index is equal to 0 for the first element in the line (the subject id) and
         * greater than 0 for the atlases */
        int item_index = -1;

        /* Split a line using the spaces */
        while(std::getline(atlases_line_stream, item, ' '))
        {

            /* If defined select the first n atlases from the ranking */
            if (this->atlases_from_ranking != -1 &&
                (int) this->selected_atlases.size() >=
                this->atlases_from_ranking)
            {
                break;
            }
            
            item_index++;

            /* Delete space, colon and equal signs from single item name */
            char chars_to_remove [] = " :=";
            for (unsigned int i = 0; i < strlen(chars_to_remove); i++)
            {
                item.erase(std::remove(item.begin(), item.end(), chars_to_remove[i]), item.end());
            }

            /* First element in the line is the subject */
            if (item_index == 0) {
                
                /* This row doesn't contain data regarding the current subject, jump it */
                if (item != this->subject_id) break; 
                 
                /* This row contains the data regarding the current subject,
                 * jump just the subject id */
                else if (item == this->subject_id) { 
                    printf("Subject = %s \n", item.c_str());
                    continue;
                }
            }
            
            /* After the first element there are the selected atlases */
            else
            {
                this->selected_atlases.push_back(std::make_pair(item, 0.0));
                printf("  Selected atlas %s \n", item.c_str());
            }
        }
    }
    
    /* Close log file */    
    ranking_file.close();
}
