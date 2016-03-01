/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>

#include "landmark_diff.h"
#include "raw_pointset.h"

int
main (int argc, char** argv)
{
    int rv;
    Raw_pointset *lm0;
    Raw_pointset *lm1;

    if (argc < 3) {
        printf ("Usage:\n"
                "  landmark_diff file_1 file_2\n\n"
        );
        return 0;
    }

    lm0 = pointset_load (argv[1]);
    lm1 = pointset_load (argv[2]);

    rv = landmark_diff (lm0, lm1);

    pointset_destroy (lm0);
    pointset_destroy (lm1); 

    return rv;
}
