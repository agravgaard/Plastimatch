/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#define _USE_32BIT_TIME_T 1
#include "dips_if.h"
#include "dips_panel.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <io.h>
#include "aqprintf.h"

#define FLUORO_MODE 0
//#define HIRES_IMAGE_HEIGHT 1536
//#define HIRES_IMAGE_WIDTH 2048
//#define HIRES_IMAGE_HEIGHT 2304
//#define HIRES_IMAGE_WIDTH 3200

Dips_panel::Dips_panel ()
{
    panel_no = 0;
    height = 0;
    width = 0;
    panelp = 0;
    pixelp = 0;
}

Dips_panel::~Dips_panel ()
{
    /* Do nothing */
}

void 
Dips_panel::open_panel (int panel_no, int height, int width)
{
    char panel_name[12], pixel_name[12];
    HANDLE panelh, pixelh;

    /* Save height and width */
    this->height = height;
    this->width = width;

    /* Set up shared memory */
    sprintf (panel_name, "PANEL%i", panel_no);
    sprintf (pixel_name, "PIXEL%i", panel_no);
    panelh = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, 
	PAGE_READWRITE,	0, sizeof (PANEL), 
	panel_name);
    if (!panelh) {
	fprintf (stderr, "Error opening shared memory for panel\n");
	exit (1);
    }
    pixelh = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, 
	PAGE_READWRITE,	0, width * height * 2, 
	pixel_name);
    if (!panelh) {
	fprintf (stderr, "Error opening shared memory for pixel\n");
	exit (1);
    }
    this->panelp = (struct PANEL*) 
	MapViewOfFile (panelh, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!this->panelp) {
	fprintf (stderr, "Error mapping shared memory for panel\n");
	exit (1);
    }
    this->pixelp = (unsigned short*) 
	MapViewOfFile (pixelh, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!this->pixelp) {
	fprintf (stderr, "Error mapping shared memory for pixel\n");
	exit (1);
    }
    this->panelp->status = READ;
    this->panelp->time = 0;
    this->panelp->ale = 0;
    this->panelp->xs = width;
    this->panelp->ys = height;
    this->panelp->depth = 2;
    this->panelp->pixel = (short*) this->pixelp;

    /* Initialize random number generator for dummy images */
    srand ((unsigned) time(NULL));
}

void
Dips_panel::wait_for_dips (void)
{
    int i = 0;
    while ((panelp->status & READ) == 0) {
        aqprintf ("Waiting for dips...\n");
	//PrintCurrentTime();

        SleepEx (500, FALSE);
        if (++i > 4) {
            aqprintf ("Dips didn't read the last image?  Continuing...\n");
	    //PrintCurrentTime();
            break;
        }
    }
}

void
Dips_panel::send_image (void)
{
    /* Set timestamp */
    aqprintf ("Sending image to dips.\n");
    //PrintCurrentTime();

    time (&panelp->time);

    /* Let DIPS know we have an image */
    panelp->status = VALID;
}

/* A READ value of 1 means that DIPS has read the image. */
void
Dips_panel::poll_dummy (void)
{
    int x, y;
    unsigned short* p;

    while (1) {
	while ((panelp->status & READ) == 0) {
	    SleepEx (500, FALSE);
	}

	/* Fill in dummy image */
	p = pixelp;
	for (y = 0; y < height; y++) {
	    for (x = 0; x < width; x++) {
		if (x < 500 || x > width - 500) {
		    *p = 4400;
		} else if (y < 500 || y > height - 500) {
		    *p = 5500;
		} else {
		    *p = 8800;
		}
		*p += (rand() % 1000);
		p++;
	    }
        }
	aqprintf ("\"Captured\" image!\n");
	////PrintCurrentTime();

	/* Set timestamp */
	time (&panelp->time);

	/* For DIPS 3, we don't want so often ;-) */
	SleepEx (5000, FALSE);

	aqprintf ("Hit any key to send another image to dips\n");
	//PrintCurrentTime();
	getchar ();

	/* Let DIPS know we have an image */
	panelp->status = VALID;
    }
}
