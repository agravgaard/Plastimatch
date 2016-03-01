/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* Libraries */
#include <math.h>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>
#include <stdlib.h>
#include <string.h>

/* Headers */
#include "fatm.h"
#include "iqt_application.h"
#include "iqt_main_window.h"
#include "iqt_video_widget.h"
#include "iqt_tracker.h"
#include "sleeper.h"
#include "tracker_thread.h"

Tracker_thread::Tracker_thread ()
{
    this->best_score = -1;
    this->best_row = 0;
    this->best_col = 0;
}

Tracker_thread::~Tracker_thread () {
}

void 
Tracker_thread::set_tracker (Tracker *t)
{
    this->tracker = t;
}

void
Tracker_thread::run ()
{
    while (1) {
	ise_app->mutex.lock();
	ise_app->frameLoaded.wait(&(ise_app->mutex));
	ise_app->frameLoaded.wakeAll();

	this->tracker->tracker_initialize();
	fatm_run(tracker->fatm);

	this->find_max ();
       //qDebug ("Maximum Point: %d, %d", (int)this->best_row, (int)this->best_col);
	qDebug("Score point: %d, %d",
	       (int)this->best_row,
	       (int)this->best_col);
	qDebug("Signal point: %d, %d",
	       (int)this->best_row + 175,
	       (int)this->best_col + 175);

	//this->tracker->mw->trackPoint ((this->best_row+175+5), (this->best_col+175+5));//deleted by YK

#if defined (commentout)
	/* Save to File */
        static int i = 0;
        QString fn;
        fn.sprintf ("/home/willemro/tmp/score_%04d.pfm", i);
        image_write (&tracker->fatm->score, fn.toUtf8().constData());
        fn.sprintf ("/home/willemro/tmp/sig_%04d.pfm", i);
        image_write (&tracker->fatm->sig, fn.toUtf8().constData());
        fn.sprintf ("/home/willemro/tmp/pat_%04d.pfm", i);
        image_write (&tracker->fatm->pat, fn.toUtf8().constData());

        i++;
#endif
	ise_app->mutex.unlock();
    }
    /* after running, send signal to this->tracker->mw with scoring information */
    /* or to this->tracker->mw->vid_screen for the point around which to make
       the tracking square */
}

void
Tracker_thread::find_max ()
{
    this->best_score = -1;
    for (int r = 0; r < 175; r++) {
        for (int c = 0; c < 175; c++) {
            /* Force column major */
	    double score = image_data(&tracker->fatm->score)[c*175 + r];
	    if (score > this->best_score) {
		this->best_score = score;
		this->best_row = (double) r;
		this->best_col = (double) c;
	    }
	}
    }
}
