/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "config.h"
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <math.h>
#include <errno.h>
#include "ise.h"
#include "ise_config.h"
#include "debug.h"
#include "igpax.h"
#include "tracker.h"
#include "ise_framework.h"
#include "ise_ontrak.h"
#include "ise_globals.h"
#include "ise_igpax.h"
#include "synthetic_source.h"
#include "matrox_source.h"
#include "bitflow.h"
#include "fileload.h"

#define M_PI 3.14159265358979323846
#define ROTATE_PANEL_B 1

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
static void ise_grab_continuous_thread (void* v);
static Ise_Error grabber_init (IseFramework* ig);
static Ise_Error imaging_system_startup (IseFramework* ig,
		int idx, char* ip_client, char* ip_server,
		unsigned int board_no, unsigned int rotate,
		unsigned int track, unsigned int num_frames,
		double framerate);

/* -----------------------------------------------------------------------
    Public functions
   ----------------------------------------------------------------------- */
int
ise_startup (unsigned long mode, 
	     int num_panels, 
	     char *client_ip_1,
	     char *server_ip_1,
	     int board_1,
	     int flip_1,
	     unsigned int num_frames_1,
	     double framerate_1,
	     char *client_ip_2,
	     char *server_ip_2,
	     int board_2,
	     int flip_2,
	     unsigned int num_frames_2,
	     double framerate_2)
{
    const int use_ontrak = 1;
    Ise_Error rc;

    /* Verify mode is valid */
    switch (mode) 
    {
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
	    break;
        case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
	    break;
        default:
            exit(1);
    }

    /* Shutdown if alrady started */
    ise_shutdown ();

    /* Set variables */
    globals.ig.image_source = mode;
    globals.ig.num_idx = num_panels;

    /* Initialize command processors */
    if (mode == ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO ||
        mode == ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO ||
        mode == ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO ||
        mode == ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO)        
        ise_igpax_init ();

    /* Initialize frame grabbers */
    rc = grabber_init (&globals.ig);
    if (rc != ISE_SUCCESS) {
	printf ("Error: irisgrab_init() failed\n");
	return rc;
    }

    /* Initialize relay hardware */
    globals.ig.od = 0;
    if (use_ontrak) {
	globals.ig.od = ise_ontrak_init ();
    }

    /* Initialize circular buffer */
    cbuf_init (&globals.ig);

    /* Start up the individual imaging systems */
    rc = imaging_system_startup (&globals.ig, 0, 
			client_ip_1, server_ip_1, 
			board_1, flip_1, ISE_TRACKING_ENABLED,
			num_frames_1, framerate_1);
    if (rc != ISE_SUCCESS) {
	return rc;
    }
    if (num_panels == 2) {
	rc = imaging_system_startup (&globals.ig, 1, 
			    client_ip_2, server_ip_2, 
			    board_2, flip_2, ISE_TRACKING_ENABLED,
			    num_frames_2, framerate_2);
	if (rc != ISE_SUCCESS) {
	    return rc;
	}
    }

    /* Start up the writer */
    globals.ig.write_flag = 0;
    globals.ig.write_dark_flag = 0;
    globals.ig.fw = filewrite_init (&globals.ig);
    if (!globals.ig.fw) {
	printf ("Error: filewrite_init() failed\n");
        exit(1);
    }

#if defined (commentout)
    /* Put it into fluoro mode */
    /* GCS: Both panels must have same framerate */
    rc = ise_grab_set_igpax_fluoro (&globals.ig, framerate_1);
    if (rc) {
	printf ("Error: irisgrab_init_fluoro() failed\n");
        exit(1);
    }
#endif

    return 0;
}

void
ise_shutdown (void)
{
    if (globals.ig.fw) {
	filewrite_stop (globals.ig.fw);
	globals.ig.fw = 0;
    }
	
    if (globals.ig.num_idx > 0) {
	ise_grab_continuous_stop (&globals.ig);
	ise_grab_close (&globals.ig);
    }
}

/* -----------------------------------------------------------------------
    Private functions
   ----------------------------------------------------------------------- */
