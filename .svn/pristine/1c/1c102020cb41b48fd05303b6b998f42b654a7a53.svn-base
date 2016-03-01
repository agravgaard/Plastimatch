/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
   igtalk 0      // simulates panel 0
   igtalk 1      // simulates panel 1
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#define _USE_32BIT_TIME_T 1
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <io.h>
//#include "dips_if.h"
#include "dips_panel.h"

#define FLUORO_MODE 0
//#define HIRES_IMAGE_HEIGHT 1536
//#define HIRES_IMAGE_WIDTH 2048

void igpax_poll_images (PANEL* panelp, unsigned short* pixelp);

int
main (int argc, char* argv[])
{
    int selector;

    if (argc != 2) {
        printf ("Usage: igpax panel_number\n");
	exit (2);
    }
    selector = atoi(argv[1]);

    /* Open shared memory connection with DIPS */
    Dips_panel dp;
    dp.open_panel (selector, HIRES_IMAGE_HEIGHT, HIRES_IMAGE_WIDTH);

    /* Create dummy images in infinite loop */
    dp.poll_dummy ();

    /* Never gets here */
    return 0;
}

