/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <limits>
#include <set>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "plm_image_header.h"
#include "plm_int.h"
#include "plm_math.h"
#include "rtplan_beam.h"
#include "rtplan.h"
#include "rt_study_metadata.h"
#include "string_util.h"

Rtplan::Rtplan()
{
    this->number_of_fractions_planned = 0;
}

Rtplan::~Rtplan()
{
    this->clear ();
}

void
Rtplan::init(void)
{
    this->clear ();
}

void
Rtplan::clear(void)
{
    this->number_of_fractions_planned = 0;
    this->patient_position = "HFS";
    this->snout_id = "";
    this->general_accessory_id = "";
    this->general_accessory_code = "";
    this->range_shifter_id = "";
    this->range_shifter_code = "";
    this->range_modulator_id = "";
    this->range_modulator_code = "";
    this->rt_plan_label = "";
    this->rt_plan_name = "";
    this->rt_plan_date = "";
    this->rt_plan_time = "";
    this->tolerance_table_label = "";
    this->tolerance_gantry_angle = "";
    this->tolerance_patient_support_angle = "";
    this->tolerance_table_top_vertical = "";
    this->tolerance_table_top_longitudinal = "";
    this->tolerance_table_top_lateral = "";
    this->tolerance_table_top_pitch = "";
    this->tolerance_table_top_roll = "";
    this->tolerance_snout_position = "";
    for (size_t i = 0; i < this->beamlist.size(); i++) {
        delete this->beamlist[i];
    }
    this->beamlist.clear ();
}

/* Add structure (if it doesn't already exist) */
Rtplan_beam*
Rtplan::add_beam (
    const std::string& beam_name,     
    int beam_id)
{
    Rtplan_beam* new_beam;

    new_beam = this->find_beam_by_id(beam_id);
    if (new_beam) {
        return new_beam;
    }

    new_beam = new Rtplan_beam;
    new_beam->name = beam_name;
    if (beam_name == "") {
        new_beam->name = "Unknown beam";
    }
    new_beam->name = string_trim (new_beam->name);

    this->beamlist.push_back (new_beam);
    return new_beam;
}

void
Rtplan::delete_beam(int index)
{
    delete this->beamlist[index];
    this->beamlist.erase (this->beamlist.begin() + index);
}

Rtplan_beam*
Rtplan::find_beam_by_id (size_t index)
{
    if (index < this->beamlist.size()) {
        return this->beamlist[index];
    }
    return 0;
}

void 
Rtplan::set_beam_name (size_t index, const std::string& name)
{
    if (index < this->beamlist.size()) {
        this->beamlist[index]->name = name;
    }
}

std::string
Rtplan::get_beam_name(size_t index)
{
    if (index < this->beamlist.size()) {
        return this->beamlist[index]->name;
    } else {
        return "";
    }
}

void
Rtplan::debug(void)
{   
}
