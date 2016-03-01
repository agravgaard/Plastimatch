/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include "ise.h"
#include "ise_rsc.h"
#include "ise_framework.h"
#include "debug.h"
#include "ise_gdi.h"
#include "ise_gl.h"
#include "ise_config.h"
#include "fileload.h"

#include <commctrl.h>

#define MAX_LOADSTRING 100

/* ---------------------------------------------------------------------------- *
    Global variables
 * ---------------------------------------------------------------------------- */
TCHAR dialog_window_class[MAX_LOADSTRING];
DLGTEMPLATE* dialog_template;

/* ---------------------------------------------------------------------------- *
    Function declarations
 * ---------------------------------------------------------------------------- */
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK about_callback (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK source_settings_callback (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dialog_callback (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void adjust_dib_sections (void);
static void highlight_dialog_buttons (void);
static void highlight_dialog_buttons_win (HWND hwnd);
void update_wl_status (int idx, unsigned short bot, unsigned short top);

/* ---------------------------------------------------------------------------- *
    Global functions
 * ---------------------------------------------------------------------------- */
void
exit_message (char* message)
{
    exit (1);
}

void
check_display (HINSTANCE hInstance)
{
    HDC hDCGlobal = GetDC (NULL);
    INT iRasterCaps;

    iRasterCaps = GetDeviceCaps (hDCGlobal, RASTERCAPS);
    if (iRasterCaps & RC_PALETTE) {
	exit_message ("Error: display has palette");
    }

    globals.color_depth = GetDeviceCaps (hDCGlobal, BITSPIXEL);
    ReleaseDC (NULL, hDCGlobal);
  
    globals.screen_w = GetSystemMetrics (SM_CXSCREEN);
    globals.screen_h = GetSystemMetrics (SM_CYSCREEN);
    globals.fullscreen_client_w = (INT) GetSystemMetrics (SM_CXFULLSCREEN);
    globals.fullscreen_client_h = (INT) GetSystemMetrics (SM_CYFULLSCREEN);
}

int
point_in_rect (RECT rect, int x, int y)
{
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void
register_dialog_class (HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    ATOM atom;
    HRSRC hrsrc;
    HGLOBAL hglb;

    wcex.cbSize = sizeof(WNDCLASSEX); 
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC) dialog_callback;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = DLGWINDOWEXTRA;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_ISE);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCSTR)IDC_ISE;
    wcex.lpszClassName  = "SuperSlik";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
    atom = RegisterClassEx(&wcex);

    hrsrc = FindResource(NULL, (LPCTSTR) IDD_SUPER_DIALOG, RT_DIALOG); 
    hglb = LoadResource(hInstance, hrsrc); 
    dialog_template = LockResource(hglb); 
} 

void
init_frame_slider (void)
{
    int i;
    for (i = 0; i < globals.num_panels; i++) {
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_FRAME, TBM_SETRANGEMIN, 
	    FALSE, 0);
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_FRAME, TBM_SETRANGEMAX, 
	    FALSE, 0);
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_FRAME, TBM_SETPOS, 
	    TRUE, 0);
    }
}

void
gdi_update_frame_slider (int idx)
{
    int oldpos;

    /* GCS: assignment from queue_len is atomic, no need to lock */
    /* GCS: but, note race condition on the notify flag. */
    int newmax = globals.ig.cbuf[idx].waiting.queue_len - 1;
    if (newmax < 0) newmax = 0;
    oldpos = SendDlgItemMessage (globals.win[idx].hwin, 
	IDC_SLIDER_FRAME, TBM_GETPOS, 0, 0);
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_FRAME, 
	TBM_SETRANGEMAX, TRUE, newmax);
    switch (globals.program_state) {
    case PROGRAM_STATE_STOPPED:
	if (oldpos > newmax) {
	    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_FRAME, 
		TBM_SETPOS, TRUE, newmax);
	}
	break;
    case PROGRAM_STATE_GRABBING:
    case PROGRAM_STATE_RECORDING:
	SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_FRAME, 
	    TBM_SETPOS, TRUE, newmax);
	break;
    case PROGRAM_STATE_REPLAYING:
	if (oldpos < newmax) {
	    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_FRAME, 
		TBM_SETPOS, TRUE, oldpos+1);
	}
	break;
    }
}

