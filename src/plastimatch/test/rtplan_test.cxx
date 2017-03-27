/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler_warnings.h"
#include "dcmtk_rt_study.h"
#include "rtplan.h"
#include "rtplan_beam.h"
#include "rtplan_control_pt.h"
#include "string_util.h"

int
main (int argc, char *argv[])
{
    char *dicom_dir;
    UNUSED_VARIABLE (dicom_dir);
    if (argc == 2) {
	dicom_dir = argv[1];
    } else {
	printf ("Usage: rtplan_test output_dir\n");
	exit (1);
    }

    Dcmtk_rt_study drs;
    Rt_study_metadata::Pointer rsm = Rt_study_metadata::New ();
    Rtplan::Pointer rtplan = Rtplan::New ();

    drs.set_rt_study_metadata (rsm);
    drs.set_rtplan (rtplan);

    /* Fill in metadata */
    rsm->set_patient_name ("Test^Rtplan");

    /* Fill in plan data */
    rtplan->number_of_fractions_planned = 5; 
    rtplan->snout_id = "Standard snout";
    rtplan->general_accessory_id = "General accessory A";
    rtplan->general_accessory_code = "1";
    rtplan->range_shifter_id = "Range shifter A";
    rtplan->range_shifter_code = "3";
    rtplan->range_modulator_id = "Range modulator A";
    rtplan->range_modulator_code = "6";

    rtplan->tolerance_table_label = "Standard";
    rtplan->tolerance_gantry_angle = "0.3";
    rtplan->tolerance_patient_support_angle = "0.3";
    rtplan->tolerance_table_top_vertical = "1.0";
    rtplan->tolerance_table_top_longitudinal = "1.0";
    rtplan->tolerance_table_top_lateral = "1.0";
    rtplan->tolerance_snout_position = "0.3";

    /* Fill in beam data */
    for (size_t i = 0; i < 1; i++) {
        std::string beam_name = string_format ("Beam %d", (int) i);
        Rtplan_beam *beam = rtplan->add_beam (beam_name, (int) i);
        beam->description = string_format ("Beam %d description", (int) i);
        beam->beam_dose_specification_point = "0\\-10.5\\0";
        beam->beam_dose = 2.f;

	float snout_pos = 250.f;
	beam->snout_position = snout_pos;
    
        float cum_gp = 0.f;
        for (size_t seg = 0; seg < 3; seg++) {
            float energy = 100.f + seg * 25.f;
            float gp = 25.f;
            for (size_t cpi = 0; cpi < 2; cpi++) {
                Rtplan_control_pt *cp = beam->add_control_pt ();
                cp->cumulative_meterset_weight = cum_gp + cpi * gp;
                cp->nominal_beam_energy = energy;
                cp->meterset_rate = 400;

                cp->scan_spot_position_map.push_back (-10.f - seg);
                cp->scan_spot_position_map.push_back (-10.f);
                cp->scan_spot_position_map.push_back (+10.f + seg);
                cp->scan_spot_position_map.push_back (-10.f);
                cp->scan_spot_position_map.push_back (-10.f - seg);
                cp->scan_spot_position_map.push_back (+10.f);
                cp->scan_spot_position_map.push_back (+10.f + seg);
                cp->scan_spot_position_map.push_back (+10.f);

                cp->scan_spot_meterset_weights.push_back (5);
                cp->scan_spot_meterset_weights.push_back (5);
                cp->scan_spot_meterset_weights.push_back (10);
                cp->scan_spot_meterset_weights.push_back (5);
            }
            cum_gp += gp;
        }
        beam->final_cumulative_meterset_weight = cum_gp;
    }

    /* Save to file */
    drs.save (dicom_dir);
    
    return 0;
}
