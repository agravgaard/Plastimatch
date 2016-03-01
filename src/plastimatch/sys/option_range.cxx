/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <stdio.h>
#include <stdlib.h>

#include "option_range.h"
#include "plm_math.h"
#include "string_util.h"

class Option_range_private {
public:
    std::list<float> range;
};

Option_range::Option_range ()
{
    d_ptr = new Option_range_private;
}

Option_range::~Option_range ()
{
    delete d_ptr;
}

void
Option_range::set_log_range (const std::string& range)
{
    float min_value;
    float max_value;
    float incr;
    d_ptr->range.clear ();

    /* Create list of values from the string */
    int rc = sscanf (range.c_str(), "%f:%f:%f", &min_value, &incr, &max_value);
    if (rc == 3) {
        /* Handle start:stride:stop form */
        for (float log_value = min_value; 
             log_value <= max_value;
             log_value += incr)
        {
            d_ptr->range.push_back (exp10_ (log_value));
        }
    } else {
        /* Handle value,value,.. form */
        const char *p = range.c_str();
        int n = 0;
        do {
            float log_value;
            n = 0;
            rc = sscanf (p, " %f ,%n", &log_value, &n);
            p += n;
            if (rc >= 1) {
                d_ptr->range.push_back (exp10_ (log_value));
            }
        } while (rc >= 1 && n > 0);
    }
}

void
Option_range::set_linear_range (const std::string& range)
{
    float min_value;
    float max_value;
    float incr;
    d_ptr->range.clear ();

    /* Create list of values from the string */
    int rc = sscanf (range.c_str(), "%f:%f:%f", &min_value, &incr, &max_value);
    if (rc == 3) {
        /* Handle start:stride:stop form */
        for (float lin_value = min_value; 
             lin_value <= max_value;
             lin_value += incr)
        {
            d_ptr->range.push_back (lin_value);
        }
    } else {
        /* Handle value,value,.. form */
        const char *p = range.c_str();
        int n = 0;
        do {
            float lin_value;
            n = 0;
            rc = sscanf (p, " %f ,%n", &lin_value, &n);
            p += n;
            if (rc >= 1) {
                d_ptr->range.push_back (lin_value);
            }
        } while (rc >= 1 && n > 0);
    }
}

void
Option_range::set_range (const std::string& range)
{
    if (range.length() > 1 && range[0] == 'L') {
        printf ("Setting log range\n");
        this->set_log_range (range.substr (1, std::string::npos));
    } else {
        printf ("Setting linear range\n");
        this->set_linear_range (range);
    }
}

void
Option_range::set_range (float range)
{
    d_ptr->range.clear ();
    d_ptr->range.push_back (range);
}

const std::list<float>&
Option_range::get_range ()
{
    return d_ptr->range;
}
