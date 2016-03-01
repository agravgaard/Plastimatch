/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "fatm.h"
#include "iqt_tracker.h"
#include "tracker_thread.h"
#include "iqt_main_window.h"
#include "image.h"
#include "frame.h"
#include "iqt_application.h"
#include "cbuf.h"
#include <QDebug>

Tracker::Tracker (Iqt_main_window *mw) 
{
    this->tracker_thread = new Tracker_thread;
    this->tracker_thread->set_tracker (this);
    this->mw = mw;
    this->fatm = new FATM_Options;
}

Tracker::~Tracker () 
{
    delete tracker_thread;
    delete fatm;
}


void
Tracker::tracker_initialize (/*bool user_track*/)
{
#if defined (this_is_today)
    FATM_Options *fopt;
    int pat_size[] = { 21, 21 };
    int sig_size[] = { 151, 151 };

    /* Initialize fopt array */
    fopt = fatm_initialize ();

    /* 
       fopt->pat_rect = pointer to pattern
       fopt->sig_rect = pointer to signal 
       fopt->score = pointer to score
    */

    fopt->command = MATCH_COMMAND_COMPILE;
    fopt->alg = MATCH_ALGORITHM_FNCC;
    fatm_compile (fopt);

    fatm_run (fopt);
    /*
      fopt->pat_rect = new pattern 
      fatm_run (fopt);
    */
#endif

    bool user_track = false;
    bool user_def = false;
    int dims[2] = {10, 10};

    int sig_dims[2] = {175, 175};
    //signal->set_dims (sig_dims); /*arbitrary, could be user-defined as well*/
    
    fatm = fatm_initialize ();
    fatm->alg = MATCH_ALGORITHM_FNCC;

    image_init(&fatm->sig);
    image_init(&fatm->pat);
        
    Frame *f = *(ise_app->cbuf[0]->display_ptr);
    unsigned short *img = f->img;

    if (!user_def) {
        image_malloc (&fatm->sig, sig_dims);
    }

    image_malloc (&fatm->pat, dims);
    //image_double (&fatm->pat);
    //double patt[dims[0]*dims[1]];

    if (!user_track) {
	double value;
	
	for (int i = 0; i < dims[0]; i++) {
	    for (int j = 0; j < dims[1]; j++) {
		if (i>1 && i<8) {
		    if (j == 4 || j == 5) {
			value = 1;
		    } else {
			value = 0;
		    }
		} else {
		    value = 0;
		}
		/* vertical bright rectangle */
                image_data(&fatm->pat)[i*dims[1]+j] = value;
    	    }
    	}
    }

    /* initial score area */
    
    int score_dims[2] = {175, 175};
    int score_val2[2] = {20, 20};

    image_init (&fatm->score);
    image_malloc (&fatm->score, score_dims);
    
    fatm->score_rect.score_rect_full.pmin[0] = 0;
    fatm->score_rect.score_rect_full.pmin[1] = 0;
    fatm->score_rect.score_rect_full.set_dims(score_dims);

    fatm->score_rect.score_rect_valid.pmin[0] = 0;
    fatm->score_rect.score_rect_valid.pmin[1] = 0;
    fatm->score_rect.score_rect_valid.set_dims(score_val2);

    /* for subsequent iterations, make the score window 50 pixels
       (in each direction) from previous detection point */

//    fatm->pat.data = patt;

//    fatm->sig_rect.pmin[0] = 175;
//    fatm->sig_rect.pmin[1] = 175;
    fatm->sig_rect.pmin[0] = 0;
    fatm->sig_rect.pmin[1] = 0;
    fatm->sig_rect.set_dims(sig_dims);
    fatm->pat_rect.pmin[0] = 0;
    fatm->pat_rect.pmin[1] = 0;
    fatm->pat_rect.set_dims(dims);
    

//    double signal_img[fatm->sig.dims[0]*fatm->sig.dims[1]];
    
    for (int i = 0; i < (fatm->sig.dims[0]); i++) {
	for (int j = 0; j < (fatm->sig.dims[1]); j++){
//	    signal_img[i*fatm->sig.dims[0] + j] = img[(i+175)*(512) + (j+175)];
            image_data(&fatm->sig)[i*fatm->sig.dims[0] + j]
                = img[(i+175)*(512) + (j+175)];
	}
    }

//    fatm->sig.data = signal_img;
//    fatm->score.data = signal_img;

    /*
      if (!user_track) {
      unsigned short value;
      //int value;
	
      /* create fatm->pat_rect 
      for (int i = 0; i < dims[0]; i++) {
      for (int j = 0; j < dims[1]; j++) {
      if (i>1 && i<8) {
      if (j == 4 || j == 5) {
      value = 0xFFFF;
      } else {
      value = 0;
      }
      } else {
      value = 0;
      }
      vertical bright rectangle 
      fatm->pat.data[i*dims[1]+j] = value;
      }
      }
      }
    
      else {
      for (int i=0; i < 10; i++) {
      for (int j=0; j < 10; j++) {
      fatm->pat.data[i*dims[1]+j] = img[(fatm->pat_rect.pmin[0]+i)*(512) 
      + (fatm->pat_rect.pmin[1]+j)];
      pmin[] is top-left pixel ({x, y}) of user-defined fatm->pat_rect,
      would be from "trace" in iqt_video_widget 
      need to know how the img[] array is laid out
      }
      }
      }    

    */

    //fatm->pat_rect = *(fatm->pat_rect);

    fatm_compile (fatm);
}