void
gdi_rewind_frame_slider (int idx)
{
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_FRAME, 
	TBM_SETPOS, TRUE, 0);
}

void
init_lut (void)
{
    int i;
    for (i = 0; i < globals.num_panels; i++) {
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_BOT, TBM_SETRANGEMIN, 
	    FALSE, 0);
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_BOT, TBM_SETRANGEMAX, 
	    FALSE, MAXGREY-1);
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_BOT, TBM_SETPOS, 
	    TRUE, 0);
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_TOP, TBM_SETRANGEMIN, 
	    FALSE, 0);
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_TOP, TBM_SETRANGEMAX, 
	    FALSE, MAXGREY-1);
	SendDlgItemMessage (globals.win[i].hwin, IDC_SLIDER_TOP, TBM_SETPOS, 
	    TRUE, MAXGREY-1);
    }
}

void
gdi_update_lut_slider (int idx, unsigned short bot, unsigned short top)
{
    unsigned short smin, smax;

#define BOT_OFFSET 20
#define TOP_OFFSET 40

    if (bot < BOT_OFFSET) {
	smin = 0;
    } else {
	smin = bot - BOT_OFFSET;
    }
    if (top + TOP_OFFSET > MAXGREY-1) {
	smax = MAXGREY-1;
    } else {
	smax = top + TOP_OFFSET;
    }
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_BOT, TBM_SETRANGEMIN, 
	FALSE, smin);
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_BOT, TBM_SETRANGEMAX, 
	FALSE, smax);
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_BOT, TBM_SETPOS, 
	TRUE, bot);
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_TOP, TBM_SETRANGEMIN, 
	FALSE, smin);
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_TOP, TBM_SETRANGEMAX, 
	FALSE, smax);
    SendDlgItemMessage (globals.win[idx].hwin, IDC_SLIDER_TOP, TBM_SETPOS, 
	TRUE, top);
    update_wl_status (idx, smin, smax);
}

int
hwnd_to_idx (HWND hwnd)
{
    if (hwnd == globals.win[0].hwin) {
	return 0;
    } else {
	return 1;
    }
}

int
idx_to_panel_no (int idx)
{
    switch (globals.panel_select) {
    case USE_PANEL_1:
	return 1;
    case USE_PANEL_2:
	return 2;
    default:
    case USE_BOTH_PANELS:
	return idx+1;
    }
}

void
update_lut (HWND hwnd, long bot, long top)
{
    int idx;

    idx = hwnd_to_idx (hwnd);
    gl_update_lut (idx, (unsigned short) bot, (unsigned short) top);
}

void
update_queue_status (int idx)
{
    char status_buf[256];

    sprintf (status_buf, "Q%d: %3d %3d %3d %3d",
	idx_to_panel_no (idx), 
	globals.ig.cbuf[idx].num_frames,
	globals.ig.cbuf[idx].writable,
	globals.ig.cbuf[idx].waiting_unwritten,
	globals.ig.cbuf[idx].dropped);
    SetWindowText (GetDlgItem (globals.win[idx].hwin, IDC_QUEUE_STATUS),
	status_buf);
}

void
update_wl_status (int idx, unsigned short bot, unsigned short top)
{
    char status_buf[256];
    sprintf (status_buf, "WL: (%5d %5d)", bot, top);
    SetWindowText (GetDlgItem (globals.win[idx].hwin, IDC_WL_STATUS),
	status_buf);
}

/* The hwnd used to be that of the parent, now it's the picture window */
void
get_picture_window_rect_old (HWND hwnd, RECT* rect)
{
    // HWND cwnd = GetDlgItem (hwnd, IDC_PICTURE_WINDOW);
    //GetWindowRect (cwnd, rect);
    GetWindowRect (hwnd, rect);
    /* This is a hack, but it transforms the RECT into coordinates of hwnd */
    ScreenToClient (hwnd, (POINT*) rect);
    ScreenToClient (hwnd, (POINT*) &rect->right);
}

