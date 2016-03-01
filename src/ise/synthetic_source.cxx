/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <QDebug>
#include <QObject>
#include "frame.h"
#include "iqt_main_window.h"
#include "ise_globals.h"
#include "synthetic_source.h"
#include "synthetic_source_thread.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Synthetic_source::Synthetic_source (Iqt_main_window* mw, int width, int height, double ampl, int fps)
{
    this->width = width;
    this->height = height;
    this->ampl = ampl;
    this->fps = fps;
    this->thread = new Synthetic_source_thread (width, height, fps);
    this->thread->set_synthetic_source (this);
    qDebug ("connecting: %p %p", this->thread, mw);
    QObject::connect (this->thread, SIGNAL(frame_ready(int, int)), 
        mw, SLOT(slot_frame_ready(int, int)));    
}

void
Synthetic_source::start ()
{
    this->thread->playing = true;
    this->thread->start ();
}

void
Synthetic_source::stop ()
{
    this->thread->playing = false;
}

unsigned long
Synthetic_source::get_size_x (int x) {
    return x;
    //return 2048;
}

unsigned long
Synthetic_source::get_size_y (int y) {
    return y;
    //return 1536;
}

const std::string 
Synthetic_source::get_type () {
    return std::string("Synthetic");
}

/* -------------------------------------------------------------------------*
   Simulated-related routines
 * -------------------------------------------------------------------------*/
void
simulate_image_ramp (Frame* f, int x_size, int y_size)
{
    int x, y;
    unsigned short* p;

    /* Fill in dummy image */
    p = (unsigned short*) f->img;
    for (y = 0; y < y_size; y++) {
	for (x = 0; x < x_size; x++) {
	    if (y < 50 || y > y_size - 50) {
		*p = MAXGREY;
	    } else {
		*p = (unsigned short) (MAXGREY * ((double) x / (double) x_size));
	    }
	    p++;
	}
    }
}

static void
simulate_clip_pos_sin (Frame* f, int x_size, int y_size, double amplitude)
{
    static double phase = 0.0;
    double amp = 50.0 * amplitude;

    int xpos = x_size / 2;
    int ypos = (y_size / 2) + (int) (amp*sin(phase));
    f->clip_x = xpos;
    f->clip_y = ypos;
    // 15fps=60f/cyc
    phase += (M_PI / 30.0);	/* 60 frames per cycle */
}

static void
simulate_image_fill_bg (Frame* f, int x_size, int y_size, 
    unsigned short bg_color)
{
    for (int i = 0; i < x_size * y_size; i++) {
	f->img[i] = bg_color;
    }
}

static void
simulate_image_fill_fg (Frame* f, int x_size, int y_size, 
    unsigned short fg_color)
{
    int xmin = f->clip_x - 3, xmax = f->clip_x + 3;
    int ymin = f->clip_y - 5, ymax = f->clip_y + 5;
    int x, y;

    for (y = ymin; y <= ymax; y++) {
	for (x = xmin; x <= xmax; x++) {
	    f->img[y*x_size+x] = fg_color;
	}
    }

}

void
simulate_image_clip_sin (Frame* f, int x_size, int y_size, double amplitude)
{
    simulate_image_fill_bg (f, x_size, y_size, 800);
    simulate_clip_pos_sin (f, x_size, y_size, amplitude);
    simulate_image_fill_fg (f, x_size, y_size, 200);
}

void
simulate_image_clip_fluoro_pulse (Frame* f, int x_size, int y_size, double amplitude)
{
    static int countdown1 = 10;
    static int countdown2 = 20;
    if (countdown1 > 0 || countdown2 <= 0) {
	if (--countdown1 < 0) countdown1 = 0;
	simulate_image_fill_bg (f, x_size, y_size, 30);
	simulate_clip_pos_sin (f, x_size, y_size, amplitude);
	simulate_image_fill_fg (f, x_size, y_size, 10);
    } else {
	--countdown2;
	simulate_image_fill_bg (f, x_size, y_size, 300);
	simulate_clip_pos_sin (f, x_size, y_size, amplitude);
	simulate_image_fill_fg (f, x_size, y_size, 100);
    }
}


void
simulate_image_clip_fluoro_pulses (Frame* f, int x_size, int y_size, double amplitude)
{
    static int countdown1 = 10;
    static int countdown2 = 10;
    if (countdown1 > 0) {
	if (--countdown1 == 0) {
	    countdown2 = 10;
	}
	simulate_image_fill_bg (f, x_size, y_size, 20);
	simulate_clip_pos_sin (f, x_size, y_size, amplitude);
	simulate_image_fill_fg (f, x_size, y_size, 10);
    } else {
	if (--countdown2 == 0) {
	    countdown1 = 10;
	}
	simulate_image_fill_bg (f, x_size, y_size, 300);
	simulate_clip_pos_sin (f, x_size, y_size, amplitude);
	simulate_image_fill_fg (f, x_size, y_size, 100);
    }
}

void
simulate_image_clip_line (Frame* f, int x_size, int y_size, double amplitude)
{
    int y;
    simulate_image_fill_bg (f, x_size, y_size, 800);
    simulate_clip_pos_sin (f, x_size, y_size, amplitude);
    for (y = 0; y < y_size; y++) {
	int x1 = y;
	int x2 = y + (x_size-y_size);
	int x3 = x_size - x1 - 1;
	int x4 = x_size - x2 - 1;
	f->img[y*x_size+x1] = 400;
	f->img[y*x_size+x2] = 400;
	f->img[y*x_size+x3] = 400;
	f->img[y*x_size+x4] = 400;
    }
    simulate_image_fill_fg (f, x_size, y_size, 200);
}

static void
simulate_image (Frame* f, int x_size, int y_size, double amplitude)
{
    simulate_image_clip_sin (f, x_size, y_size, amplitude);
#if defined (commentout)
    simulate_image_ramp (f, x_size, y_size);
    simulate_image_clip_line (f, x_size, y_size);
    simulate_image_clip_fluoro_pulse (f, x_size, y_size);
    simulate_image_clip_fluoro_pulses (f, x_size, y_size);
#endif
}

void
Synthetic_source::grab_image (Frame* f)
{
    QString t;
    qDebug() << t.sprintf("F pointer = %p", f);
    qDebug() << t.sprintf("img pointer = %p", f->img);
    qDebug("Height: %d \nWidth: %d \nAmplitude: %d", height, width, (int)ampl);
    simulate_image (f, width, height, ampl);
    qDebug() << "Simulation complete";
}
