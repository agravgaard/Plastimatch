/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rtplan_beam_h_
#define _rtplan_beam_h_

#include "plmbase_config.h"
#include <string>

class Rtplan_control_pt;

/*! \brief 
 * The Rtplan_beam class describes a single beam within an Rtplan.
 */
class PLMBASE_API Rtplan_beam {
public:
    /*! \brief Beam name */
    std::string name;    
    /*! \brief Beam description */
    std::string description;
    /*! \other brief descriptions */
    std::string treatment_machine_name;
    std::string manufacturer;
    std::string institution_name;
    std::string institution_address;
    std::string institutional_department_name;
    std::string manufacturer_model_name;
    std::string virtual_source_axis_distances;

    /*! \brief Meterset at end of all control points */
    float final_cumulative_meterset_weight;
    /*! \brief Coordiates of point where beam dose is specified */
    std::string beam_dose_specification_point;
    /*! \brief Dose in Gy at beam specification point */
    float beam_dose;
    float snout_position;
    float gantry_angle;
    std::string gantry_rotation_direction;
    float beam_limiting_device_angle;
    std::string beam_limiting_device_rotation_direction;
    float patient_support_angle;
    std::string patient_support_rotation_direction;
    float table_top_vertical_position;
    float table_top_longitudinal_position;
    float table_top_lateral_position;
    float table_top_pitch_angle;
    std::string table_top_pitch_rotation_direction;
    float table_top_roll_angle;
    std::string table_top_roll_rotation_direction;
    float gantry_pitch_angle;
    std::string gantry_pitch_rotation_direction;
    float isocenter_position[3];

    /*! \brief Control point list */
    std::vector<Rtplan_control_pt*> cplist;


public:
    Rtplan_beam();
    ~Rtplan_beam();

    void clear ();
    Rtplan_control_pt* add_control_pt ();
    bool check_isocenter_identical ();
#if defined (commentout)
    float* get_isocenter_pos (); //float[3], dicom coordinate
#endif
};


#endif
