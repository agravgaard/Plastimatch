/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <limits>

#include "float_pair_list.h"
#include "plm_math.h"
#include "string_util.h"

static void 
get_next_number (int& rc, float& f, const char*& c)
{
    int n;

    /* Skip whitespace */
    while (isspace(*c)) ++c;

    if (string_starts_with (c, "inf")) {
        rc = 1;
        f = std::numeric_limits<float>::max();
        c += 3;
    } else if (string_starts_with (c, "-inf")) {
        rc = 1;
        f = -std::numeric_limits<float>::max();
        c += 4;
    } else {
        rc = sscanf (c, "%f%n", &f, &n);
        if (rc >= 1) {
            c += n;
        }
    }

    /* Skip whitespace */
    while (isspace(*c)) ++c;
    /* Skip trailing comma */
    while (*c == ',') ++c;
}

Float_pair_list
parse_float_pairs (const std::string& s)
{
    Float_pair_list al;
    const char* c = s.c_str();

    while (1) {
        int rc;
        float f1, f2;
        get_next_number (rc, f1, c);
        if (rc < 1) {
            break;
        }
        get_next_number (rc, f2, c);
        if (rc < 1) {
            break;
        }

        /* Look for end-caps */
        if (!is_number(f1)) {
            if (al.size() == 0) {
                f1 = -std::numeric_limits<float>::max();
            } else {
                f1 = std::numeric_limits<float>::max();
            }
        }
        /* Append (x,y) pair to list */
        al.push_back (std::make_pair (f1, f2));
    }

    return al;
}
