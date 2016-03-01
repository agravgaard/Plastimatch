/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _clamp_h_
#define _clamp_h_

#define CLAMP(value, min_value, max_value)        \
    do {                                          \
        if (value < min_value) {                  \
            value = min_value;                    \
        } else if (value > max_value) {           \
            value = max_value;                    \
        }                                         \
    } while (0)

#endif
