/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* -------------------------------------------------------------------------*
   The suffix "_crit" means that the function assumes we are 
   within a critical section.
 * -------------------------------------------------------------------------*/
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <process.h>
#include <direct.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <QDebug>
#include <QMutex>
#include "cbuf.h"
#include "debug.h"
#include "frame.h"

/* -------------------------------------------------------------------------*
   Declarations
 * -------------------------------------------------------------------------*/
#if defined (commentout)
static void cbuf_insert_update_stats (Cbuf* cbuf, Frame* frame);
static void cbuf_remove_update_stats (Cbuf* cbuf, Frame* frame);
#endif

Cbuf::Cbuf ()
{
    this->num_frames = 0;
    this->write_ptr = 0;
    this->display_ptr = this->waiting.end();
    this->internal_grab_ptr = 0;
    this->writable = 0;
    this->waiting_unwritten = 0;
    this->dropped = 0;

    this->mutex = new QMutex;
}

Cbuf::~Cbuf ()
{
}

void
Cbuf::clear (void)
{
    if (this->num_frames == 0) {
	return;
    }

    this->mutex->lock ();

    /* Release frames */
    while (!this->frames.empty()) {
        Frame *f = this->frames.front();
        this->frames.pop_front();
        delete f;
    }

    this->empty.clear();
    this->waiting.clear();
    this->num_frames = 0;
    this->writable = 0;
    this->waiting_unwritten = 0;
    this->dropped = 0;

    this->mutex->unlock ();
}

int
Cbuf::init (unsigned int idx, unsigned int num_frames, 
    unsigned int size_x, unsigned int size_y)
{
    this->mutex->lock ();

    this->idx = idx;
    this->num_frames = 0;
    this->size_x = size_x;
    this->size_y = size_y;
    //qDebug("Height: %d \nWidth:  %d", size_y, size_x);
    while (this->num_frames < num_frames) {
        /* Allocate new frame */
        Frame *f = new Frame (size_x, size_y);
        debug_printf ("cbuf[%d,%3d] = %d bytes\n", idx, this->num_frames, 
            sizeof(unsigned short) * size_x * size_y);

        QString t;
        qDebug() << t.sprintf("[%p %p]", f, f->img);

        if (!f) {
            /* We got less than the desired number of frames.  
               This is probably an error condition */
            this->mutex->unlock ();
            return 1;
        }
        this->num_frames ++;

        /* Add frame to frame array */
        this->frames.push_back (f);

        /* Add frame to list of empty frames */
        this->empty.push_back (f);
    }

    this->mutex->unlock ();
    return 0;
}

Frame*
Cbuf::get_frame ()
{
    this->mutex->lock ();

    /* Look for frame in empty list */
    if (!this->empty.empty()) {
        Frame *f = this->empty.front();
        this->empty.pop_front();
	frame_clear (f);
        this->mutex->unlock ();
        return f;
    }

    /* No frame in empty list, so grab oldest from waiting queue */
    Frame *f = this->waiting.front();
    this->waiting.pop_front();
    frame_clear (f);
    this->mutex->unlock ();
    return f;
}

void
Cbuf::add_waiting_frame (Frame* new_frame)
{
    if (!new_frame) return;

    this->mutex->lock ();
    this->waiting.push_back (new_frame);
    this->mutex->unlock ();
}

void
Cbuf::add_empty_frame (Frame* new_frame)
{
    if (!new_frame) return;

    this->mutex->lock ();
    this->empty.push_back (new_frame);
    this->mutex->unlock ();
}

std::list<Frame*>::iterator&
Cbuf::display_lock_newest_frame ()
{
    this->mutex->lock ();
    this->display_ptr = this->waiting.end();
    this->display_ptr --;
    this->mutex->unlock();
    return this->display_ptr;
}

std::list<Frame*>::iterator&
Cbuf::display_lock_frame (int pos)
{
    this->mutex->lock ();
    std::list<Frame*>::iterator it = this->waiting.begin();
    for (int i = 0; i < pos; i++) {
        ++it;
    }
    this->display_ptr = it;
    this->mutex->unlock();
    return this->display_ptr;
}

std::list<Frame*>::iterator&
Cbuf::display_lock_oldest_frame ()
{
    this->mutex->lock ();
    this->display_ptr = this->waiting.begin();
    this->mutex->unlock();
    return this->display_ptr;
}

