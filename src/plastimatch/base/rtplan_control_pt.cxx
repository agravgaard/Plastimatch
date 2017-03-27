/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include "logfile.h"
#include "plm_math.h"
#include "rtplan_control_pt.h"

Rtplan_control_pt::Rtplan_control_pt()
{
    this->cumulative_meterset_weight = -1.f;
    this->nominal_beam_energy = -1.f;
    this->meterset_rate = 0.f;

    this->gantry_angle = 0.f;
    this->gantry_rotation_direction = "NONE";
    this->gantry_pitch_angle = 0.f;
    this->gantry_pitch_rotation_direction = "NONE";
    this->beam_limiting_device_angle = 0.f;
    this->beam_limiting_device_rotation_direction = "NONE";

    this->scan_spot_tune_id = "Big Spots V2.3";
    this->number_of_scan_spot_positions = 0;
    this->scan_spot_reordering_allowed = "ALLOWED";
    this->number_of_paintings = 1;
    this->scanning_spot_size[0] = 1.0f;
    this->scanning_spot_size[1] = 1.0f;

    this->patient_support_angle = 0.f;
    this->patient_support_rotation_direction = "NONE";
    this->table_top_pitch_angle = 0.f;
    this->table_top_pitch_rotation_direction = "NONE";
    this->table_top_roll_angle = 0.f;
    this->table_top_roll_rotation_direction = "NONE";
    this->table_top_vertical_position = 0.f;
    this->table_top_longitudinal_position = 0.f;
    this->table_top_lateral_position = 0.f;
    
    this->isocenter_position[0] = 0.f;
    this->isocenter_position[1] = 0.f;
    this->isocenter_position[2] = 0.f;
}

Rtplan_control_pt::~Rtplan_control_pt()
{

}
