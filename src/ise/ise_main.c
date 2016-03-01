/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include "ise.h"
#include "ise_rsc.h"
#include "ise_framework.h"
#include "matrox_source.h"
#include "bitflow.h"
#include "frame.h"
#include "ise_gdi.h"
#include "ise_gl.h"
#include "debug.h"
#include "ise_gl_shader.h"
#include "cbuf.h"
#include "ise_config.h"
#include "indico_info.h"

#define MAX_LOADSTRING 100

/* -------------------------------------------------------------------------*
    Global variables
 * -------------------------------------------------------------------------*/
TCHAR szWindowClass[MAX_LOADSTRING];

/* -------------------------------------------------------------------------*
    Public functions
 * -------------------------------------------------------------------------*/
void
parse_command_line (void)
{   
    LPWSTR *szArglist;
    int nArgs;
    int i;

    szArglist = CommandLineToArgvW (GetCommandLineW(), &nArgs);
    if (NULL == szArglist) {
	wprintf (L"CommandLineToArgvW failed\n");
	exit (1);
    } else {
	for (i=0; i<nArgs; i++) {
	    debug_printf ("%d: %ws\n", i, szArglist[i]);
	}
    }
    LocalFree (szArglist);
}

static int
ise_needs_drawing (int idx)
{
    if (!globals.notify[idx])
	return 0;
    globals.notify[idx] = 0;
    return 1;
}

Frame*
ise_get_next_drawable (int idx)
{
    Frame* f;

    switch (globals.program_state) {
    case PROGRAM_STATE_STOPPED:
	f = ise_fluoro_get_drawable_stopped (idx);
	break;
    case PROGRAM_STATE_GRABBING:
    case PROGRAM_STATE_RECORDING:
	f = ise_fluoro_get_drawable_grabbing (idx);
	break;
    case PROGRAM_STATE_REPLAYING:
	/* GCS FIX: Need to check timestamps, etc */
	/* GCS NOTE: This does stop at the end (i.e. get_next returns null). */
	f = ise_fluoro_get_drawable_replaying (idx);
	break;
    }
    return f;
}

int
do_main_loop (void)
{
    int idx;
    Frame* frame;

    while (1) 
    {
        /* Check for windows messages */
        MSG msg;
        while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) 
        {
            if (!TranslateAccelerator (msg.hwnd, (HACCEL) globals.hAccelTable, &msg)) 
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }

        /* Check for program end */
        if (globals.quit) break;

        /* BLT fluoro to screen */
        for (idx = 0; idx < globals.num_panels; idx++) 
        {
            if (ise_needs_drawing (idx)) 
            {
                frame = ise_get_next_drawable (idx);
                if (frame) 
                {
                    if (globals.auto_window_level) 
                    {
                        gdi_update_lut_slider (idx, frame->autosense.min_brightness, 
                            frame->autosense.max_brightness);
                        gl_update_lut (idx, frame->autosense.min_brightness, 
                            frame->autosense.max_brightness);
                        gdi_update_frame_slider (idx);
                    }
                    // MODIFIED by RUI!!
                    // provide texture size information here instead of hardcoding it
                    // in the display function
                    if (globals.have_bitflow_hardware && !globals.loadfrom_file) 
                        blt_frame_gl (idx, frame, frame->img, ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO);
                    else
                        blt_frame_gl (idx, frame, frame->img, ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO);
                    

                    ise_fluoro_display_lock_release (idx);
                } 
                else 
                {
                    black_frame_gl (idx);
                }
            }

            update_queue_status (idx);
        }

        /* Give up timeslice */
	    Sleep (20);
    } //end while(1)

    return 0;
}

int APIENTRY
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
         LPSTR lpCmdLine, int nCmdShow)
{
    int rc;
    unsigned long image_source;

    globals.hinst = hInstance;
    //LoadString (hInstance, IDC_ISE, szWindowClass, MAX_LOADSTRING);
    register_dialog_class (hInstance);
    globals.hAccelTable = LoadAccelerators (hInstance, (LPCTSTR)IDC_ISE);

    /* Load config file */
    init_globals ();

    /* Just in case we don't have a config file, create one now */
    save_globals ();

    /* Find out if we have a matrox board installed in the computer */
#if (MIL_FOUND)
    globals.have_matrox_hardware = matrox_probe ();
#else
    globals.have_matrox_hardware = 0;
#endif
#if (BITFLOW_FOUND)
    globals.have_bitflow_hardware = bitflow_probe ();
#else
    globals.have_bitflow_hardware = 0;
#endif
    if (globals.have_matrox_hardware) 
        image_source = ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO;
    else if(globals.have_bitflow_hardware)
        image_source = ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO;
    else 
        image_source = ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO;
    

    /* Initialize communication with indico process (x-ray generator control) */
    init_indico_shmem (&globals.indico_info);

    /* Command line is not currently used */
    parse_command_line ();

    /* Init GUI */
    check_display (hInstance);
    create_windows (hInstance, nCmdShow);
    init_lut ();
    init_frame_slider ();
    init_gl ();

    /* Init frame grabber threads and file writer thread */
    switch (globals.panel_select) 
    {
        case USE_PANEL_1:
	    rc = ise_startup (
	        image_source, 
	        1,
                ISE_CLIENT_IP_1,
                ISE_SERVER_IP_1,
                ISE_BOARD_1,
                ISE_FLIP_1,
                globals.buffer_num_frames,
                ISE_FRAMERATE_1,
                0,0,0,0,0,0);
            break;
        case USE_PANEL_2:
            rc = ise_startup (
                image_source, 
                1,
                ISE_CLIENT_IP_2,
                ISE_SERVER_IP_2,
                ISE_BOARD_2,
                ISE_FLIP_2,
                globals.buffer_num_frames,
                ISE_FRAMERATE_2,
                0,0,0,0,0,0);
            break;
        case USE_BOTH_PANELS:
            rc = ise_startup (
                image_source, 
                2,
                ISE_CLIENT_IP_1,
                ISE_SERVER_IP_1,
                ISE_BOARD_1,
                ISE_FLIP_1,
                globals.buffer_num_frames,
                ISE_FRAMERATE_1,
                ISE_CLIENT_IP_2,
                ISE_SERVER_IP_2,
                ISE_BOARD_2,
                ISE_FLIP_2,
                globals.buffer_num_frames,
                ISE_FRAMERATE_2
            );
            break;
    }

    if (rc != ISE_SUCCESS)
    {
        MessageBox(NULL, "ISE initialization failed!", "Error!!", MB_OK);
        ise_shutdown();
        return 0;
    }

    do_main_loop ();

    ise_shutdown ();

    return 0;
}
