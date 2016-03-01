/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include <stdio.h>
#include "frame.h"
#include "ise_globals.h"

Frame::Frame (unsigned long size_x, unsigned long size_y)
{
    frame_clear (this);
    this->img = new unsigned short[size_x * size_y];
}

Frame::~Frame ()
{
    delete[] this->img;
}

void
frame_clear (Frame* f)
{
    /* NOTE: This does not clear img */

    f->id = 0;
    f->timestamp = 0;
    f->writable = 0;
    f->written = 0;
    f->write_lock = 0;
    f->display_lock = 0;

    f->autosense.is_dark = 0;
    f->autosense.min_brightness = 0;
    f->autosense.max_brightness = 0;
    f->autosense.mean_brightness = 0;
    f->indico_state = 0;

    f->clip_x = -1;
    f->clip_y = -1;
}

int
frame_is_dark (Frame* f)
{
    return f->autosense.is_dark;
}

int
frame_is_bright (Frame* f)
{
    return ! f->autosense.is_dark;
}

int
frame_needs_write (Frame* f)
{
    if (f->writable && !f->written) 
    {
        return 1;
    } 
    else 
    {
        return 0;
    }
}

void
frame_autosense (Frame* f, int prev_dark, unsigned long rows, unsigned long cols)
{
    Autosense* as = &(f->autosense);
    unsigned long brightness;
    unsigned short max_brightness;
    unsigned short *pImg16 = (unsigned short *)f->img;
    int i;
    int num_samples = 6;
    int pR1[6], pR2[6], pC[6];
    int bigstep = 100;
    int smallstep = 2;
    const int dark_to_bright_thresh = 35;
    const int bright_to_dark_thresh = 25;

#if defined (commentout)
    const int dark_to_bright_thresh = 20;
    const int bright_to_dark_thresh = 10;
#endif
    
    pR1[0] = rows/2 - 3 * bigstep;
    pR2[0] = rows/2 - 3 * smallstep + 1;
    pC[0] = cols/2 - 3 * bigstep;

    for (i = 1; i < num_samples; i ++)
    {
        pR1[i] = pR1[i-1] + bigstep;
        pR2[i] = pR2[i-1] + smallstep;
        pC[i] = pC[i-1] + bigstep;
    }
    
    
    // ??? Rui: some of these indices are out of range
    /*int c1[] = { 700, 801, 900, 1001, 1100, 1201
    };
    int r1[] = { 500, 620, 741, 860, 980, 1201
    };
    int c2[] = { 700, 801, 900, 1001, 1100, 1201
    };
    int r2[] = { 760, 762, 764, 766, 768
    };
    */

    /* First check (random) */
    brightness = 0;
    as->max_brightness = 0;
    as->min_brightness = MAXGREY;
    for (i = 0; i < num_samples; i++) {
	    /* raw frames are row major, right? */
	    unsigned short sample = pImg16[pR1[i]*cols+pC[i]];
        brightness += sample;
	    if (sample > as->max_brightness) 
	    {
	        as->max_brightness = sample;
	    }
	    if (sample < as->min_brightness) 
	    {
	        as->min_brightness = sample;
	    }
    }

    as->mean_brightness = (unsigned short) (brightness / num_samples);

    /* Second check (near center) */
    brightness = 0;
    for (i = 0; i < num_samples; i++) {
		/* raw frames are row major, right? */
		unsigned short sample = pImg16[pR2[i]*cols+pC[i]];
        brightness += sample;
		if (sample > as->max_brightness) 
		{
			as->max_brightness = sample;
		}
    }
    as->ctr_brightness = (unsigned short) (brightness / num_samples);

    max_brightness = as->mean_brightness;
    if (as->ctr_brightness > max_brightness) {
	max_brightness = as->ctr_brightness;
    }

    if (prev_dark) {
	if (max_brightness < dark_to_bright_thresh) {
	    as->is_dark = 1;
	} else {
	    as->is_dark = 0;
	}
    } else {
	if (max_brightness > bright_to_dark_thresh) {
	    as->is_dark = 0;
	} else {
	    as->is_dark = 1;
	}
    }
}