Frame*
Cbuf::get_display_frame ()
{
    if (this->display_ptr == this->waiting.end()) {
        return 0;
    } else {
        return *(this->display_ptr);
    }
}

int
Cbuf::get_size_x ()
{
    return this->size_x;
}

int
Cbuf::get_size_y ()
{
    return this->size_y;
}

#if defined (commentout)
/* Return 1 if the frame is the tail frame */
int
cbuf_is_tail_frame (Frame* f)
{
    return !f->next;
}

static int
cbuf_is_locked (Cbuf* cbuf, Frame* f)
{
    //return (f && (cbuf->display_ptr == f || cbuf->write_ptr == f));
    return (f && (f->display_lock || f->write_lock));
}

void
cbuf_prepend_frame_crit (FrameQueue* queue, Frame* new_frame)
{
    Frame* f;

    new_frame->next = 0;
    if (queue->head) 
    {
        f = queue->head;
        f->prev = new_frame;
        new_frame->next = f;
        queue->head = new_frame;
    } 
    else 
    {
        new_frame->prev = 0;
        queue->head = new_frame;
        queue->tail = new_frame;
    }
    queue->queue_len ++;
}

void
cbuf_append_frame_crit (FrameQueue* queue, Frame* new_frame)
{
    Frame* f;

    new_frame->next = 0;
    if (queue->head) {
	f = queue->tail;
	f->next = new_frame;
	new_frame->prev = f;
	queue->tail = new_frame;
    } else {
	new_frame->prev = 0;
	queue->head = new_frame;
	queue->tail = new_frame;
    }
    queue->queue_len ++;
}

/* This function only unlinks the frame.  Caller needs to make sure that
   display_ptr & write_ptr don't point to it. */
void
cbuf_unlink_frame_crit (FrameQueue* queue, Frame* f)
{
    if (f->next) {
	f->next->prev = f->prev;
    } else {
	queue->tail = f->prev;
    }
    if (f->prev) {
	f->prev->next = f->next;
    } else {
	queue->head = f->next;
    }
    f->next = 0;
    f->prev = 0;
    queue->queue_len --;
}

static void
cbuf_set_display_lock_crit (Cbuf* cbuf)
{
    if (cbuf->display_ptr) cbuf->display_ptr->display_lock = 1;
}

static void
cbuf_reset_display_lock_crit (Cbuf* cbuf)
{
    if (cbuf->display_ptr) cbuf->display_ptr->display_lock = 0;
}

static void
cbuf_reset_display_ptr_crit (Cbuf* cbuf)
{
    if (cbuf->display_ptr && !cbuf->display_ptr->display_lock) {
	cbuf->display_ptr = 0;
    }
}

static void
cbuf_set_write_lock_crit (Cbuf* cbuf)
{
    if (cbuf->write_ptr) cbuf->write_ptr->write_lock = 1;
}

static void
cbuf_reset_write_lock_crit (Cbuf* cbuf)
{
    if (cbuf->write_ptr) cbuf->write_ptr->write_lock = 0;
}

static void
cbuf_reset_write_ptr_crit (Cbuf* cbuf)
{
    if (cbuf->write_ptr && !cbuf->write_ptr->write_lock) {
	cbuf->write_ptr = 0;
    }
}

void
cbuf_append_empty (Cbuf* cbuf, Frame* new_frame)
{
    EnterCriticalSection(&cbuf->cs);
    cbuf_append_frame_crit (&cbuf->empty, new_frame);
    LeaveCriticalSection(&cbuf->cs);
}

void
cbuf_append_waiting (Cbuf* cbuf, Frame* new_frame)
{
    EnterCriticalSection(&cbuf->cs);
    cbuf_insert_update_stats (cbuf, new_frame);
    cbuf_append_frame_crit (&cbuf->waiting, new_frame);
    LeaveCriticalSection(&cbuf->cs);
}

void
cbuf_insert_waiting (Cbuf* cbuf, Frame* new_frame)
{
    Frame* f;
    EnterCriticalSection(&cbuf->cs);
    f = cbuf->waiting.head;
    if (!f || new_frame->id > cbuf->waiting.tail->id) {
	/* Put at end of queue */
	cbuf_append_frame_crit (&cbuf->waiting, new_frame);
    } else if (new_frame->id <= cbuf->waiting.head->id) {
	/* Put at beginning of queue */
	cbuf_prepend_frame_crit (&cbuf->waiting, new_frame);
    } else {
	/* Search through queue for the right spot to insert */
	while (f) {
	    if (new_frame->id > f->id) {
		f = f->next;
	    } else {
		/* Put new_frame before f */
		new_frame->next = f->next;
		new_frame->prev = f;
		f->next = new_frame;
		new_frame->next->prev = new_frame;
		break;
	    }
	}
    }
    cbuf_insert_update_stats (cbuf, new_frame);
    LeaveCriticalSection(&cbuf->cs);
}