void
get_picture_window_rect (int idx, RECT* rect)
{
    HWND hpwnd = globals.win[idx].hpwin;    /* Picture window */
    HWND hwnd = globals.win[idx].hwin;	    /* Parent */

    GetWindowRect (hpwnd, rect);
    ScreenToClient (hwnd, (POINT*) rect);
    ScreenToClient (hwnd, (POINT*) &rect->right);
}

/* Return 1 if the (mouse event) was in the picture window */
int
in_picture_window_rect (int idx, int x, int y)
{
    RECT rect;
    HWND hpwnd = globals.win[idx].hpwin;    /* Picture window */
    HWND hwnd = globals.win[idx].hwin;	    /* Parent */

    GetWindowRect (hpwnd, &rect);
    ScreenToClient (hwnd, (POINT*) &rect);
    ScreenToClient (hwnd, (POINT*) &rect.right);

    if (x > rect.left && x < rect.right && y > rect.top && y < rect.bottom)
    {
	return 1;
    } else {
	return 0;
    }
}

void
do_resize (HWND hwnd)
{
    int i;
    for (i = 0; i < globals.num_panels; i++) {
	Resize_Data* rd = &globals.win[i].rd;
	ise_resize_on_event (rd, hwnd);
    }
}

#if defined (commentout)
void
create_button (HINSTANCE hInstance, HWND hwnd_parent)
{
    HWND hwndButton;
    hwndButton = CreateWindow( 
    "BUTTON",   // predefined class 
    "OK",       // button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles 
 
    // Size and position values are given explicitly, because 
    // the CW_USEDEFAULT constant gives zero values for buttons. 
    10,         // starting x position 
    10,         // starting y position 
    100,        // button width 
    100,        // button height 
    hwnd_parent,       // parent window 
    NULL,       // No menu 
    (HINSTANCE) GetWindowLong(hwnd_parent, GWL_HINSTANCE), 
    NULL);      // pointer not needed 
}
#endif

void
init_toolbars (HINSTANCE hInstance, int win_no, int nCmdShow)
{
#if defined (commentout)
    HWND toolbarwin;

    toolbarwin = CreateToolbarEx(
    globals.win[w].hwin,	
    WS_CHILD,	
    IDR_TOOLBAR1, 	
    2, 	
    hInstance,
    IDB_BITMAP1,
    lpButtons, 	
    int iNumButtons, 	
    int dxButton, 	
    int dyButton, 	
    int dxBitmap, 	
    int dyBitmap, 	
    UINT uStructSize	
);
#endif

    /* Waiting for ethernet connection... */
}

void
init_window (HINSTANCE hInstance, int win_no, int nCmdShow)
{
    int w = win_no;

    Resize_Data* rd = &globals.win[w].rd;

    globals.win[w].hwin = CreateDialogIndirect (hInstance,
		    dialog_template,
		    (HWND) NULL,
		    dialog_callback);
    ShowWindow (globals.win[w].hwin, nCmdShow);
    UpdateWindow (globals.win[w].hwin);
    ise_resize_init (rd, globals.win[w].hwin);
    ise_resize_add (rd, IDC_PICTURE_WINDOW, BIND_TOP | BIND_BOT | BIND_RIGHT | BIND_LEFT);
    ise_resize_add (rd, IDC_SLIDER_BOT, BIND_TOP | BIND_BOT | BIND_LEFT);
    ise_resize_add (rd, IDC_SLIDER_TOP, BIND_TOP | BIND_BOT | BIND_LEFT);
    ise_resize_freeze (rd);
    globals.win[w].hpwin = GetDlgItem (globals.win[w].hwin, IDC_PICTURE_WINDOW);
    globals.win[w].hdc = GetDC(globals.win[w].hwin);
    globals.win[w].hpdc = GetDC(globals.win[w].hpwin);
    globals.win[w].is_zoomed;
    globals.win[w].zoomx = 1.0;
    globals.win[w].zoomy = 1.0;
    globals.win[w].panx = 0.0;
    globals.win[w].pany = 0.0;
    globals.win[w].findtrack_overlay_flag = 0;
    globals.win[w].findtrack_overlay_x = 0.0;
    globals.win[w].findtrack_overlay_y = 0.0;
}