static Ise_Error
grabber_init (IseFramework* ig)
{
    switch (ig->image_source) 
    {
        case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
            {
#if (MIL_FOUND)
	    Ise_Error rc;
	    rc = matrox_init (&ig->matrox, ig->image_source);
	    if (rc) {
		debug_printf ("matrox_init failed!\n");
		return rc;
	    }
#endif
            }
	    break;
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
	    {
#if (BITFLOW_FOUND)
	// initialize the card
	Ise_Error rc;
	rc = bitflow_init (&ig->bitflow, ig->image_source);
	if (rc) 
	{
		debug_printf ("bitflow_init failed!\n");
		return rc;
	}
#endif
	    }
            break;
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
            {
                Ise_Error rc;
                rc = fileload_init(&ig->fileload, ig->image_source);
	        if (rc) 
	        {
		    debug_printf ("bitflow_init failed!\n");
		    return rc;
	        }
            }
            break;
        }
    return ISE_SUCCESS;
}

/* All startup related to a single system */
static Ise_Error
imaging_system_startup (IseFramework* ig,
		int idx,
		char* ip_client,
		char* ip_server,
		unsigned int board_no,
		unsigned int rotate,
		unsigned int track,
		unsigned int num_frames,
		double framerate
		)
{
    int rc;

    /* Only two choices for framerates */
    if (framerate <= 2.0) {
	ig->panel[idx].framerate = ISE_FRAMERATE_1_FPS;
    } else {
	ig->panel[idx].framerate = ISE_FRAMERATE_7_5_FPS;
    }

    if (IS_REAL_FLUORO (ig->image_source)) {
	rc = ise_igpax_open (&ig->igpax[idx], ip_server, ip_client);
	if (rc != ISE_SUCCESS) {
	    debug_printf ("igpax_init[%d] failed!\n", idx);
	    return rc;
	}
    }
    
    /* Set image size */
    switch (ig->image_source) 
    {
        case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
	    ig->size_x = 1024;
	    ig->size_y = 768;
	    break;
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
	    ig->size_x = 2048;
	    ig->size_y = 1536;
	    break;
        default:
	    return ISE_ERR_INVALID_PARM;
    }

    switch (ig->image_source) {
        case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
#if (MIL_FOUND)
	rc = matrox_open (&ig->matrox, idx, board_no, ig->image_source, 
	    ig->panel[idx].framerate);
	if (rc != ISE_SUCCESS) {
	    debug_printf ("matrox_open failed [%d]!\n", idx);
	    return rc;
	}
	if (ig->size_x != ig->matrox.mtx_size_x || ig->size_y != ig->matrox.mtx_size_y) {
	    return ISE_MATROX_BAD_IMAGE_SIZE;
	}
#endif
	    break;
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
#if (BITFLOW_FOUND)
	rc = bitflow_open (&ig->bitflow, idx, board_no, ig->image_source, 
		ig->panel[idx].framerate);
	if (rc != ISE_SUCCESS) 
	{
	    debug_printf ("bitflow_open failed [%d]!\n", idx);
	    return rc;
	}
	if (ig->size_x != ig->bitflow.sizeX || ig->size_y != ig->bitflow.sizeY) 
	{
	    return ISE_BITFLOW_BAD_IMAGE_SIZE;
	}
#endif
	    break;
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
            // open file
            rc = fileload_open(&ig->fileload);
	    if (rc != ISE_SUCCESS) 
	    {
	        debug_printf ("fileload_open failed [%d]!\n", idx);
	        return rc;
	    }
	    if (ig->size_x != ig->fileload.sizeX || ig->size_y != ig->fileload.sizeY) 
    	        return ISE_FILE_BAD_IMAGE_SIZE;

            break;
    default:
	return ISE_ERR_INVALID_PARM;
    }

    rc = cbuf_init_queue (ig, idx, num_frames);
    if (rc != ISE_SUCCESS) {
	debug_printf ("cbuf_open failed!\n");
	return 1;
    }
    ig->panel[idx].rotate_flag = rotate;

    ig->panel[idx].have_tracker = 0;
    ig->panel[idx].now_tracking = 0;
    if (track) {
        ig->panel[idx].have_tracker = 1;
	tracker_init (&ig->panel[idx].tracker_info);
    }

    /* Put Paxscan into fluoro mode */
    if (IS_REAL_FLUORO (ig->image_source)) {
	rc = ise_igpax_start_fluoro (&ig->igpax[idx], ig->image_source, 
    				ig->panel[idx].framerate);
	if (rc != ISE_SUCCESS) return rc;
    }

    return ISE_SUCCESS;
}

