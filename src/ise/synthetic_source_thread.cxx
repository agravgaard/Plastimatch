/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <QDebug>

#include "cbuf.h"
#include "frame.h"
#include "ise_globals.h"
#include "iqt_main_window.h"
#include "sleeper.h"
#include "synthetic_source.h"
#include "synthetic_source_thread.h"

Synthetic_source_thread::Synthetic_source_thread (int width, int height, int fps) 
{
    this->width = width;
    this->height = height;
    this->sleep = 1000/fps;
}

Synthetic_source_thread::~Synthetic_source_thread () {
}

void 
Synthetic_source_thread::set_synthetic_source (Synthetic_source *ss)
{
    this->ss = ss;
}

void
Synthetic_source_thread::run () {
    while (playing) {
        //qDebug() << "Hello world";
        Frame *f = this->ss->cbuf->get_frame ();
        qDebug() << "Got frame.";
        Sleeper::msleep (sleep);
        
        qDebug() << "Grabbing synth image.";
        this->ss->grab_image (f);
        this->ss->cbuf->add_waiting_frame (f);

        this->ss->cbuf->display_lock_newest_frame ();
        emit frame_ready (width, height);

        qDebug() << "Done.";
    }
}