BOOL
create_windows (HINSTANCE hInstance, int nCmdShow)
{
    int i;
    RECT client_size;
    DWORD dwStyle;

    dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX 
		| WS_SIZEBOX | WS_MAXIMIZE;

    client_size.left = 0;
    client_size.top = 0;
    client_size.right = 100 - 1;
    client_size.bottom = 100 - 1;
    /* AdjustWindowRect (&client_size, dwStyle, TRUE); */

    for (i = 0; i < globals.num_panels; i++) {
	init_window (hInstance, i, nCmdShow);
    }

    return TRUE;
}

static void
highlight_source_menu (HWND hwnd)
{
    switch (globals.program_state) 
    {
        case PROGRAM_STATE_STOPPED:
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_MATROX, MF_BYCOMMAND | MF_ENABLED);
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_BITFLOW, MF_BYCOMMAND | MF_ENABLED);
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_SYNTHETIC, MF_BYCOMMAND | MF_ENABLED);
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_INTERNAL, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_FILE, MF_BYCOMMAND | MF_ENABLED);
	    break;
        case PROGRAM_STATE_GRABBING:
        case PROGRAM_STATE_RECORDING:
        case PROGRAM_STATE_REPLAYING:
        default:
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_MATROX, MF_BYCOMMAND | MF_GRAYED);
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_BITFLOW, MF_BYCOMMAND | MF_GRAYED);
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_SYNTHETIC, MF_BYCOMMAND | MF_GRAYED);
	    EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_INTERNAL, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem (GetMenu(hwnd), IDM_SOURCE_FILE, MF_BYCOMMAND | MF_GRAYED);
	    break;
    }

    switch (globals.ig.image_source) 
    {
        case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_MATROX, MF_BYCOMMAND | MF_CHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_BITFLOW, MF_BYCOMMAND | MF_UNCHECKED); 
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_SYNTHETIC, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_INTERNAL, MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_FILE, MF_BYCOMMAND | MF_UNCHECKED);
	    break;
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_MATROX, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_BITFLOW, MF_BYCOMMAND | MF_CHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_SYNTHETIC, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_INTERNAL, MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_FILE, MF_BYCOMMAND | MF_UNCHECKED);
	    break;
        case ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_MATROX, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_BITFLOW, MF_BYCOMMAND | MF_UNCHECKED); 
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_SYNTHETIC, MF_BYCOMMAND | MF_CHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_INTERNAL, MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_FILE, MF_BYCOMMAND | MF_UNCHECKED);
	    break;
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_MATROX, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_BITFLOW, MF_BYCOMMAND | MF_UNCHECKED); 
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_SYNTHETIC, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_INTERNAL, MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_FILE, MF_BYCOMMAND | MF_CHECKED);
	    break;
        case ISE_IMAGE_SOURCE_INTERNAL_FLUORO:
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_MATROX, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_BITFLOW, MF_BYCOMMAND | MF_UNCHECKED); 
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_SYNTHETIC, MF_BYCOMMAND | MF_UNCHECKED);
	    CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_INTERNAL, MF_BYCOMMAND | MF_CHECKED);
            CheckMenuItem (GetMenu(hwnd), IDM_SOURCE_FILE, MF_BYCOMMAND | MF_UNCHECKED);
	    break;
        default:
	    break;
    }
}

static void
set_button_state (HWND hwnd, int dlg_item_id, BOOL pushed)
{
    //SendDlgItemMessage (hwnd, dlg_item_id, BM_SETSTATE, pushed, 0);
    SendDlgItemMessage (hwnd, dlg_item_id, BM_SETCHECK, 
			pushed ? BST_CHECKED : BST_UNCHECKED, 0);
    UpdateWindow (GetDlgItem(hwnd, dlg_item_id));
}