void
ise_grab_get_resolution (IseFramework* ig, int* h, int* w)
{
    *w = ig->size_x;
    *h = ig->size_y;
}

void 
ise_grab_configure_writing (IseFramework* ig, int write_flag, int write_dark)
{
    /* No need to lock, writing 32-bit int is atomic in win32 */
    ig->write_dark_flag = write_dark;
    ig->write_flag = write_flag;
}

int 
ise_grab_start_capture_threads (IseFramework* ig, void (*notify)(int))
{
    int rc;
    printf ("IRISGRAB.DLL gonna grab continuous...\n");
    rc = ise_grab_continuous_start (ig, notify);
    printf ("IRISGRAB.DLL did grab continuous (rc = %d)\n",rc);
    return rc;
}

void 
ise_grab_close (IseFramework* ig)
{
    int idx;

    
    if (IS_REAL_FLUORO(ig->image_source) 
        || ISE_IMAGE_SOURCE_FILE_LORES_FLUORO 
        || ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO) 
    {
	cbuf_shutdown (ig);
#if (MIL_FOUND)
        if (ig->image_source == ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO || 
            ig->image_source == ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO )
	    matrox_shutdown (&ig->matrox, ig->num_idx);
#endif
#if (BITFLOW_FOUND) 
        if (ig->image_source == ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO || 
            ig->image_source == ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO )
	    bitflow_shutdown (&ig->bitflow, ig->num_idx);
#endif

        for (idx = 0; idx < globals.ig.num_idx; idx++) 
        {
            if (IS_REAL_FLUORO(ig->image_source)) 
                ise_igpax_shutdown (&ig->igpax[idx]);
            if (ig->panel[idx].have_tracker) 
                tracker_shutdown (&ig->panel[idx].tracker_info);
        }    
    }
}

void
ise_grab_set_source (IseFramework* ig, unsigned long new_source)
{
    if (ig->image_source == new_source) return;

    switch (ig->image_source) 
    {
        case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
#if (MIL_FOUND)
	matrox_shutdown (&ig->matrox, globals.ig.num_idx);
#endif
	    break;
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
#if(BITFLOW_FOUND)
        bitflow_shutdown(&ig->bitflow, globals.ig.num_idx);
#endif
	    break;
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
            fileload_shutdown(&ig->fileload);
            break;
        case ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_INTERNAL_FLUORO:
        default:
            break;
    }

    switch (new_source) 
    {
        case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
#if (MIL_FOUND)
    matrox_init (&ig->matrox, new_source);
#endif
            break;	
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
#if (BITFLOW_FOUND)
    bitflow_init (&ig->bitflow, new_source);
#endif	
	    break;
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
            fileload_init (&ig->fileload, new_source);
            break;
        case ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_INTERNAL_FLUORO:
        default:
            break;
    }

    ig->image_source = new_source;
}

int
ise_grab_continuous_start (IseFramework* ig, void (*notify)(int))
{
    int idx;
    /* Launch threads */
    for (idx = 0; idx < globals.ig.num_idx; idx++) {
	BOOL rc;
	ig->grab_thread_data[idx].ig = ig;
	ig->grab_thread_data[idx].imager_no = idx;
	ig->grab_thread_data[idx].notify_routine = notify;
	ig->grab_thread_data[idx].done = 0;
	debug_printf ("IRISGRAB launching thread %d\n", idx);
	ig->grab_thread[idx] = (HANDLE) _beginthread (ise_grab_continuous_thread, 0, (void*) &ig->grab_thread_data[idx]);
	if (ig->grab_thread[idx] == (HANDLE) -1) {
	    debug_printf ("Error spawning thread: %d (%d,%d)\n", errno, EAGAIN, EINVAL);
	}
	//rc = SetThreadPriority (ig->grab_thread[idx], THREAD_PRIORITY_HIGHEST);
	rc = SetThreadPriority (ig->grab_thread[idx], THREAD_PRIORITY_TIME_CRITICAL);
	if (!rc) {
	    debug_printf ("Error setting thread priority: %d\n",GetLastError());
	}
    }

    return 0;
}

