/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _frame_h_
#define _frame_h_

#include "autosense.h"

class Frame {
public:
    Frame (unsigned long size_x, unsigned long size_y);
    ~Frame ();
public:
    unsigned short* img;
	
    unsigned long id;
    double timestamp;
    int writable;
    int written;
    int write_lock;
    int display_lock;
    int indico_state;

    Autosense autosense;

    long clip_x;
    long clip_y;
};

void frame_clear (Frame* f);
void frame_autosense (Frame* f, int prev_dark, unsigned long rows, unsigned long cols);
int frame_is_dark (Frame* f);
int frame_is_bright (Frame* f);
int frame_needs_write (Frame* f);

#endif
