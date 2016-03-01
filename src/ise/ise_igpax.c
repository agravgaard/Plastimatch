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
#include "ise.h"
#include "debug.h"
#include "cbuf.h"
#include "frame.h"
#include "igpax.h"
#include "tracker.h"
#include "ise_ontrak.h"
#include "ise_globals.h"
#include "kill.h"

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
static void ise_igpax_thread (void* arg);
static VOID CALLBACK ise_igpax_send_command_internal (DWORD arg);

/* -----------------------------------------------------------------------
   Global variables
   ----------------------------------------------------------------------- */
#define M_PI 3.14159265358979323846

/* -----------------------------------------------------------------------
    Public functions
   ----------------------------------------------------------------------- */
void
ise_igpax_init (void)
{
    kill_igpax();
}

Ise_Error
ise_igpax_open (IgpaxInfo* igpax, char* ip_address_server, char* ip_address_client)
{
    char s1[20], s2[20];
    char buffer[_MAX_PATH];

    /* Initialize IgpaxInfo */
    memset (igpax, 0, sizeof(igpax));
    strcpy (igpax->ip_address_client, ip_address_client);
    strcpy (igpax->ip_address_server, ip_address_server);

    /* The pid is used to determine if the connection is made */
    igpax->pid = -1;

    /* Open a set of pipes */
    if (_pipe (igpax->pipe_to_igpax, 256, O_BINARY) == -1) {
	debug_printf ("Error creating pipe_to_igpax()\n");
	return ISE_IGPAX_CREATE_PIPE_FAILURE;
    }
    if (_pipe (igpax->pipe_from_igpax, 256, O_BINARY) == -1) {
	debug_printf ("Error creating pipe_from_igpax()\n");
	return ISE_IGPAX_CREATE_PIPE_FAILURE;
    }

    /* Create child process */
    itoa (igpax->pipe_to_igpax[0], s1, 10);
    itoa (igpax->pipe_from_igpax[1], s2, 10);
    _getcwd (buffer, _MAX_PATH);
    debug_printf("PWD = %s\n", buffer);

    /* We'll try it both ways */
    igpax->pid = spawnl (P_NOWAIT, "igpax", "igpax",
	igpax->ip_address_client, igpax->ip_address_server, s1, s2, NULL);
    if (igpax->pid == -1) {
	igpax->pid = spawnl (P_NOWAIT, "debug/igpax", "igpax",
	    igpax->ip_address_client, igpax->ip_address_server, s1, s2, NULL);
    }
    if (igpax->pid == -1) {
	igpax->pid = spawnl (P_NOWAIT, "c:/gcs6/build/irisgrab-vse2005/debug/igpax", "igpax",
	    igpax->ip_address_client, igpax->ip_address_server, s1, s2, NULL);
    }

    if (igpax->pid == -1) {
        debug_printf ("Spawn failed\n");
	debug_printf ("Errno = %d, \"%s\"\n", errno, strerror (errno));
	return ISE_IGPAX_SPAWN_FAILURE;
    }

    /* Initialize queue */
    igpax->cmd_queue_len = 0;
    igpax->cmd_queue_err = 0;

    /* Create critical section */
    InitializeCriticalSection (&igpax->crit_section);

    /* Launch threads */
    igpax->hthread = (HANDLE) _beginthread (ise_igpax_thread, 0, (void*) igpax);
    debug_printf ("IGPAX launching thread %d\n", igpax->hthread);

    /* Test reading and writing.  Send character 'a', and get 
	integer 0 in response.  This will happen asyncrhonously 
	in worker thread. */
    return ise_igpax_send_command (igpax, IGPAXCMD_TEST_PIPES);
}

Ise_Error
ise_igpax_start_fluoro (IgpaxInfo* igpax, int image_source, Framerate framerate)
{
    int rc;
    char cmd;

    switch (image_source) {
    case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
    case ISE_IMAGE_SOURCE_BITFLOW_LORES_FLUORO:
	cmd = IGPAXCMD_MODE_0;
	break;
    case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
    case ISE_IMAGE_SOURCE_BITFLOW_HIRES_FLUORO:
	cmd = IGPAXCMD_MODE_1;
	break;
    case ISE_IMAGE_SOURCE_SIMULATED_LORES_FLUORO:
    case ISE_IMAGE_SOURCE_SIMULATED_HIRES_FLUORO:
    default:
	return ISE_SUCCESS;
    }

    /* Open link and reset state */
    rc = ise_igpax_send_command (igpax,IGPAXCMD_INIT);
    if (rc) return rc;

    /* Set mode */
    rc = ise_igpax_send_command (igpax,cmd);
    if (rc) return rc;

    /* Set frame rate */
    switch (framerate) {
    case ISE_FRAMERATE_1_FPS:
	rc = ise_igpax_send_command (igpax,IGPAXCMD_SET_1FPS);
	if (rc) return rc;
	break;
    default:
    case ISE_FRAMERATE_7_5_FPS:
	rc = ise_igpax_send_command (igpax,IGPAXCMD_SET_7_5FPS);
	if (rc) return rc;
	break;
    }

    /* Take offset correction and disable update of offset correction  */
    rc = ise_igpax_send_command (igpax,IGPAXCMD_IGTALK2_CORRECTIONS);
    if (rc) return rc;

    /* Engage software handshaking  */
    rc = ise_igpax_send_command (igpax,IGPAXCMD_START_GRABBING);
    if (rc) return rc;

    return ISE_SUCCESS;
}

