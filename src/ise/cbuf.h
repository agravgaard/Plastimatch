/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _cbuf_h_
#define _cbuf_h_

#include <list>

class Frame;
class QMutex;

class Cbuf {
public:
    Cbuf ();
    ~Cbuf ();

    void clear (void);
    int init (unsigned int idx, unsigned int num_frames, 
        unsigned int size_x, unsigned int size_y);

    Frame* get_frame ();
    void add_empty_frame (Frame* new_frame);
    void add_waiting_frame (Frame* f);
    std::list<Frame*>::iterator& display_lock_newest_frame ();
    std::list<Frame*>::iterator& display_lock_frame (int pos);
    std::list<Frame*>::iterator& display_lock_oldest_frame ();
    Frame* get_display_frame ();
    int get_size_x ();
    int get_size_y ();

public:
    unsigned long idx;
    unsigned long num_frames;
    unsigned long size_x;
    unsigned long size_y;
    std::list<Frame*> frames;
    unsigned long writable;
    unsigned long waiting_unwritten;
    unsigned long dropped;
    std::list<Frame*> empty;
    std::list<Frame*> waiting;
    Frame* write_ptr;
    //Frame* display_ptr;
    Frame* internal_grab_ptr;
    std::list<Frame*>::iterator display_ptr;

    QMutex *mutex;
};


#if defined (commentout)
void cbuf_init (IseFramework* ig);
int cbuf_init_queue (IseFramework* ig, unsigned int idx, unsigned int num_frames);
void cbuf_append_waiting (CBuf* cbuf, Frame* new_frame);
void cbuf_insert_waiting (CBuf* cbuf, Frame* new_frame);
void cbuf_append_empty (CBuf* cbuf, Frame* new_frame);
Frame* cbuf_get_empty_frame (CBuf* cbuf);
Frame* cbuf_get_any_frame (CBuf* cbuf, int flush_dark);
unsigned long cbuf_queuelen (CBuf* cbuf, FrameQueue* queue);
int cbuf_shutdown (IseFramework* ig);
void cbuf_shutdown_queue (IseFramework* ig, CBuf* cbuf);
Frame* cbuf_display_lock_newest_frame (CBuf* cbuf);
Frame* cbuf_display_lock_newest_bright (CBuf* cbuf);
Frame* cbuf_display_lock_oldest_frame (CBuf* cbuf);
Frame* cbuf_display_lock_frame_by_idx (CBuf* cbuf, int frame_no);
Frame* cbuf_display_lock_next_frame (CBuf* cbuf);
Frame* cbuf_get_next_writable_frame (CBuf* cbuf);
void cbuf_reset_queue (CBuf* cbuf);
void cbuf_display_lock_release (CBuf* cbuf);
void cbuf_internal_grab_rewind (CBuf* cbuf);
Frame* cbuf_internal_grab_next_frame (CBuf* cbuf);
Frame* cbuf_display_lock_internal_grab (CBuf* cbuf);
void cbuf_append_waiting (CBuf* cbuf, Frame* new_frame);
void cbuf_mark_frame_written (CBuf* cbuf, Frame* frame);
#endif

#endif
