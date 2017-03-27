/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsegment_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "file_util.h"
#include "parameter_parser.h"
#include "mabs_parms.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "string_util.h"

class Mabs_parms_parser : public Parameter_parser
{
public:
    Mabs_parms *mp;
    Mabs_seg_weights ors;
public:
    Mabs_parms_parser (Mabs_parms *mp)
    {
        this->mp = mp;
    }
public:
    virtual Plm_return_code begin_section (
        const std::string& section)
    {
        if (section == "CONVERT") {
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }
        if (section == "PREALIGN" || section == "PREALIGNMENT") {
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }
        if (section == "ATLAS-SELECTION") {
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }
        if (section == "TRAINING") {
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }
        if (section == "REGISTRATION") {
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }
        if (section == "STRUCTURES") {
            this->enable_key_regularization (false);
            return PLM_SUCCESS;
        }
        if (section == "LABELING") {
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }
        if (section == "OPTIMIZATION-RESULT-REG") {
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }
        if (section == "OPTIMIZATION-RESULT-SEG") {
            ors.factory_reset ();
            this->enable_key_regularization (true);
            return PLM_SUCCESS;
        }

        /* else, unknown section */
        return PLM_ERROR;
    }
    virtual Plm_return_code end_section (
        const std::string& section)
    {
        if (section == "OPTIMIZATION-RESULT-SEG") {
            this->mp->optimization_result_seg.push_back (ors);
        }
        return PLM_SUCCESS;
    }
    virtual Plm_return_code set_key_value (
        const std::string& section,
        const std::string& key, 
        //const std::string& index, 
        const std::string& val);
};

