/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ISE_GDI_H__
#define __ISE_GDI_H__

void process_pending_events (void);
void blt_frame (int imager_no, void* frame, void* buf);
void register_dialog_class (HINSTANCE hInstance);
void check_display (HINSTANCE hInstance);
BOOL create_windows (HINSTANCE hInstance, int nCmdShow);
void init_lut (void);
void init_frame_slider (void);
void init_dib_sections (void);
void exit_message (char* message);
void get_picture_window_rect (int idx, RECT* rect);
void gdi_update_frame_slider (int idx);
void gdi_update_lut_slider (int idx, unsigned short bot, unsigned short top);
void update_queue_status (int idx);

#endif
