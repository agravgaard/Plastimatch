/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "plm_math.h"
#include "plm_timer.h"
#include "rt_plan.h"
#include "volume.h"

int
main (int argc, char* argv[])
{
    Plm_timer timer;
    Rt_plan plan;
    timer.start ();
    if (plan.parse_args (argc, argv) != PLM_SUCCESS) {
        return 1;
    }
    plan.compute_plan ();
    printf("Execution time : %f secondes.\n", timer.report ());
    return 0;
}
