/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __ISE_GL_H__
#define __ISE_GL_H__

void init_gl (void);
void blt_frame_gl (int imager_no, Frame* frame, void* buf, int image_source);
void black_frame_gl (int idx);
void resize_gl_window (int idx);
void gl_update_lut (int idx, unsigned short bot, unsigned short top);
void gl_zoom_at_pos (int idx, int x, int y);
void gl_set_findtrack_overlay_pos (int idx, int x, int y);
void gl_get_image_pos (int idx, int x, int y, int* im_x, int* im_y);
void gl_image_to_video (float* vx, float* vy, int ix, int iy);
void gl_video_to_image (float* ix, float* iy, float vx, float vy);
void gl_client_to_image (int idx, float* ix, float* iy, int cx, int cy);
void gl_client_to_video (int idx, float* vx, float* vy, int cx, int cy);

#endif
