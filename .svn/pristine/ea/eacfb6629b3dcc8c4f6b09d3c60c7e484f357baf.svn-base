/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include "isewatch.h"

HANDLE h_shared_mem;
struct shared_mem_struct* sms;

void
map_shared_memory (void)
{
    h_shared_mem = OpenFileMapping (FILE_MAP_ALL_ACCESS,   // read/write access
				    FALSE,                 // do not inherit the name
				    SHARED_MEM_NAME);
    if (!h_shared_mem) {
	fprintf (stderr, "Error opening shared memory for watchdog slave\n");
	exit (1);
    }
    sms = (struct shared_mem_struct*) MapViewOfFile (h_shared_mem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!sms) {
	fprintf (stderr, "Error mapping shared memory for watchdog process\n");
	exit (1);
    }
}

void
signal_watchdog (void)
{
    sms->timer++;
}