Ise_Error
ise_igpax_send_command (IgpaxInfo* igpax, char cmd)
{
    DWORD rc;

    EnterCriticalSection (&igpax->crit_section);
    if (igpax->cmd_queue_len >= IGPAX_CMD_QUEUE_SIZE) {
	LeaveCriticalSection (&igpax->crit_section);
	return ISE_IGPAX_QUEUE_OVERFLOW;
    }
    igpax->cmd_queue[igpax->cmd_queue_len] = cmd;
    igpax->cmd_queue_len ++;
    LeaveCriticalSection (&igpax->crit_section);

#if defined (USE_APC)
    rc = QueueUserAPC (ise_igpax_send_command_internal, igpax->hthread, (DWORD) igpax);
    if (rc == 0) {
        debug_printf("ISE_IGPAX_QUEUE_USER_APC_FAILED\n");
	return ISE_IGPAX_QUEUE_USER_APC_FAILED;
    }
#endif
    return ISE_SUCCESS;
}

void
ise_igpax_shutdown (IgpaxInfo* igpax)
{
    char cmd = IGPAXCMD_QUIT;

    write (igpax->pipe_to_igpax[1], (char*) &cmd, sizeof(char));

    /* Normally I would wait until spawned program is done processing,
       as commented out below.  But the _spawnl()/_cwait() doesn't have 
       any timeout, so this program would hang if child doesn't exit.  
       Therefore, we'll just shut down the pipes and hope it all goes ok... */
#if defined (commentout)
    int termstat;
    _cwait (&termstat, igpax->pid, WAIT_CHILD);
    if( termstat & 0x0 )
        printf( "Child failed\n" );
#endif

    SleepEx (200,FALSE);
    close (igpax->pipe_from_igpax[0]);
    close (igpax->pipe_from_igpax[1]);
    close (igpax->pipe_to_igpax[0]);
    close (igpax->pipe_to_igpax[1]);
    igpax->pid = -1;
}


/* -------------------------------------------------------------------------*
   Unused public functions
   Some of this code may be used in the future
 * -------------------------------------------------------------------------*/
#if defined (commentout)
int 
ise_grab_warmup (IseFramework* ig, int idx)
{
    int rc;
    rc = igpax_send_command (&ig->igpax[idx],IGPAXCMD_INIT);
    debug_printf ("igpax_send_command[%d] (%c -> %d)\n",idx,IGPAXCMD_INIT,rc);
    rc = igpax_send_command (&ig->igpax[idx],IGPAXCMD_OFFSET_CAL);
    debug_printf ("igpax_send_command[%d] (%c -> %d)\n",idx,IGPAXCMD_OFFSET_CAL,rc);
    rc = igpax_send_command (&ig->igpax[idx],IGPAXCMD_CLEAR_CORRECTIONS);
    debug_printf ("igpax_send_command[%d] (%c -> %d)\n",idx,IGPAXCMD_CLEAR_CORRECTIONS,rc);
    rc = igpax_send_command (&ig->igpax[idx],IGPAXCMD_SET_1FPS);
    debug_printf ("igpax_send_command[%d] (%c -> %d)\n",idx,IGPAXCMD_SET_1FPS,rc);
    return 0;
}

int 
ise_grab_enable_radiographic_autosense (IseFramework* ig, int idx)
{
    int rc;
    if (ig->image_source != ISE_IMAGE_SOURCE_HIRES_RADIO) {
	return 1;
    }
    rc = igpax_send_command (&ig->igpax[idx],IGPAXCMD_INIT);
    debug_printf ("igpax_send_command[%d] (%c -> %d)\n",idx,IGPAXCMD_INIT,rc);
    rc = igpax_send_command (&ig->igpax[idx],IGPAXCMD_MODE_2);
    debug_printf ("igpax_send_command[%d] (%c -> %d)\n",idx,IGPAXCMD_MODE_2,rc);
    return 0;
}

