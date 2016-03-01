/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
/* -------------------------------------------------------------------------*
   This file include "misc framework routines".  
 * -------------------------------------------------------------------------*/
#include "ise.h"
#include "cbuf.h"
#include "filewrite.h"
#include "debug.h"
#include "ise_config.h"
#include "ise_framework.h"


/*#define ip0_client "192.168.1.2"
#define ip0_server "192.168.1.3"
#define ip1_client "192.168.1.4"
#define ip1_server "192.168.1.5"
*/

/* ---------------------------------------------------------------------------- *
    Global functions
 * ---------------------------------------------------------------------------- */
void
ise_fluoro_notify_callback (int imager_no)
{
    /* GCS - There may be a race condition here */
#if defined (commentout)
    if (globals.notify[imager_no]) {
	debug_printf ("Dropped display imager %d\n",imager_no);
    }
#endif
    globals.notify[imager_no] = 1;
}

void
ise_fluoro_start_grabbing (void)
{
    int rc;
    rc = ise_grab_start_capture_threads (&globals.ig, ise_fluoro_notify_callback);
    if (rc) {
	printf ("Error: irisgrab_start_capture_threads() failed\n");
        exit(1);
    }
}

void
ise_fluoro_stop_grabbing (void)
{
    ise_grab_continuous_stop (&globals.ig);
}

Frame*
ise_fluoro_get_drawable_grabbing (int idx)
{
    Frame* f;
    if (globals.ig.image_source == ISE_IMAGE_SOURCE_INTERNAL_FLUORO)
	f = cbuf_display_lock_internal_grab (&globals.ig.cbuf[idx]);
    else if (globals.hold_bright_frame) {
	f = cbuf_display_lock_newest_bright (&globals.ig.cbuf[idx]);
    } else {
	f = cbuf_display_lock_newest_frame (&globals.ig.cbuf[idx]);
    }
    return f;
}

Frame*
ise_fluoro_get_drawable_stopped (int idx)
{
    Frame* f;
    f = globals.ig.cbuf[idx].display_ptr;
    return f;
}

Frame*
ise_fluoro_get_drawable_replaying (int idx)
{
    Frame* f;
    f = ise_fluoro_get_next (idx);
    if (f) {
	globals.notify[idx] = 1;
    } else {
	f = cbuf_display_lock_newest_frame (&globals.ig.cbuf[idx]);
    }
    return f;
}

Frame*
ise_fluoro_get_next (int idx)
{
    return cbuf_display_lock_next_frame (&globals.ig.cbuf[idx]);
}

void
ise_fluoro_display_frame_no (int frame_no)
{
    int idx;
    for (idx = 0; idx < globals.ig.num_idx; idx++) {
	cbuf_display_lock_frame_by_idx (&globals.ig.cbuf[idx], frame_no);
	globals.notify[idx] = 1;
    }
}

void
ise_fluoro_rewind_display (void)
{
    int idx;
    for (idx = 0; idx < globals.ig.num_idx; idx++) {
	cbuf_display_lock_oldest_frame (&globals.ig.cbuf[idx]);
	globals.notify[idx] = 1;
    }
}

void
ise_fluoro_reset_queue (void)
{
    int idx;
    for (idx = 0; idx < globals.ig.num_idx; idx++) {
	cbuf_reset_queue (&globals.ig.cbuf[idx]);
	/* GCS FIX: Need some way of getting a blank frame */
	//globals.notify[i] = 1;
    }
}

Frame*
ise_fluoro_get_empty_frame (int idx)
{
    return cbuf_get_empty_frame (&globals.ig.cbuf[idx]);
}

void
ise_fluoro_insert_frame (int idx, Frame* f)
{
    cbuf_insert_waiting (&globals.ig.cbuf[idx], f);
}

void
ise_fluoro_display_lock_release (int idx)
{
    cbuf_display_lock_release (&globals.ig.cbuf[idx]);
}

