/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "bitflow.h"

int main (int argc, char* argv[])
{
    unsigned short *img;
    BitflowInfo bf;
    Ise_Error rc;
    FILE* fp;


    //initalize board
    bitflow_init (&bf, ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO);

    //open board
    rc = bitflow_open (&bf, 0, 0, ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO, 
	ISE_FRAMERATE_7_5_FPS);
    if (rc) {
	fprintf(stdout, "board opend failed\n");
	exit(-1);
    }

    //prepare to grab
    rc = bitflow_grab_setup(&bf, 0);

    //start grabbing
    img = (unsigned short *) malloc(bf.imageSize);
    bitflow_grab_image(img, &bf, 0);
	
    fp = fopen("tmp1.raw","wb");
    if (!fp) {
	fprintf (stdout, "Couldn't open tmp1.raw\n");
    }
    fwrite (img, 2, 768*1024, fp);
    fclose (fp);

    bitflow_grab_image(img, &bf, 0);

    fp = fopen("tmp2.raw","wb");
    if (!fp) {
	fprintf (stdout, "Couldn't open tmp2.raw\n");
    }
    fwrite (img, 2, 768*1024, fp);
    fclose (fp);


    rc = bitflow_shutdown(&bf, 0);

    if (rc) 
    {
	fprintf(stdout, "close board failed\n");
	exit(-1);
    }

    free(img);
}
