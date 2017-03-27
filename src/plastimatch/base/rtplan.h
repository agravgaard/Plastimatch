/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _rtplan_h_
#define _rtplan_h_

#include "plmbase_config.h"
#include <list>
#include <vector>

#include "direction_cosines.h"
#include "plm_int.h"
#include "rt_study_metadata.h"
#include "smart_pointer.h"

class Plm_image;
class Plm_image_header;
class Rtplan_beam;

class PLMBASE_API Rtplan {
public:
    SMART_POINTER_SUPPORT(Rtplan);
public:
    size_t number_of_fractions_planned;
    std::string patient_position;
    std::string snout_id;
    std::string general_accessory_id;
    std::string general_accessory_code;
    std::string number_of_range_shifters;
    std::string range_shifter_id;
    std::string range_shifter_number;
    std::string range_shifter_code;
    std::string range_shifter_type;
    std::string range_modulator_id;
    std::string range_modulator_code;
    std::string patient_support_id;    
    std::string patient_support_accessory_code;
    std::string rt_plan_label;
    std::string rt_plan_name;
    std::string rt_plan_date;
    std::string rt_plan_time;
    std::string tolerance_table_label;
    std::string tolerance_gantry_angle;
    std::string tolerance_patient_support_angle;
    std::string tolerance_table_top_vertical;
    std::string tolerance_table_top_longitudinal;
    std::string tolerance_table_top_lateral;
    std::string tolerance_table_top_pitch;
    std::string tolerance_table_top_roll;
    std::string tolerance_snout_position;
    std::vector<Rtplan_beam*> beamlist;
public:
    Rtplan();
    ~Rtplan();
    void init (void);
    void clear (void);
    Rtplan_beam* add_beam (
        const std::string& beam_name,         
	int beam_id);
    void delete_beam (int index);
    Rtplan_beam* find_beam_by_id (size_t index);
    std::string get_beam_name (size_t index);
    void set_beam_name (size_t index, const std::string& name);
    void debug (void);
};

#endif