Frame*
cbuf_get_empty_frame (Cbuf* cbuf)
{
    Frame* empty_frame = 0;
    EnterCriticalSection(&cbuf->cs);
    if (cbuf->empty.head) {
	empty_frame = cbuf->empty.head;
	cbuf_unlink_frame_crit (&cbuf->empty, empty_frame);
    }
    LeaveCriticalSection(&cbuf->cs);
    if (empty_frame) {
	frame_clear (empty_frame);
    }
    return empty_frame;
}

static void
cbuf_insert_update_stats (Cbuf* cbuf, Frame* frame)
{
    if (frame->writable) {
	cbuf->writable ++;
    }
    if (frame->writable && !frame->written) {
	cbuf->waiting_unwritten ++;
    }
}

static void
cbuf_remove_update_stats (Cbuf* cbuf, Frame* frame)
{
    if (frame->writable && !frame->written) {
	cbuf->waiting_unwritten --;
	cbuf->dropped ++;
    }
}

#if defined (commentout)
/* Find a frame.  First, look in the empty queue.  If there is none,
   then get an unlocked frame from the waiting queue. */
Frame*
cbuf_get_any_frame (Cbuf* cbuf, int flush_dark)
{
    Frame* empty_frame = 0;
    Frame* oldest_bright = 0;
    EnterCriticalSection(&cbuf->cs);
    if (cbuf->empty.head) {
	empty_frame = cbuf->empty.head;
	cbuf_unlink_frame_crit (&cbuf->empty, empty_frame);
    } else {
	Frame* f = cbuf->waiting.head;
	while (f) {
	    if (cbuf_is_locked(cbuf,f)) {
		f = f->next;
	    } else {
		if (flush_dark) {
		    if (!oldest_bright) {
			oldest_bright = f;
		    }
		    if (!frame_is_dark(f)) {
			f = f->next;
			continue;
		    }
		}
		empty_frame = f;
		cbuf_remove_update_stats (cbuf, empty_frame);
		cbuf_unlink_frame_crit (&cbuf->waiting, empty_frame);
		break;
	    }
	}
	if (flush_dark && !empty_frame) {
	    empty_frame = oldest_bright;
	    cbuf_remove_update_stats (cbuf, empty_frame);
	    cbuf_unlink_frame_crit (&cbuf->waiting, empty_frame);
	    /* GCS ADD HERE: Set the global variable about most recently 
		dropped frame.  This is done so that I can display something 
	        to the user interface that frames are being dropped.  */
	}
    }
    LeaveCriticalSection(&cbuf->cs);
    if (empty_frame) {
	frame_clear (empty_frame);
    }
    return empty_frame;
}
#endif


/* Compute the priority of a frame.  We give a higher priority 
   for frames we prefer to keep in the buffer.
   Range is between 1 and 4.  See "frame discard algorithms" in ISE.odt. */
int
frame_priority (Frame* f)
{
    if (f->indico_state == INDICO_SHMEM_XRAY_ON) {
	if (f->writable && !f->written) {
	    return 4;
	}
	return 2;
    }
    /* Dark frames */
    if (f->writable && !f->written) {
	return 3;
    }
    return 1;
}

/* Find a frame.  First, look in the empty queue.  If there is none,
   then get an unlocked frame from the waiting queue. */
