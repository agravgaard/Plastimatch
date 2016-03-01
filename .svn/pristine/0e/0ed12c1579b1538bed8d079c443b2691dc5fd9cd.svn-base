/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _mabs_atlas_selection_h_
#define _mabs_atlas_selection_h_

#include "plmsegment_config.h"
#include <algorithm>
#include <list>
#include <stdio.h>
#include "itkImageMaskSpatialObject.h"

#include "plm_image.h"

typedef itk::ImageMaskSpatialObject<3> MaskType;
typedef MaskType::Pointer MaskTypePointer;

class PLMSEGMENT_API Mabs_atlas_selection {

public:
    Mabs_atlas_selection();
    ~Mabs_atlas_selection();
    void run_selection();
    void similarity_ranking();
    double compute_general_similarity_value();
    double compute_similarity_value_ratio();
    double compute_similarity_value_post();
    double compute_nmi (
        const Plm_image::Pointer& img1, 
        const Plm_image::Pointer& img2);
    double compute_mse (
        const Plm_image::Pointer& img1, 
        const Plm_image::Pointer& img2);
    void random_ranking();
    void precomputed_ranking(); 

public:
    Plm_image::Pointer subject;
    std::string subject_id;
    std::list<std::string> atlas_dir_list;
    std::string atlas_selection_criteria;
    std::string selection_reg_parms_fn;
    std::string atlas_dir;
    float similarity_percent_threshold;
    int atlases_from_ranking; // -1 if this parameter is not defined
    int number_of_atlases;
    Plm_image::Pointer atlas;
    int hist_bins;
    float percentage_nmi_random_sample; // -1 if this parameter is not defined
    MaskTypePointer mask;
    bool min_hist_sub_value_defined;
    int min_hist_sub_value;
    bool max_hist_sub_value_defined;
    int max_hist_sub_value;
    bool min_hist_atl_value_defined;
    int min_hist_atl_value;
    bool max_hist_atl_value_defined;
    int max_hist_atl_value;
    int max_random_atlases;
    int min_random_atlases;
    std::string precomputed_ranking_fn;
    std::list<std::pair<std::string, double> > ranked_atlases; // all the atlases, only ranked
    std::list<std::pair<std::string, double> > selected_atlases; // selected_atlases, subset of ranked_atlases

};

#endif /* #ifndef _mabs_atlases_selection_h_ */