static void
highlight_dialog_buttons_win (HWND hwnd)
{
    switch (globals.program_state) {
    case PROGRAM_STATE_STOPPED:
	set_button_state (hwnd, IDC_BUTTON_STOP, TRUE);
	set_button_state (hwnd, IDC_BUTTON_GRAB, FALSE);
	set_button_state (hwnd, IDC_BUTTON_REC, FALSE);
	set_button_state (hwnd, IDC_BUTTON_REPLAY, FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REWBEG), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_PAUSE), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REPLAY), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_FF), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_FRAME), TRUE);
	break;
    case PROGRAM_STATE_GRABBING:
	set_button_state (hwnd, IDC_BUTTON_STOP, FALSE);
	set_button_state (hwnd, IDC_BUTTON_GRAB, TRUE);
	set_button_state (hwnd, IDC_BUTTON_REC, FALSE);
	set_button_state (hwnd, IDC_BUTTON_REPLAY, FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REWBEG), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_PAUSE), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REPLAY), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_FF), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_FRAME), FALSE);
	break;
    case PROGRAM_STATE_RECORDING:
	set_button_state (hwnd, IDC_BUTTON_STOP, FALSE);
	set_button_state (hwnd, IDC_BUTTON_GRAB, TRUE);
	set_button_state (hwnd, IDC_BUTTON_REC, TRUE);
	set_button_state (hwnd, IDC_BUTTON_REPLAY, FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REWBEG), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_PAUSE), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REPLAY), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_FF), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_FRAME), FALSE);
	break;
    case PROGRAM_STATE_REPLAYING:
	set_button_state (hwnd, IDC_BUTTON_STOP, FALSE);
	set_button_state (hwnd, IDC_BUTTON_GRAB, FALSE);
	set_button_state (hwnd, IDC_BUTTON_REC, FALSE);
	set_button_state (hwnd, IDC_BUTTON_REPLAY, TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REWBEG), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_PAUSE), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_REPLAY), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_BUTTON_FF), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_FRAME), TRUE);
	break;
    }

    highlight_source_menu (hwnd);

    if (globals.ig.write_dark_flag) {
	CheckMenuItem (GetMenu(hwnd), IDM_OPTIONS_WRITE_DARK, MF_BYCOMMAND | MF_CHECKED);
    } else {
	CheckMenuItem (GetMenu(hwnd), IDM_OPTIONS_WRITE_DARK, MF_BYCOMMAND | MF_UNCHECKED);
    }

    if (globals.auto_window_level) {
	set_button_state (hwnd, IDC_BUTTON_AWL, TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_TOP), FALSE);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_BOT), FALSE);
    } else {
	set_button_state (hwnd, IDC_BUTTON_AWL, FALSE);
	gdi_update_lut_slider (hwnd_to_idx(hwnd), 0, MAXGREY-1);
	update_lut (hwnd, 0, MAXGREY-1);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_TOP), TRUE);
	EnableWindow (GetDlgItem(hwnd,IDC_SLIDER_BOT), TRUE);
    }

    if (globals.hold_bright_frame) {
	CheckMenuItem (GetMenu(hwnd), IDM_OPTIONS_HOLD_BRIGHT, MF_BYCOMMAND | MF_CHECKED);
    } else {
	CheckMenuItem (GetMenu(hwnd), IDM_OPTIONS_HOLD_BRIGHT, MF_BYCOMMAND | MF_UNCHECKED);
    }

    if (globals.drop_dark_frames) {
	CheckMenuItem (GetMenu(hwnd), IDM_OPTIONS_DROP_DARK, MF_BYCOMMAND | MF_CHECKED);
    } else {
	CheckMenuItem (GetMenu(hwnd), IDM_OPTIONS_DROP_DARK, MF_BYCOMMAND | MF_UNCHECKED);
    }

    if (globals.tracking_flag) {
	set_button_state (hwnd, IDC_BUTTON_TRACK, TRUE);
    } else {
	set_button_state (hwnd, IDC_BUTTON_TRACK, FALSE);
    }

    if (globals.gating_flag) {
	set_button_state (hwnd, IDC_BUTTON_GATE, TRUE);
    } else {
	set_button_state (hwnd, IDC_BUTTON_GATE, FALSE);
    }
    UpdateWindow (hwnd);
}

