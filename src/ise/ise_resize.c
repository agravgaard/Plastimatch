/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include <windows.h>
#include "ise_resize.h"

void
ise_resize_init (Resize_Data*rd, HWND hwnd)
{
    rd->hwnd = hwnd;
    rd->num_items = 0;
    rd->rilist = 0;
}

Resize_Data*
ise_resize_create (HWND hwnd)
{
    Resize_Data* rd = (Resize_Data*) malloc (sizeof(Resize_Data));
    ise_resize_init (rd, hwnd);
    return rd;
}

void
ise_resize_destroy (Resize_Data* rd)
{
    free (rd);
}

void
ise_resize_add (Resize_Data* rd, int control_id, unsigned int bind_type)
{
    int i = rd->num_items ++;
    rd->rilist = realloc(rd->rilist, rd->num_items * sizeof(Resize_Item));
    rd->rilist[i].dialog_item = control_id;
    rd->rilist[i].bind_type = bind_type;
}

static void
ise_resize_freeze_item (Resize_Item* ri, HWND hwnd)
{
    HWND cwnd;
    RECT parent_rect;
    RECT item_rect;

    cwnd = GetDlgItem (hwnd, ri->dialog_item);
    if (!cwnd) {
	exit (-1);
    }
    if (!GetWindowRect (cwnd, &item_rect)) {
	exit (-1);
    }
    /* This is a hack, but it transforms the RECT into coordinates of hwnd */
    ScreenToClient (hwnd, (POINT*) &item_rect);
    ScreenToClient (hwnd, (POINT*) &item_rect.right);

    /* Set l,t,w,h */
    ri->init_l = item_rect.left;
    ri->init_w = item_rect.right - item_rect.left;
    ri->init_t = item_rect.top;
    ri->init_h = item_rect.bottom - item_rect.top;

    /* Finally, get distance from right/bot of parent to right/bot of child */
    GetClientRect (hwnd, &parent_rect);
    ri->init_r = parent_rect.right - item_rect.right;
    ri->init_b = parent_rect.bottom - item_rect.bottom;
}

void
ise_resize_freeze (Resize_Data* rd)
{
    int i;

    for (i = 0; i < rd->num_items; i++) {
	Resize_Item* ri = &rd->rilist[i];
	ise_resize_freeze_item (ri, rd->hwnd);
    }
}

void
ise_resize_on_event (Resize_Data* rd, HWND hwnd)
{
    int i;
    RECT full_rect;

    if (hwnd != rd->hwnd) return;
    GetClientRect (rd->hwnd, &full_rect);
    for (i = 0; i < rd->num_items; i++) {
	int x, y, w, h;
	Resize_Item* ri = &rd->rilist[i];
	if (ri->bind_type & BIND_TOP) {
	    y = ri->init_t;
	}
	if (ri->bind_type & BIND_LEFT) {
	    x = ri->init_l;
	}
	if (ri->bind_type & BIND_BOT) {
	    h = full_rect.bottom - full_rect.top - y - ri->init_b;
	} else {
	    h = ri->init_h;
	}
	if (ri->bind_type & BIND_RIGHT) {
	    w = full_rect.right - full_rect.left - x - ri->init_r;
	} else {
	    w = ri->init_w;
	}
	MoveWindow (GetDlgItem (hwnd, ri->dialog_item), x, y, w, h, TRUE);
    }
}
