/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
//#pragma once
#ifndef _dips_panel_h_
#define _dips_panel_h_

#define HIRES_IMAGE_HEIGHT 3200
#define HIRES_IMAGE_WIDTH 2304

class Dips_panel
{
public:
    Dips_panel ();
    ~Dips_panel ();
    void open_panel (int panel_no, int height, int width);
    void poll_dummy (void);
    void wait_for_dips (void);
    void send_image (void);

public:
    int panel_no;
    int height;
    int width;     /* width == x, height == y */    
    struct PANEL *panelp;
    unsigned short* pixelp;
};

#endif