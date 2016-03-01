/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xpm_h_
#define _xpm_h_

#include "plmbase_config.h"

class Xpm_brush_private;
class Xpm_canvas_private;

enum xpm_brushes { XPM_BOX, XPM_CIRCLE };

class PLMBASE_API Xpm_brush {
public:
    Xpm_brush ();
    ~Xpm_brush ();
    void set_type (xpm_brushes type);
    void set_color (char color);
    void set_pos (int x, int y);
    void set_width (int width);
    void set_height (int height);
    void set_x (int x);
    void set_y (int y);

    char get_color ();
    xpm_brushes get_type ();
    int get_width ();
    int get_height ();
    int get_x ();
    int get_y ();

    void inc_x (int x);
    void inc_y (int y);
private:
    Xpm_brush_private *d_ptr;
};

class PLMBASE_API Xpm_canvas {
public:
    Xpm_canvas (int width, int height, int cpp);
    ~Xpm_canvas ();

    void add_color (char color_code, int color);
    int draw (Xpm_brush* brush);
    void prime (char color_code);
    int remove_color (char color_code);
    void write (char* xpm_file);
private:
    Xpm_canvas_private *d_ptr;
};

#endif
