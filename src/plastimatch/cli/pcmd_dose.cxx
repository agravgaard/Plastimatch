/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdlib.h>

#include "logfile.h"
#include "pcmd_dose.h"
#include "plm_return_code.h"
#include "plm_timer.h"
#include "rt_plan.h"

void
do_command_dose (int argc, char* argv[])
{

    if (argc < 2) {
        lprintf ("Usage: plastimatch dose command_file\n");
        exit (1);
    }

    char *command_file = argv[2];

    Plm_timer timer;
    Rt_plan plan;
    timer.start ();
    if (plan.set_command_file (command_file) != PLM_SUCCESS) {
        lprintf ("Error parsing command file.\n");
        return;
    }
    plan.compute_plan ();
    lprintf ("Execution time : %f secondes.\n", timer.report ());
}