static void
highlight_dialog_buttons (void)
{
    int idx;
    for (idx = 0; idx < globals.num_panels; idx++) {
	highlight_dialog_buttons_win (globals.win[idx].hwin);
    }
}

static void
handle_button_reset ()
{
    int idx;
    if (globals.program_state == PROGRAM_STATE_GRABBING 
	|| globals.program_state == PROGRAM_STATE_RECORDING)
    {
	ise_fluoro_stop_grabbing ();
    }
    globals.ig.write_flag = 0;
    ise_fluoro_reset_queue ();
    for (idx = 0; idx < globals.num_panels; idx++) {
	gdi_update_frame_slider (idx);
	gdi_rewind_frame_slider (idx);
    }
    globals.program_state = PROGRAM_STATE_STOPPED;
}

static void
handle_button_stop ()
{
    if (globals.program_state == PROGRAM_STATE_GRABBING 
	|| globals.program_state == PROGRAM_STATE_RECORDING)
    {
	ise_fluoro_stop_grabbing ();
    }
    globals.ig.write_flag = 0;
    globals.program_state = PROGRAM_STATE_STOPPED;
}

static void
handle_button_grab ()
{
    if (globals.program_state == PROGRAM_STATE_STOPPED) {
	ise_fluoro_start_grabbing ();
	globals.program_state = PROGRAM_STATE_GRABBING;
    } else if (globals.program_state == PROGRAM_STATE_RECORDING) {
	globals.ig.write_flag = 0;
	globals.program_state = PROGRAM_STATE_GRABBING;
    }
}

static void
handle_button_rec ()
{
    if (globals.program_state == PROGRAM_STATE_STOPPED) {
	ise_fluoro_start_grabbing ();
	globals.program_state = PROGRAM_STATE_RECORDING;
    } else if (globals.program_state == PROGRAM_STATE_GRABBING) {
	globals.program_state = PROGRAM_STATE_RECORDING;
//    } else if (globals.program_state == PROGRAM_STATE_RECORDING) {
//	globals.program_state = PROGRAM_STATE_GRABBING;
    }
    globals.ig.write_flag = 1;
}

static void
handle_wdf ()
{
    globals.ig.write_dark_flag = !globals.ig.write_dark_flag;
}

static void
handle_button_awl ()
{
    globals.auto_window_level = !globals.auto_window_level;
}

static void
handle_ddf ()
{
    globals.drop_dark_frames = !globals.drop_dark_frames;
}

static void
handle_hbf ()
{
    globals.hold_bright_frame = !globals.hold_bright_frame;
}

static void
handle_button_gate ()
{
    globals.gating_flag = !globals.gating_flag;
    if (!globals.gating_flag && globals.ig.od) {
	ise_ontrak_engage_relay (globals.ig.od, 0, 0);   /* GCS FIX: brightframe relay */
    }
}

static void
handle_button_track ()
{
    globals.tracking_flag = !globals.tracking_flag;
    if (!globals.tracking_flag && globals.gating_flag) {
	globals.gating_flag = !globals.gating_flag;
	if (!globals.gating_flag && globals.ig.od) {
	    ise_ontrak_engage_relay (globals.ig.od, 0, 0);   /* GCS FIX: brightframe relay */
	}
    }
    /* GCS FIX: stop tracking within irisgrab2 for both panels */
}

static void
handle_button_replay ()
{
    globals.program_state = PROGRAM_STATE_REPLAYING;
}

static void
handle_button_pause ()
{
    globals.program_state = PROGRAM_STATE_STOPPED;
}

static void
handle_slider_frame (HWND hwnd)
{
    int pos;
    pos = (int) SendDlgItemMessage (hwnd, IDC_SLIDER_FRAME, TBM_GETPOS, 0, 0);
    ise_fluoro_display_frame_no (pos);
}

