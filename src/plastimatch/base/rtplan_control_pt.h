/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rtplan_control_pt_h_
#define _rtplan_control_pt_h_

#include "plmbase_config.h"
#include <string>
#include <vector>

class PLMBASE_API Rtplan_control_pt {
public:
    float cumulative_meterset_weight;
    float nominal_beam_energy;
    float meterset_rate;

    float gantry_angle;
    std::string gantry_rotation_direction;
    float gantry_pitch_angle;
    std::string gantry_pitch_rotation_direction;
    float beam_limiting_device_angle;
    std::string beam_limiting_device_rotation_direction;

    std::string scan_spot_tune_id;
    size_t number_of_scan_spot_positions;
    std::string scan_spot_reordering_allowed;
    std::vector<float> scan_spot_position_map;
    std::vector<float> scan_spot_meterset_weights;

    size_t number_of_paintings;
    float scanning_spot_size[2];
    
    float patient_support_angle;
    std::string patient_support_rotation_direction;
    
    float table_top_pitch_angle;
    std::string table_top_pitch_rotation_direction;
    float table_top_roll_angle;
    std::string table_top_roll_rotation_direction;
    float table_top_vertical_position;
    float table_top_longitudinal_position;
    float table_top_lateral_position;
    
    float isocenter_position[3];

public:
    Rtplan_control_pt();
    ~Rtplan_control_pt();
    float* get_isocenter () { return isocenter_position; }
    void set_isocenter (float *iso) {
        isocenter_position[0] = iso[0];
        isocenter_position[1] = iso[1];
        isocenter_position[2] = iso[2];
    }
};

#endif