Plm_return_code
Mabs_parms_parser::set_key_value (
    const std::string& section,
    const std::string& key, 
    //const std::string& index, 
    const std::string& val)
{
    /* [CONVERT] */
    if (section == "CONVERT") {
        if (key == "spacing") {
            mp->convert_spacing = val;
        }
        else {
            goto error_exit;
        }
    }
    /* [PREALIGNMENT] */
    if (section == "PREALIGN" || section == "PREALIGNMENT") {
        if (key == "mode") {
            if (val == "DISABLED" || val == "disabled" 
                || val == "Disabled" || val == "0")
            {
                mp->prealign_mode = "disabled";
            }
            else if (val == "DEFAULT" || val == "default" || val == "Default") {
                mp->prealign_mode = "default";
            }
            else if (val == "CUSTOM" || val == "custom" || val == "Custom") {
                mp->prealign_mode = "custom";
            }
        }
        else if (key == "reference") {
            mp->prealign_reference = val;
        }
        else if (key == "spacing") {
            mp->prealign_spacing = val;
        }
        else if (key == "registration_config") {
            mp->prealign_registration_config = val;
        }
        else if (key == "prealign_struct") {
            mp->prealign_roi_cfg_name = val;
        }
        else {
            goto error_exit;
        }
    }

    /* [ATLAS-SELECTION] */
    if (section == "ATLAS-SELECTION") {
        if (key == "enable_atlas_selection") {
            if (val == "True" || val == "true" || val == "1") {
                mp->enable_atlas_selection = true;
            }
            else {
                mp->enable_atlas_selection = false;
            }   
        }
        else if (key == "atlas_selection_criteria") {
            if (val == "nmi" || val == "NMI") {
                mp->atlas_selection_criteria="nmi";
            }
            else if (val == "nmi-post" || val == "NMI-POST") {
                mp->atlas_selection_criteria="nmi-post";
            }
            else if (val == "nmi-ratio" || val == "NMI-RATIO") {
                mp->atlas_selection_criteria="nmi-ratio";
            }
            else if (val == "mse" || val == "MSE") {
                mp->atlas_selection_criteria="mse";
            }
            else if (val == "mse-post" || val == "MSE-POST") {
                mp->atlas_selection_criteria="mse-post";
            }
            else if (val == "mse-ratio" || val == "MSE-RATIO") {
                mp->atlas_selection_criteria="mse-ratio";
            }
            else if (val == "random" || val == "RANDOM") {
                mp->atlas_selection_criteria="random";
            }
            else if (val == "precomputed" || val == "PRECOMPUTED") {
                mp->atlas_selection_criteria="precomputed";
            }
        }
        else if (key == "similarity_percent_threshold") {
            sscanf (val.c_str(), "%g", &mp->similarity_percent_threshold);
        }
        else if (key == "atlases_from_ranking") {
            sscanf (val.c_str(), "%d", &mp->atlases_from_ranking);
        }
        else if (key == "mi_histogram_bins") {
            sscanf (val.c_str(), "%d", &mp->mi_histogram_bins);
        }
        else if (key == "percentage_nmi_random_sample") {
            sscanf (val.c_str(), "%g", &mp->percentage_nmi_random_sample);
        }
        else if (key == "roi_mask_fn" || key == "roi_mask") {
            mp->roi_mask_fn = val;
        }
        else if (key == "selection_reg_parms") {
            mp->selection_reg_parms_fn = val;
        }
        else if (key == "lower_mi_value_subject") {
            sscanf (val.c_str(), "%d", &mp->lower_mi_value_sub);
            mp->lower_mi_value_sub_defined = true;
        }
        else if (key == "upper_mi_value_subject") {
            sscanf (val.c_str(), "%d", &mp->upper_mi_value_sub);
            mp->upper_mi_value_sub_defined = true;
        }
        else if (key == "lower_mi_value_atlas") {
            sscanf (val.c_str(), "%d", &mp->lower_mi_value_atl);
            mp->lower_mi_value_atl_defined = true;
        }
        else if (key == "upper_mi_value_atlas") {
            sscanf (val.c_str(), "%d", &mp->upper_mi_value_atl);
            mp->upper_mi_value_atl_defined = true;
        }
        else if (key == "min_random_atlases") {
            sscanf (val.c_str(), "%d", &mp->min_random_atlases);
        }
        else if (key == "max_random_atlases") {
            sscanf (val.c_str(), "%d", &mp->max_random_atlases);
        }
        else if (key == "precomputed_ranking") {
            mp->precomputed_ranking_fn = val;
        }
        else {
            goto error_exit;
        }
    }
        	
    /* [TRAINING] */
    if (section == "TRAINING") {
        if (key == "atlas_dir") {
            mp->atlas_dir = val;
        }
        else if (key == "training_dir") {
            mp->training_dir = val;
        }
        else if (key == "convert_dir") {
            mp->convert_dir = val;
        }
        else if (key == "prealign_dir") {
            mp->prealign_dir = val;
        }
        else if (key == "fusion_criteria") {
            if (val == "gaussian" || val == "GAUSSIAN" || val == "Gaussian") {
                mp->fusion_criteria = "gaussian";
            }
            else if (val == "staple" || val == "STAPLE" || val == "Staple") {
                mp->fusion_criteria = "staple";
            }
            else if (val == "gaussian,staple" || val == "GAUSSIAN,STAPLE" || val == "Gaussian,Staple" ||
                     val == "staple,gaussian" || val == "STAPLE,GAUSSIAN" || val == "Staple,Gaussian") {
                mp->fusion_criteria = "gaussian_and_staple";
            }
            else if (val == "none" || val == "NONE" || val == "None") {
                mp->fusion_criteria = "none";
            }
        }
        else if (key == "distance_map_algorithm") {
            mp->distance_map_algorithm = val;
        }
        else if (key == "minimum_similarity") {
            mp->minsim_values = val;
        }
        else if (key == "rho_values") {
            mp->rho_values = val;
        }
        else if (key == "sigma_values") {
            mp->sigma_values = val;
        }
        else if (key == "threshold_values") {
            mp->threshold_values = val;
        }
        else if (key == "confidence_weight") {
            mp->confidence_weight = val;
        }
        else if (key == "write_distance_map_files") {
            mp->write_distance_map_files = string_value_true (val);
        }
        else if (key == "write_thresholded_files") {
            mp->write_thresholded_files = string_value_true (val);
        }
        else if (key == "write_weight_files") {
            mp->write_weight_files = string_value_true (val);
        }
        else if (key == "write_warped_images") {
            mp->write_warped_images = string_value_true (val);
        }
        else if (key == "write_warped_structures") {
            mp->write_warped_structures = string_value_true (val);
        }
        else {
            goto error_exit;
        }
    }

    /* [REGISTRATION] */
    if (section == "REGISTRATION") {
        if (key == "registration_config") {
            mp->registration_config = val;
        }
        else {
            goto error_exit;
        }
    }

    /* [STRUCTURES] */
    if (section == "STRUCTURES") {
        /* Add key to list of structures */
        mp->structure_map[key] = key;
        if (val != "") {
            /* Key/value pair, so add for renaming */
            mp->structure_map[val] = key;
        }
        /* Add key to the set */
        mp->structure_set.insert (key);
        /* There is no filtering of structures values */
    }

    /* [LABELING] */
    if (section == "LABELING") {
        if (key == "input") {
            mp->labeling_input_fn = val;
        }
        else if (key == "output") {
            mp->labeling_output_fn = val;
        }
        else {
            goto error_exit;
        }
    }

    /* [OPTIMIZATION-RESULT-REG] */
    if (section == "OPTIMIZATION-RESULT-REG") {
        if (key == "registration") {
            mp->optimization_result_reg = val;
        }
        else {
            goto error_exit;
        }
    }

    /* [OPTIMIZATION-RESULT-SEG] */
    if (section == "OPTIMIZATION-RESULT-SEG") {

        if (key == "structure") {
            ors.structure = val;
        }
        else if (key == "gaussian_weighting_voting_rho" || key == "rho") {
            sscanf (val.c_str(), "%g", &ors.rho);
        }
        else if (key == "gaussian_weighting_voting_sigma" || key == "sigma") {
            sscanf (val.c_str(), "%g", &ors.sigma);
        }
        else if (key == "gaussian_weighting_voting_minsim"
            || key == "minsim")
        {
            sscanf (val.c_str(), "%g", &ors.minsim);
        }
        else if (key == "gaussian_weighting_voting_thresh"
            || key == "thresh")
        {
            ors.thresh = val;
        }
        else if (key == "optimization_result_confidence_weight"
            || key == "confidence_weight")
        {
            sscanf (val.c_str(), "%g", &ors.confidence_weight);
        }
        else {
            goto error_exit;
        }
    }

    return PLM_SUCCESS;

error_exit:
    print_and_exit ("Unknown (sec,key,val) combination: (%s,%s,%s)\n", 
        section.c_str(), key.c_str(), val.c_str());
    return PLM_ERROR;
}