static void
handle_mouse_lbutton_up (int idx, int x, int y)
{
    float ix, iy;
    int img_x, img_y;
    if (in_picture_window_rect (idx, x, y)) {
	if (globals.ig.panel[idx].have_tracker) {
	    // gl_get_image_pos (idx, x, y, &img_x, &img_y);
	    gl_client_to_image (idx, &ix, &iy, x, y);
	    img_x = (int) (ix + 0.5);
	    img_y = (int) (iy + 0.5);
	    globals.ig.panel[idx].tracker_info.m_curr_x = img_x;
	    globals.ig.panel[idx].tracker_info.m_curr_y = img_y;
	    globals.ig.panel[idx].now_tracking = 1;
	}
    }
}

static void
handle_mouse_motion (int idx, int x, int y)
{
    if (in_picture_window_rect (idx, x, y)) {
	gl_set_findtrack_overlay_pos (idx, x, y);
	globals.win[idx].findtrack_overlay_flag = 1;
	globals.notify[idx] = 1;
    } else {
	if (globals.win[idx].findtrack_overlay_flag) {
	    globals.win[idx].findtrack_overlay_flag = 0;
	    globals.notify[idx] = 1;
	}
    }
}

LRESULT CALLBACK
dialog_callback (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    Frame* curr;
    switch (message) {
    case WM_COMMAND:
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	// Parse the menu selections:
	switch (wmId)
	{
	case IDM_ABOUT:
	    DialogBox (globals.hinst, (LPCTSTR) IDD_ABOUTBOX, hwnd, (DLGPROC) about_callback);
	    break;
	case IDM_FILE_OPEN:
	    break;
	case IDM_EXIT:
	    DestroyWindow (hwnd);
	    break;
	case IDM_SOURCE_MATROX:
	    ise_grab_set_source (&globals.ig, ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO);
	    highlight_source_menu (hwnd);
	    break;
	case IDM_SOURCE_BITFLOW:
	    ise_grab_set_source (&globals.ig, ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO); //default is low
	    highlight_source_menu (hwnd);
	    break;
	case IDM_SOURCE_SYNTHETIC:
	    ise_grab_set_source (&globals.ig, ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO);
	    highlight_source_menu (hwnd);
	    break;
	case IDM_SOURCE_INTERNAL:
	    ise_grab_set_source (&globals.ig, ISE_IMAGE_SOURCE_INTERNAL_FLUORO);
	    highlight_source_menu (hwnd);
	    break;
	case IDM_SOURCE_FILE:
	    ise_grab_set_source (&globals.ig, ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO); //default is high;
            fileload_open (&globals.ig.fileload);             
            highlight_source_menu (hwnd);
            globals.loadfrom_file = !globals.loadfrom_file;
                        //blt_frame_gl (0, curr, curr->img, ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO);
	    break;
	case IDM_SOURCE_SETTINGS:
	    DialogBox (globals.hinst, (LPCTSTR) IDD_SOURCE_DIALOG, hwnd, (DLGPROC) source_settings_callback);
	    break;
	case IDM_OPTIONS_DROP_DARK:
	    handle_ddf ();
	    highlight_dialog_buttons ();
	    break;
	case IDM_OPTIONS_WRITE_DARK:
	    handle_wdf ();
	    highlight_dialog_buttons ();
	    break;
	case IDM_OPTIONS_HOLD_BRIGHT:
	    handle_hbf ();
	    highlight_dialog_buttons ();
	    break;

	case IDC_BUTTON_PAUSE:
	case IDC_BUTTON_STOP:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_stop ();
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	case IDC_BUTTON_GRAB:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_grab ();
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	case IDC_BUTTON_REC:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_rec ();
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	case IDC_BUTTON_RESET:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_reset ();
		globals.notify[hwnd_to_idx (hwnd)] = 1;
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	case IDC_BUTTON_AWL:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_awl ();
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	case IDC_BUTTON_GATE:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_gate ();
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	case IDC_BUTTON_TRACK:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_track ();
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	/* Replay buttons */
	case IDC_BUTTON_REWBEG:
	    switch (wmEvent) {
	    case BN_CLICKED:
		ise_fluoro_rewind_display ();
		gdi_rewind_frame_slider (hwnd_to_idx (hwnd));
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	case IDC_BUTTON_REPLAY:
	    switch (wmEvent) {
	    case BN_CLICKED:
		handle_button_replay ();
		globals.notify[hwnd_to_idx (hwnd)] = 1;
		highlight_dialog_buttons ();
		return 1;
	    default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	    }
	}
	break;
    case WM_LBUTTONUP:
	{
	    int x = LOWORD(lParam);
	    int y = HIWORD(lParam);
	    int idx = hwnd_to_idx (hwnd);
	    handle_mouse_lbutton_up (idx, x, y);
	}
	break;
    case WM_RBUTTONUP:
	{
	    int x = LOWORD(lParam);
	    int y = HIWORD(lParam);
	    gl_zoom_at_pos (hwnd_to_idx (hwnd), x, y);
	    globals.notify[hwnd_to_idx (hwnd)] = 1;
	}
	break;
    case WM_MOUSEMOVE:
	{
	    int x = LOWORD(lParam);
	    int y = HIWORD(lParam);
	    int idx = hwnd_to_idx (hwnd);
	    handle_mouse_motion (idx, x, y);
	}
	break;
    case WM_SIZE:
        // Resize window
        if (wParam != SIZE_MINIMIZED) {
	    do_resize (hwnd);
	    resize_gl_window (hwnd_to_idx (hwnd));
	}
        break;
    case WM_PAINT:
	{
	    debug_printf ("Got WM_PAINT event\n");
	    hdc = BeginPaint(hwnd, &ps);
	    globals.notify[hwnd_to_idx (hwnd)] = 1;
	    EndPaint(hwnd, &ps);
	}
	break;
    case WM_HSCROLL:
	{
	    handle_slider_frame (hwnd);
	    return 1;
	}
    case WM_VSCROLL:
	{
	    HWND sb_hwnd = (HWND) lParam;
	    unsigned short bot, top;

	    bot = (unsigned short) SendDlgItemMessage (hwnd, IDC_SLIDER_BOT, TBM_GETPOS, 0, 0);
	    top = (unsigned short) SendDlgItemMessage (hwnd, IDC_SLIDER_TOP, TBM_GETPOS, 0, 0);
	    if (sb_hwnd == GetDlgItem(hwnd, IDC_SLIDER_BOT)) {
		if (bot > top) {
		    SendDlgItemMessage (hwnd, IDC_SLIDER_TOP, TBM_SETPOS, TRUE, bot);
		    bot = top;
		}
	    }
	    if (sb_hwnd == GetDlgItem(hwnd, IDC_SLIDER_TOP)) {
		if (top < bot) {
		    SendDlgItemMessage (hwnd, IDC_SLIDER_BOT, TBM_SETPOS, TRUE, top);
		    bot = top;
		}
	    }
	    update_wl_status (hwnd_to_idx(hwnd), bot, top);
	    update_lut (hwnd, (long) bot, (long) top);
	    globals.notify[hwnd_to_idx (hwnd)] = 1;
	}
	return 0;
    case WM_NOTIFY:
	{
	    NMHDR *pnmh = (LPNMHDR) lParam;
	    if (wParam == IDC_SLIDER_BOT) {
		return 0;
	    }
	}
	return 0;
    case WM_INITDIALOG:
	return TRUE;
    case WM_CLOSE:
    case WM_DESTROY:
	PostQuitMessage(0);
	globals.quit = 1;
	break;
    default:
	return 0;
    }
    return 0;
}

LRESULT CALLBACK
about_callback (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
	return TRUE;
    case WM_COMMAND:
	if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
	    EndDialog(hDlg, LOWORD(wParam));
	    return TRUE;
	}
	break;
    }
    return FALSE;
}

LRESULT CALLBACK
source_settings_callback (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    switch (message)
    {
    case WM_INITDIALOG:
	return TRUE;
    case WM_COMMAND:
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId) {
	case IDOK:
	    EndDialog(hDlg, wmId);
	    return TRUE;
	case IDCANCEL:
	    EndDialog(hDlg, wmId);
	    return TRUE;
	}
	break;
    }
    return FALSE;
}
