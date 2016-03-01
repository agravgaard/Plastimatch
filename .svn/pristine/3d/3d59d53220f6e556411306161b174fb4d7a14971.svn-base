/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
/* -------------------------------------------------------------------------*
   Ports 0 & 1 are redundant relays for the gating hardware.
   Port 3 is for the watchdog relay.  
   Port 7 is the relay that indicates if we are exposing 
   fluoro or not (sent to RPM for internal/external correlation).
 * -------------------------------------------------------------------------*/
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "AduHid.h"
#include "ise.h"
#include "debug.h"

void read_rpk0 (void* ontrak);
static void ise_ontrak_thread (void* v);

OntrakData* 
ise_ontrak_init (void)
{
    OntrakData *od;
    const int initial_count = 0;
    const int max_count = 1;

    od = (OntrakData*) malloc(sizeof(OntrakData));
    if (!od) {
	return 0;
    }

    od->bright_frame = 0;
    od->gate_beam = 0;
    od->semaphore = CreateSemaphore (0,	initial_count, max_count, 0);

    // Returns -1 for failure (e.g. device not plugged in)
    od->ontrak_device = OpenAduDevice(0);
    printf ("ONTRAK = %p\n", od->ontrak_device);
    if ((long) od->ontrak_device == -1) {
	printf ("Couldn't connect to ontrak device\n");
	return 0;
    }

    /* Start up a single thread */
    od->thread_data.od = od;
    debug_printf ("ONTRAK launching thread %d\n", 0);
    od->thread = (HANDLE) _beginthread (ise_ontrak_thread, 0, (void*) &od->thread_data);

    return od;
}

static void
ise_ontrak_thread (void* v)
{
    OntrakThreadData* data = (OntrakThreadData*) v;
    OntrakData* od = data->od;
    DWORD dwWaitResult;
    unsigned char command_bits;
    char command_word[6];

    while (1) {
	command_bits = 0;
	dwWaitResult = WaitForSingleObject (od->semaphore, INFINITE);

	switch (dwWaitResult) {
	case WAIT_OBJECT_0:
	    if (od->gate_beam) {
		command_bits |= 0x01;    /* Relay k0 */
		command_bits |= 0x02;    /* Relay k1 */
	    }
	    if (od->bright_frame) {
		command_bits |= 0x04;    /* Relay k2 */
		command_bits |= 0x80;    /* Relay k7 */
	    }
	break; 

	case WAIT_TIMEOUT:
	    /* Reset all relays on timeout */
	    /* GCS FIX: Need to log an error too */
	break;
	}

	sprintf (command_word, "MK%d", (int) command_bits);
	WriteAduDevice (od->ontrak_device, command_word, strlen(command_word), 0, 0);
    }
    CloseAduDevice(od->ontrak_device);
}

void
ise_ontrak_engage_relay (OntrakData* od, int gate_beam, int bright_frame)
{
    if (!od) return;
    od->gate_beam = gate_beam;
    od->bright_frame = bright_frame;
    if (!ReleaseSemaphore (od->semaphore, 1, 0)) {
	printf("ReleaseSemaphore error: %d\n", GetLastError());
    }
}

void
ise_ontrak_shutdown (OntrakData* od)
{
    if (!od) return;
    /* GCS FIX: Need to tell thread to abort */
    CloseHandle (od->semaphore);
    free (od);
}

void
read_rpk0 (void* ontrak)
{
    char sBuffer[8];
    memset (sBuffer, 0, sizeof(sBuffer));
    WriteAduDevice (ontrak, "rpk0", 4, 0, 0);
    ReadAduDevice (ontrak, sBuffer, 7, 0, 0);
    printf ("Relay value is %s\n", sBuffer);
}