Mabs_parms::Mabs_parms ()
{
    /* [CONVERT] */
    this->convert_spacing = "";

    /* [PREALIGNMENT] */
    this->prealign_mode="disabled";
    this->prealign_reference = "";
    this->prealign_spacing = "";
    this->prealign_registration_config = "";
    this->prealign_roi_cfg_name = "";

    /* [ATLAS-SELECTION] */
    this->enable_atlas_selection = false;
    this->atlas_selection_criteria="nmi";
    this->similarity_percent_threshold = 0.40;
    this->atlases_from_ranking = -1;
    this->mi_histogram_bins = 100;
    this->percentage_nmi_random_sample = -1;
    this->roi_mask_fn = "";
    this->selection_reg_parms_fn = "";
    this->lower_mi_value_sub_defined=false;
    this->lower_mi_value_sub = 0;
    this->upper_mi_value_sub_defined=false;
    this->upper_mi_value_sub = 0;
    this->lower_mi_value_atl_defined=false;
    this->lower_mi_value_atl = 0;
    this->upper_mi_value_atl_defined=false;
    this->upper_mi_value_atl = 0;
    this->max_random_atlases = 14;
    this->min_random_atlases = 6;
    this->precomputed_ranking_fn = "";
	
    /* [TRAINING] */
    this->distance_map_algorithm = "";

    this->fusion_criteria = "gaussian";

    this->minsim_values = "L 0.0001:1:0.0001";
    this->rho_values = "1:1:1";
    this->sigma_values = "L 1.7:1:1.7";
    this->threshold_values = "0.5";

    this->confidence_weight = "1:1:1";

    this->write_distance_map_files = true;
    this->write_thresholded_files = true;
    this->write_weight_files = true;
    this->write_warped_images = true;
    this->write_warped_structures = true;

    /* [OPTIMIZATION-RESULT-REG] */
    this->optimization_result_reg = "";

    /* [OPTIMIZATION-RESULT-SEG] */

    /* misc */
    this->debug = false;
}

Mabs_parms::~Mabs_parms ()
{
}

static void
print_usage ()
{
    printf (
        "Usage: mabs [options] config_file\n"
        "Options:\n"
        " --debug           Enable various debug output\n"
    );
    exit (1);
}

void
Mabs_parms::parse_config (
    const char* config_fn
)
{
    Mabs_parms_parser mpp (this);

    /* Parse the main config file */
    mpp.parse_config_file (config_fn);

    /* After parsing main config file, also parse 
       optimization result files */

    std::string reg_fn = string_format (
        "%s/mabs-train/optimization_result_reg.txt",
        this->training_dir.c_str());
    std::string seg_fn = string_format (
        "%s/mabs-train/optimization_result_seg.txt",
        this->training_dir.c_str());
    if (file_exists (reg_fn)) {
        mpp.parse_config_file (reg_fn.c_str());
    }
    if (file_exists (seg_fn)) {
        mpp.parse_config_file (seg_fn.c_str());
    }
}

bool
Mabs_parms::parse_args (int argc, char** argv)
{
    int i;
    for (i=1; i<argc; i++) {
        if (argv[i][0] != '-') break;

        if (!strcmp (argv[i], "--debug")) {
            this->debug = 1;
        }
        else {
            print_usage ();
            break;
        }
    }

    if (!argv[i]) {
        print_usage ();
    } else {
        this->parse_config (argv[i]);
    }

    return true;
}
