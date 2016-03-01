/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"
#include <stdio.h>
#include <stdlib.h>

#include "diff.h"
#include "pcmd_diff.h"

static void
diff_print_usage (void)
{
    printf ("Usage: plastimatch diff image_in_1 image_in_2 image_out\n"
	    );
    exit (-1);
}

static void
diff_parse_args (Diff_parms* parms, int argc, char* argv[])
{
    if (argc != 5) {
	diff_print_usage ();
    }
    
    parms->img_in_1_fn = argv[2];
    parms->img_in_2_fn = argv[3];
    parms->img_out_fn = argv[4];
}

void
do_command_diff (int argc, char *argv[])
{
    Diff_parms parms;
    
    diff_parse_args (&parms, argc, argv);

    diff_main (&parms);
}
