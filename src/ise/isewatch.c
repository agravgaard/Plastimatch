/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include "config.h"
#include <stdio.h> 
#include <windows.h> 
#include "AduHid.h"
#include "isewatch.h"

HANDLE h_shared_mem;
struct shared_mem_struct* sms;
void* ontrak_device;

void
create_shared_memory (void)
{
    h_shared_mem = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, 
				    PAGE_READWRITE, 0, 
				    sizeof (struct shared_mem_struct), 
				    SHARED_MEM_NAME);
    if (!h_shared_mem) {
	fprintf (stderr, "Error opening shared memory for watchdog process\n");
	exit (1);
    }
    sms = (struct shared_mem_struct*) MapViewOfFile (h_shared_mem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!sms) {
	fprintf (stderr, "Error mapping shared memory for watchdog process\n");
	exit (1);
    }
    memset (sms, 0, sizeof(struct shared_mem_struct));
}

void
init_ontrak (void)
{
    // Returns -1 for failure (e.g. device not plugged in)
    ontrak_device = OpenAduDevice(0);
    if ((long) ontrak_device == -1) {
	printf ("Couldn't connect to ontrak device\n");
	exit (1);
    }
}

void
set_watchdog_relay (void)
{
    char command_word[4];
    sprintf (command_word, "SK3");
    WriteAduDevice (ontrak_device, command_word, strlen(command_word), 0, 0);
}

void
reset_watchdog_relay (void)
{
    char command_word[4];
    sprintf (command_word, "RK3");
    WriteAduDevice (ontrak_device, command_word, strlen(command_word), 0, 0);
}

int
main (int argc, char* argv[])
{
    int old_timer = -1;
    init_ontrak ();
    set_watchdog_relay ();
    create_shared_memory ();
    Sleep (5000);
    while (1) {
	int curr_timer = sms->timer;
	if (old_timer == curr_timer) {
	    reset_watchdog_relay ();
	    exit (1);
	}
	old_timer = curr_timer;
	Sleep (250);
    }
    return 0;
}
