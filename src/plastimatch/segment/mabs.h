/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _mabs_h_
#define _mabs_h_

#include "plmsegment_config.h"
#include <string>
#include "itk_image.h"

class Mabs_private;
class Mabs_parms;
class Mabs_seg_weights;
class Mabs_seg_weights_list;

class PLMSEGMENT_API Mabs {
public:
    Mabs ();
    ~Mabs ();
public:
    Mabs_private *d_ptr;

protected:
    bool check_seg_checkpoint (std::string folder);
    void load_process_dir_list (const std::string& dir);
    void convert (const std::string& input_dir, const std::string& output_dir);
    void prealign (const std::string& input_dir, 
        const std::string& output_dir);
    FloatImageType::Pointer compute_dmap (
        UCharImageType::Pointer& structure_image,
        const std::string& curr_output_dir,
        const std::string& mapped_name);
    void run_registration_loop ();
    void run_single_registration ();
    void run_segmentation (const Mabs_seg_weights_list& seg_weights);
    void run_segmentation_train (const Mabs_seg_weights& seg_weights);
    void run_segmentation_train_loop ();
    
    void no_voting (
        const std::string& atlas_id,
        const std::string& label_output_dir);

    void gaussian_segmentation_vote (
        const std::string& atlas_id,
        const Mabs_seg_weights_list& seg_weights
    );
    void gaussian_segmentation_label (
        const std::string& label_output_dir, 
        const Mabs_seg_weights_list& seg_weights
    );
    void staple_segmentation_prepare (
        const std::string& atlas_id,
        const Mabs_seg_weights_list& seg_weights
    );
    void staple_segmentation_label (
        const std::string& label_output_dir, 
        const Mabs_seg_weights_list& seg_weights
    );
    void train_internal ();

public:
    void set_parms (const Mabs_parms *parms);
    void parse_registration_dir (const std::string& registration_config);
    
    void set_executed_command (const std::string& executed_command);

    void set_segment_input (const std::string& input_fn);
    void set_segment_output (const std::string& output_dir);
    void set_segment_output_dicom (const std::string& output_dicom_dir);
    void set_prealign_roi_cmd_name (const std::string& input_roi_fn);

    void atlas_selection ();
    void train_atlas_selection ();
    void atlas_convert ();
    void atlas_prealign ();
    void train_registration ();
    void train ();
    void segment ();
};

#endif