/* This function assumes that the caller's buffer is large enough */
int 
ise_grab_grab_image (IseFramework* ig, int idx, unsigned char* buffer)
{
    int rc;

    if (ig->image_source != ISE_IMAGE_SOURCE_HIRES_RADIO) {
	return 1;
    }
    debug_printf ("Trying to grab from igpax %d...\n",idx);
    rc = igpax_grab_image (&ig->igpax[idx],buffer);
    debug_printf ("igpax_grab_image[%d]: rc=%d\n",idx,rc);
    if (rc != 0) {
	/* GCS FIX: Return a non-fatal error code here */
	return 0;
    }

#if defined (ROTATE_PANEL_B)
    if (idx == 1) {
	unsigned short *b = (unsigned short*) buffer;
	unsigned short *s, *t, *e;  /* source, target, end */

	s = b;
	t = b + (HIRES_IMAGE_HEIGHT * HIRES_IMAGE_WIDTH) - 1;
	e = b + ((HIRES_IMAGE_HEIGHT * HIRES_IMAGE_WIDTH) / 2);
	while (s < e) {
	    register unsigned short tmp;
	    tmp = *s;
	    *s = *t;
	    *t = tmp;
	    ++s, --t;
        }
    }
#endif
    return 0;
}

static int
ise_igpax_grab_image (IgpaxInfo* igpax, unsigned char* buffer)
{
    int rc;
    int x_size = 2*1024;
    int y_size = 2*768;
    int npixels = x_size * y_size;
    char cmd = IGPAXCMD_GET_IMAGE;

    rc = ise_igpax_send_command (igpax,cmd);
    if (rc != 0) {
	debug_printf ("Server request for read image failed: rc=%d\n",rc);
	return 3;
    }
    rc = read (igpax->pipe_from_igpax[0], buffer, npixels*sizeof(unsigned short));
    if (rc != (int) (npixels*sizeof(unsigned short))) {
	debug_printf ("Server read image failed\n");
	return 4;
    }
    return 0;
}
#endif /* commentout */


/* -------------------------------------------------------------------------*
   Private functions
 * -------------------------------------------------------------------------*/
static void
ise_igpax_thread (void* arg)
{
    IgpaxInfo* igpax = (IgpaxInfo*) arg;
#if defined (USE_APC)
    SleepEx (INFINITE, TRUE);
#else
    while (1) {
        EnterCriticalSection (&igpax->crit_section);
        if (igpax->cmd_queue_len > 0) {
            ise_igpax_send_command_internal ((DWORD) igpax);
        }
        LeaveCriticalSection (&igpax->crit_section);
        SleepEx (100, FALSE);
    }
#endif
}

static VOID CALLBACK
ise_igpax_send_command_internal (DWORD arg)
{
    IgpaxInfo* igpax = (IgpaxInfo*) arg;
    int i, rc, cmd_rc;
    char cmd;

    /*  Remove a command from the queue */
    EnterCriticalSection (&igpax->crit_section);
    if (igpax->cmd_queue_err) {
	LeaveCriticalSection (&igpax->crit_section);
	return;
    }
    if (igpax->cmd_queue_len == 0) {
	LeaveCriticalSection (&igpax->crit_section);
	return;
    }
    cmd = igpax->cmd_queue[0];
    for (i = 0; i < igpax->cmd_queue_len-1; i++) {
	igpax->cmd_queue[i] = igpax->cmd_queue[i+1];
    }
    igpax->cmd_queue_len --;
    LeaveCriticalSection (&igpax->crit_section);

    /*  Write command to pipe */
    debug_printf ("Server: Sending command %c\n",cmd);
    rc = write (igpax->pipe_to_igpax[1], (char*) &cmd, sizeof(char));
    if (rc != sizeof(char)) {
	debug_printf ("Server write failed\n");
	igpax->cmd_queue_err = ISE_IGPAX_SERVER_WRITE_FAILED;
	return;
    }

    /*  Block, waiting for response */
    rc = read (igpax->pipe_from_igpax[0], (int*) &cmd_rc, sizeof(int));
    if (rc != sizeof(int)) {
	debug_printf ("Server read failed\n");
	igpax->cmd_queue_err = ISE_IGPAX_SERVER_READ_FAILED;
	return;
    }
    if (cmd_rc != 0) {
	debug_printf ("Server read response incorrect: %d\n",cmd_rc);
	igpax->cmd_queue_err = ISE_IGPAX_SERVER_INCORRECT_RESPONSE;
	return;
    }
}
