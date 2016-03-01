/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ise_h__
#define __ise_h__

#include "ise_structs.h"
#include "ise_error.h"
#include "ise_globals.h"
#include "ise_version.h"
#include "ise_ontrak.h"
#include "ise_igpax.h"
#include "ise_error.h"
#include "cbuf.h"

Frame* ise_fluoro_get_next (int imager_no);
Frame* ise_fluoro_get_empty_frame (int idx);
void ise_fluoro_insert_frame (int idx, Frame* f);
void ise_fluoro_rewind_display (void);
void ise_fluoro_display_frame_no (int frame_no);
void ise_fluoro_start_grabbing (void);
void ise_fluoro_stop_grabbing (void);
void ise_fluoro_reset_queue (void);
void ise_fluoro_display_lock_release (int imager_no);
Frame* ise_fluoro_get_drawable_grabbing (int idx);
Frame* ise_fluoro_get_drawable_replaying (int idx);
Frame* ise_fluoro_get_drawable_stopped (int idx);

#endif