Frame*
cbuf_get_any_frame (Cbuf* cbuf, int flush_dark)
{
    Frame* frame_to_return = 0;
    Frame* oldest_bright = 0;
    int ftr_priority, this_priority;	

    EnterCriticalSection(&cbuf->cs);
    if (cbuf->empty.head) {
	frame_to_return = cbuf->empty.head;
	cbuf_unlink_frame_crit (&cbuf->empty, frame_to_return);
    } else {
	Frame* f = cbuf->waiting.head;
	while (f) {
	    if (!cbuf_is_locked(cbuf,f)) {
		if (!frame_to_return) {
		    frame_to_return = f;
		    ftr_priority = frame_priority (f);
		    if (ftr_priority == 1) {
			break;
		    }
		} else {
		    this_priority = frame_priority (f);
		    if (this_priority < ftr_priority) {
			frame_to_return = f;
			ftr_priority = this_priority;
			if (ftr_priority == 1) {
			    break;
			}
		    }
		}
	    }
	    f = f->next;
	}
	cbuf_remove_update_stats (cbuf, frame_to_return);
	cbuf_unlink_frame_crit (&cbuf->waiting, frame_to_return);
	/* GCS ADD HERE: Set the global variable about most recently 
	    dropped frame.  This is done so that I can display something 
	    to the user interface that frames are being dropped.  */
    }
    LeaveCriticalSection(&cbuf->cs);
    if (frame_to_return) {
	frame_clear (frame_to_return);
    }
    return frame_to_return;
}

void
cbuf_display_lock_release (Cbuf* cbuf)
{
    EnterCriticalSection(&cbuf->cs);
    cbuf_reset_display_lock_crit (cbuf);
    LeaveCriticalSection(&cbuf->cs);
}

Frame*
cbuf_display_lock_newest_frame (Cbuf* cbuf)
{
    EnterCriticalSection(&cbuf->cs);
    cbuf_reset_display_lock_crit (cbuf);
    cbuf->display_ptr = cbuf->waiting.tail;
    cbuf_set_display_lock_crit (cbuf);
    LeaveCriticalSection(&cbuf->cs);
    return cbuf->display_ptr;
}

Frame*
cbuf_display_lock_newest_bright (Cbuf* cbuf)
{
    Frame* f;
    EnterCriticalSection(&cbuf->cs);
    cbuf_reset_display_lock_crit (cbuf);
    f = cbuf->waiting.tail;
    while (f) {
	if (frame_is_dark(f)) {
	    f = f->prev;
	} else {
	    break;
	}
    }
    if (!f) f = cbuf->waiting.tail;
    cbuf->display_ptr = f;
    cbuf_set_display_lock_crit (cbuf);
    LeaveCriticalSection(&cbuf->cs);
    return cbuf->display_ptr;
}

Frame*
cbuf_display_lock_oldest_frame (Cbuf* cbuf)
{
    EnterCriticalSection(&cbuf->cs);
    cbuf_reset_display_lock_crit (cbuf);
    cbuf->display_ptr = cbuf->waiting.head;
    cbuf_set_display_lock_crit (cbuf);
    LeaveCriticalSection(&cbuf->cs);
    return cbuf->display_ptr;
}

/* This version of the function returns 0 is not a new one. */
Frame*
cbuf_display_lock_next_frame (Cbuf* cbuf)
{
    Frame* f = 0;
    EnterCriticalSection(&cbuf->cs);

    cbuf_reset_display_lock_crit (cbuf);
    if (cbuf->display_ptr) {
	/* Already called before. */
	if (cbuf->display_ptr->next) {
	    cbuf->display_ptr = cbuf->display_ptr->next;
	    f = cbuf->display_ptr;
	}
    } else {
        /* First time through */
	cbuf->display_ptr = cbuf->waiting.head;
	f = cbuf->display_ptr;
    }
    cbuf_set_display_lock_crit (cbuf);

    LeaveCriticalSection(&cbuf->cs);
    return f;
}

Frame* 
cbuf_display_lock_frame_by_idx (Cbuf* cbuf, int frame_no)
{
    int i;
    Frame* f = 0;
    EnterCriticalSection(&cbuf->cs);

    cbuf_reset_display_lock_crit (cbuf);
    f = cbuf->waiting.head;
    for (i = 0; i < frame_no; i++) {
	f = f->next;
    }
    cbuf->display_ptr = f;
    cbuf_set_display_lock_crit (cbuf);

    LeaveCriticalSection(&cbuf->cs);
    return f;
}

Frame* 
cbuf_display_lock_internal_grab (Cbuf* cbuf)
{
    Frame* f = 0;
    EnterCriticalSection(&cbuf->cs);
    if (cbuf->internal_grab_ptr == INTERNAL_GRAB_BEGIN) {
	f = cbuf->waiting.head;
    } else if (cbuf->internal_grab_ptr == INTERNAL_GRAB_END) {
	f = cbuf->waiting.tail;
    } else {
	f = cbuf->internal_grab_ptr->prev;
    }
    cbuf->display_ptr = f;
    cbuf_set_display_lock_crit (cbuf);

    LeaveCriticalSection(&cbuf->cs);
    return f;
}