int
ise_grab_continuous_stop (IseFramework* ig)
{
    DWORD rc;
    int idx;

    /* Wait for threads to finish */
    debug_printf ("IRISGRAB.DLL waiting for threads to finish\n");
    for (idx = 0; idx < globals.ig.num_idx; idx++) {
	ig->grab_thread_data[idx].done = 1;
    }
    rc = WaitForMultipleObjects (globals.ig.num_idx,ig->grab_thread,TRUE,1000);

    /* I hope they finished! */
    if (rc >= WAIT_OBJECT_0 && rc <= WAIT_OBJECT_0 + globals.ig.num_idx-1) {
	debug_printf ("IRISGRAB.DLL WAIT_OBJECT_0\n", rc);
    }
    if (rc >= WAIT_ABANDONED_0 && rc <= WAIT_ABANDONED_0 + globals.ig.num_idx-1) {
	debug_printf ("IRISGRAB.DLL WAIT_ABANDONED_0\n", rc);
    }
    if (rc == WAIT_TIMEOUT) {
	debug_printf ("IRISGRAB.DLL WAIT_TIMEOUT\n", rc);
    }

    return 0;
}

static void
ise_grab_continuous_thread (void* v)
{
    ThreadData* data = (ThreadData*) v;
    IseFramework* ig = data->ig;
#if (MIL_FOUND)
    MatroxInfo* matrox = &ig->matrox;
#endif
#if (BITFLOW_FOUND)
    BitflowInfo* bitflow = &ig->bitflow;
#endif

    unsigned int idx = data->imager_no;
    CBuf* cbuf = &ig->cbuf[idx];
    Frame *curr;
    LARGE_INTEGER clock_count;
    LARGE_INTEGER clock_freq;
    int num_grabbed = 0;
    const int MAX_NUM_GRABBED = 10;
    int done1 = 0, done2 = 0;
    int saw_bright_frame = 0;
    static int fid = 0;
    double prev_timestamp = 0.0;
    double timestamp_diff;
    int prev_dark = 1;
    int prev_indico_state = INDICO_SHMEM_XRAY_OFF;
    Ise_Error rc;

    QueryPerformanceFrequency (&clock_freq);

    switch (ig->image_source) 
    {
        case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
#if (MIL_FOUND)
    if (idx == 1) {
        matrox_prepare_grab (&ig->matrox, idx);
        break;
    }
    matrox_prepare_grab (&ig->matrox, idx);
#endif
            break;
        case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
#if (BITFLOW_FOUND)
    if (idx == 1) {
        rc = bitflow_grab_setup (&ig->bitflow, idx);
        break;
    }
    rc = bitflow_grab_setup (&ig->bitflow, idx);
    if (rc != ISE_SUCCESS) 
    {
        debug_printf("grab_setup failed\n");
    }
#endif
            break;
        case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
        case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
        case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
            /* Do nothing */
            break;
        case ISE_IMAGE_SOURCE_INTERNAL_FLUORO:
            /* Reset internal pointer */
            cbuf_internal_grab_rewind (cbuf);
        break;
    }

    debug_printf ("Clock Freq = %g\n", (double) clock_freq.QuadPart);
    
    do 
    {
        /* Get a frame */
        switch (ig->image_source) 
        {
            case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
            case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
            case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
            case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
            case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
                curr = cbuf_get_any_frame (cbuf, globals.drop_dark_frames);
                break;
            case ISE_IMAGE_SOURCE_INTERNAL_FLUORO:
                curr = cbuf_internal_grab_next_frame (cbuf);
                break;
            default:
                break;
        }

        /* Tell source to fill it (this call blocks) */
        switch (ig->image_source) 
        {
            case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
#if (MIL_FOUND)
    matrox_grab_image (curr, &ig->matrox, idx, ig->panel[idx].rotate_flag, data->done);
#endif
                break;
	    case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
#if (BITFLOW_FOUND)
    bitflow_grab_image (curr->img, &ig->bitflow, idx);
#endif
                break;
            case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
                Sleep (100);
                synthetic_grab_image (curr);
                break;
            case ISE_IMAGE_SOURCE_FILE_LORES_FLUORO:
            case ISE_IMAGE_SOURCE_FILE_HIRES_FLUORO:
                Sleep (100);
                fileload_load_image (curr->img, &ig->fileload, idx);
                ig->fileload.curIdx[idx] ++;
                if (ig->fileload.curIdx[idx] > ig->fileload.nImages[idx])
                    goto notify;
                break;
            case ISE_IMAGE_SOURCE_INTERNAL_FLUORO:
                Sleep (100);
                if (!curr) 
                    goto notify;
                break;
        }

        QueryPerformanceCounter(&clock_count);

        if (data->done) 
            done2 = 1;

        curr->timestamp = (double) clock_count.QuadPart / (double) clock_freq.QuadPart;
        curr->indico_state = globals.indico_info.shmem->rad_state[idx];
        curr->id = fid++;

        /* Detect bright frames */
        frame_autosense (curr, prev_dark, ig->size_y, ig->size_x);
        prev_dark = frame_is_dark (curr);

        /* Check writability */
        curr->written = 0;
        curr->writable = 0;
        if (ig->write_flag) 
            if (ig->write_dark_flag) 
                /* If we are writing dark frames */
                curr->writable = 1;
            else if (curr->indico_state == INDICO_SHMEM_XRAY_ON) 
                /* If the generator tells us it has xray on */
                curr->writable = 1;
            else if (prev_indico_state == INDICO_SHMEM_XRAY_ON) 
                /* We save one extra at the end of every run */
                curr->writable = 1;
#if defined (commentout)
            else if (!frame_is_dark(curr)) 
			/* If the autosense tells us it has xray on */
			curr->writable = 1;
#endif

	/* Do tracking */
	if (ig->panel[idx].have_tracker && ig->panel[idx].now_tracking) 
	{
	    track_frame (ig, idx, curr);
	    debug_printf ("Ground truth position: %d %d\n", curr->clip_x, curr->clip_y);
	    curr->clip_x = ig->panel[idx].tracker_info.m_curr_x;
	    curr->clip_y = ig->panel[idx].tracker_info.m_curr_y;
	}

	/* We will control relays based on imager 0 */
	if (idx == 0) 
	{
	    int close_gating_relay = 0;
	    int close_sync_relay = 0;

	    /* Check if marker is in the gating window */
	    if (ig->od && globals.gating_flag) 
                if (curr->clip_y >= 640 && curr->clip_y <= 710) 
                    close_gating_relay = 1;

	    /* Check if xrays are on */
	    if (curr->indico_state == INDICO_SHMEM_XRAY_ON) 
                close_sync_relay = 1;
#if defined (commentout)
    close_sync_relay = frame_is_bright(curr);
#endif

            /*  Tell relays to open or close */
            ise_ontrak_engage_relay (ig->od, close_gating_relay, close_sync_relay);
	}

	/* For the extra frame at the end of the run, "lie about its state" 
	    so that the cbuf code will give it save priority */
	if (prev_indico_state == INDICO_SHMEM_XRAY_ON) 
	{
	    prev_indico_state = curr->indico_state;
	    curr->indico_state = INDICO_SHMEM_XRAY_ON;
	} 
	else 
	{
	    prev_indico_state = curr->indico_state;
	}

	timestamp_diff = curr->timestamp - prev_timestamp;
	prev_timestamp = curr->timestamp;
	debug_printf ("GRABBED %d %d %21.7f %21.7f %d Mean:%6d Ctr:%6d %s\n", 
	    idx, curr->id, 
	    curr->timestamp, timestamp_diff,
	    curr->indico_state, 
	    curr->autosense.mean_brightness,
	    curr->autosense.ctr_brightness,
	    frame_is_dark(curr) ? "Dark" : "Bright");

	if (ig->image_source != ISE_IMAGE_SOURCE_INTERNAL_FLUORO) 
	{
	    /* Add frame to list */
	    cbuf_append_waiting (cbuf, curr);
	}

        /* Notify */
        notify:
            if (data->notify_routine) 
                (*data->notify_routine)(data->imager_no);

    } 
    while (!done2);
}
