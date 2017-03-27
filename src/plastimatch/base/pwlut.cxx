/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "print_and_exit.h"
#include "pwlut.h"

Pwlut::Pwlut ()
{
    left_slope = 1.0;
    right_slope = 1.0;
}

void
Pwlut::set_lut (const Float_pair_list& pwlut_fpl)
{
    /* Set the LUT */
    this->fpl = pwlut_fpl;

    /* Initialize some other variables for use during lookup */
    left_slope = 1.0;
    right_slope = 1.0;
    ait_start = fpl.begin();
    ait_end = fpl.end();
    if (ait_start->first == -std::numeric_limits<float>::max()) {
        left_slope = ait_start->second;
        ait_start++;
    }
    if ((--ait_end)->first == std::numeric_limits<float>::max()) {
        right_slope = ait_end->second;
        --ait_end;
    }
}

void
Pwlut::set_lut (const std::string& pwlut_string)
{
    /* Parse the LUT string */
    Float_pair_list fpl = parse_float_pairs (pwlut_string);
    if (fpl.empty()) {
        print_and_exit ("Error: couldn't parse pwlut string: %s\n",
            pwlut_string.c_str());
    }

    this->set_lut (fpl);
}

float
Pwlut::lookup (float vin) const
{
    float vout;

    /* Three possible cases: before first node, between two nodes, and 
       after last node */

    /* Case 1 */
    if (vin <= ait_start->first) {
        vout = ait_start->second + (vin - ait_start->first) * left_slope;
#if defined (commentout)
        printf ("[1] < %f (%f -> %f)\n", ait_start->first, vin, vout);
#endif
        return vout;
    }
    else if (ait_start != ait_end) {
        Float_pair_list::const_iterator ait = ait_start;
        Float_pair_list::const_iterator prev = ait_start;
        do {
            ait++;
            /* Case 2 */
            if (vin <= ait->first) {
                float slope = (ait->second - prev->second) 
                    / (ait->first - prev->first);
                vout = prev->second + (vin - prev->first) * slope;
#if defined (commentout)
                printf ("[2] in (%f,%f) (%f -> %f)\n", prev->first, 
                    ait->first, vin, vout);
#endif
                return vout;
            }
            prev = ait;
        } while (ait != ait_end);
    }
    /* Case 3 */
    vout = ait_end->second + (vin - ait_end->first) * right_slope;
#if defined (commentout)
    printf ("[3] > %f (%f -> %f)\n", ait_end->first, vin, vout);
#endif
    return vout;
}