void
cbuf_internal_grab_rewind (Cbuf* cbuf)
{
    EnterCriticalSection(&cbuf->cs);
    cbuf->internal_grab_ptr = INTERNAL_GRAB_BEGIN;
    LeaveCriticalSection(&cbuf->cs);
}

Frame*
cbuf_internal_grab_next_frame (Cbuf* cbuf)
{
    Frame* f;
    EnterCriticalSection(&cbuf->cs);
    if (cbuf->internal_grab_ptr == INTERNAL_GRAB_BEGIN) {
	cbuf->internal_grab_ptr = cbuf->waiting.head;
    } else if (cbuf->internal_grab_ptr) {
	cbuf->internal_grab_ptr = cbuf->internal_grab_ptr->next;
    }
    f = cbuf->internal_grab_ptr;
    LeaveCriticalSection(&cbuf->cs);
    return f;
}

void
cbuf_mark_frame_written (Cbuf* cbuf, Frame* frame)
{
    if (frame_needs_write (frame)) {
	cbuf->waiting_unwritten --;
	cbuf->write_ptr->written = 1;
    }
}

/* Return frame if writable frame exists */
Frame*
cbuf_get_next_writable_frame (Cbuf* cbuf)
{
    Frame* f = 0;
    int frame_is_writable = 0;

    EnterCriticalSection(&cbuf->cs);

    if (cbuf->write_ptr) {
	/* Already called before. */
	cbuf->write_ptr->write_lock = 0;
    } else {
        /* First time through */
	cbuf->write_ptr = cbuf->waiting.head;
	if (!cbuf->write_ptr) {
	    /* Queue is empty */
	    goto done;
	}
    }

    /* Search list for writable frame. If no writable frame is found, 
       the write_ptr is left at the tail frame and we return 0. */
    while (1) {
	if (frame_needs_write (cbuf->write_ptr)) {
	    cbuf->write_ptr->write_lock = 1;
	    f = cbuf->write_ptr;
	    break;
	} else if (cbuf->write_ptr->next) {
	    cbuf->write_ptr = cbuf->write_ptr->next;
	} else {
	    break;
	}
    }

done:
    LeaveCriticalSection(&cbuf->cs);
    return f;
}

void
cbuf_reset_queue (Cbuf* cbuf)
{
    Frame* f;
    EnterCriticalSection(&cbuf->cs);

    /* Clear ptrs, if possible */
    cbuf_reset_display_ptr_crit (cbuf);
    cbuf_reset_write_ptr_crit (cbuf);

    f = cbuf->waiting.head;
    while (f) {
	if (cbuf_is_locked(cbuf,f)) {
	    f = f->next;
	} else {
	    cbuf_unlink_frame_crit (&cbuf->waiting, f);
	    cbuf_append_frame_crit (&cbuf->empty, f);
	    f = cbuf->waiting.head;
	}
    }
    cbuf->writable = 0;
    cbuf->waiting_unwritten = 0;
    cbuf->dropped = 0;
    LeaveCriticalSection(&cbuf->cs);
}

unsigned long
cbuf_queuelen (Cbuf* cbuf, FrameQueue* queue)
{
    unsigned long len;
    EnterCriticalSection(&cbuf->cs);
    len = queue->queue_len;
    LeaveCriticalSection(&cbuf->cs);
    return len;
}

int
cbuf_shutdown (IseFramework* ig)
{
    cbuf_shutdown_queue (ig, &ig->cbuf[0]);
    cbuf_shutdown_queue (ig, &ig->cbuf[1]);
    return 0;
}

void
cbuf_shutdown_queue (IseFramework* ig, Cbuf* cbuf)
{
    unsigned long f;

    if (cbuf->num_frames == 0) {
	return;
    }

    debug_printf ("cbuf_shutdown_queue: cbuf = %p\n", cbuf);

    /* Release frames */
    for (f = 0; f < cbuf->num_frames; f++) {
	free ((void*) cbuf->frames[f].img);
    }

    /* Free memory */
    free (cbuf->frames);
    cbuf->num_frames = 0;
    cbuf->writable = 0;
    cbuf->waiting_unwritten = 0;
    cbuf->dropped = 0;

    /* Release mutex */
    DeleteCriticalSection(&cbuf->cs);
}
#endif /* commentout */
