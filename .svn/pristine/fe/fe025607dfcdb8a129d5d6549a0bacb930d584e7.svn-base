/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef _iqt_tracker_h_
#define _iqt_tracker_h_
#include "fatm.h"

class Image_Rect;
class Iqt_main_window;
class Tracker_thread;

class Tracker {
public:
    Tracker (Iqt_main_window *mw);
    ~Tracker ();
public:
    Tracker_thread *tracker_thread;
    void tracker_initialize ();
    Iqt_main_window *mw;
    FATM_Options *fatm;
};

#endif
