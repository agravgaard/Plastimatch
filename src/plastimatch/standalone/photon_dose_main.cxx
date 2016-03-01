/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>

#include "plm_math.h"
#include "photon_dose.h"
#include "photon_parms.h"
#include "photon_plan.h"
#include "volume.h"

int
main (int argc, char* argv[])
{
    
    time_t tbegin, tend;
    double texec =0.;
    tbegin = time(NULL);

    Photon_parms parms;
    if (!parms.parse_args (argc, argv)) {
        return 1;
    }

    tend = time(NULL);
    texec = difftime(tend,tbegin);
    printf("Execution time : %lg secondes.\n",texec);

    return 0;
}
