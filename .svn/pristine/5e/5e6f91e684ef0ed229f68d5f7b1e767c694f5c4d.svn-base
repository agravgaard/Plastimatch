/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _synthetic_source_h_
#define _synthetic_source_h_

#include "fluoro_source.h"

class Iqt_main_window;
class Synthetic_source_thread;

class Synthetic_source : public Fluoro_source {
public:
    Synthetic_source (Iqt_main_window *mw, int width, int height, double ampl, int fps);

public:
    virtual unsigned long get_size_x (int x);
    virtual unsigned long get_size_y (int y);
    virtual const std::string get_type ();
    virtual void start ();
    virtual void grab_image (Frame* f);
    int width;
    int height;
    double ampl;
    int fps;
    virtual void stop ();
    
public:
    Synthetic_source_thread *thread;
    
};

void synthetic_grab_image (Frame* f);

#endif
