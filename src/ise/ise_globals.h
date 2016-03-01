/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __ise_globals_h__
#define __ise_globals_h__

#ifndef MAXGREY
#define MAXGREY 16384            // 14-bit
#endif

#if defined (commentout)
#include "filewrite.h"
#include "ise_resize.h"
#include "indico_info.h"

typedef unsigned int _GLuint;    // == GLuint, but defined here
typedef struct __ShaderInfo ShaderInfo;

enum Ise_Panel_Select
{
	USE_PANEL_1,
	USE_PANEL_2,
	USE_BOTH_PANELS
};

enum Ise_Sync_Relays
{
	SYNC_RELAY_1,
	SYNC_RELAY_2,
	SYNC_RELAY_NEITHER
};

enum Ise_Program_State
{
	PROGRAM_STATE_UNKNOWN = 0,
	PROGRAM_STATE_STOPPED,
	PROGRAM_STATE_GRABBING,
	PROGRAM_STATE_RECORDING,
	PROGRAM_STATE_REPLAYING
};

struct WinVars_Type {
#ifdef _WIN32
    HWND hwin;	    /* The parent window */
    HWND hpwin;	    /* The picture subwindow */
    HDC hdc;	    /* HDC for parent window */
    HDC hpdc;	    /* HDC for picture subwindow */
    HBITMAP hbm;
    int bm_h;
    int bm_w;
    int bm_rowlen;
    unsigned char* bmb;
    unsigned long histogram[MAXGREY];
    unsigned char lut[MAXGREY];
    Resize_Data rd;

    int is_zoomed;
    float zoomx;
    float zoomy;
    float panx;
    float pany;
    int findtrack_overlay_flag;
    float findtrack_overlay_x;
    float findtrack_overlay_y;

    HGLRC hglrc;
    _GLuint texture_name;
    ShaderInfo* si;
#endif
};
typedef struct WinVars_Type WinVars;

struct Globals_Type {
    /* Program logic */
    IseFramework ig;
    int quit;
    char notify[2];

    /* Frame grabber */
    int have_matrox_hardware;
    int have_bitflow_hardware;

    /* Communication with indico process */
    Indico_Info indico_info;

    /* Image panel config */
    enum Ise_Panel_Select panel_select;
    int num_panels;
    enum Ise_Sync_Relays sync_relays;

    /* Program config */
    int buffer_num_frames;

    /* Program state */
    enum Ise_Program_State program_state;
    int hold_bright_frame;
    int auto_window_level;
    int drop_dark_frames;
    int is_writing;
    int tracking_flag;
    int gating_flag;
    int loadfrom_file;

    /* Display vars */
#ifdef _WIN32
    HINSTANCE hinst;
    HACCEL hAccelTable;
    HDC hdc_mem;
    WinVars win[2];
#endif
    int screen_w;
    int screen_h;
    int fullscreen_client_w;
    int fullscreen_client_h;
    int color_depth;
};
typedef struct Globals_Type Globals;

extern Globals globals;

void init_globals (void);
void save_globals (void);
#endif

#endif
