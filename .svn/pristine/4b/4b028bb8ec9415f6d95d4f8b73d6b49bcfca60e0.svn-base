/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _tracker_thread_h_
#define _tracker_thread_h_

#include <QThread>

class Tracker;
class Frame;

class Tracker_thread : public QThread {
public:
    Q_OBJECT
    ;
    
public:
    Tracker_thread ();
    virtual ~Tracker_thread ();
    double best_score;
    double best_row;
    double best_col;
    void find_max (void);
    void set_tracker (Tracker *t);

protected:
    virtual void run();

public:
    Tracker *tracker;

};

#endif
