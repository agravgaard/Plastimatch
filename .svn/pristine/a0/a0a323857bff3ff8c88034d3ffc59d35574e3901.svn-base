/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ise_resize_h__
#define __ise_resize_h__

#define MAX_WINDOWS 20

#define BIND_TOP	0x01
#define BIND_LEFT	0x02
#define BIND_RIGHT	0x04
#define BIND_BOT	0x08
#define BIND_ALL	0x0F
#define BIND_UNKNOWN	0x00

struct Resize_Item_Type {
    int dialog_item;
    int bind_type;
    long init_l;    /* left */
    long init_r;    /* right */
    long init_w;    /* width */
    long init_t;    /* top */
    long init_b;    /* bot */
    long init_h;    /* height */
};
typedef struct Resize_Item_Type Resize_Item;

struct Resize_Data_Type {
#ifdef _WIN32
    HWND hwnd;
    RECT initial_parent_rect;
#endif
    int num_items;
    Resize_Item* rilist;
};
typedef struct Resize_Data_Type Resize_Data;

#ifdef _WIN32
void ise_resize_init (Resize_Data*rd, HWND hwnd);
void ise_resize_add (Resize_Data* rd, int control_id, unsigned int bind_type);
void ise_resize_freeze (Resize_Data* rd);
void ise_resize_on_event (Resize_Data* rd, HWND hwnd);
#endif

#endif
