/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include "advantech.h"

int 
main (int argc, char* argv[])
{
    Advantech adv;
    printf ("Sleeping...\n");
    Sleep (2000);
    printf ("Opening relay...\n");
    adv.relay_open (0);
    Sleep (2000);
    printf ("Closing relay...\n");
    adv.relay_close (0);
    Sleep (2000);
    printf ("Done sleeping.\n");
    return 0;
}
