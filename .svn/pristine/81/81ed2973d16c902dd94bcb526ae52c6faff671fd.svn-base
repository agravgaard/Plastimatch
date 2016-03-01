/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include "ise.h"
#include "ise_config.h"
#include "indico_info.h"

/* -------------------------------------------------------------------------*
    Global functions
 * -------------------------------------------------------------------------*/
void
init_indico_shmem (Indico_Info* ii)
{
    int cmd_rc = 0;

    /* Set up shared memory */
    ii->h_shmem = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, 
				     PAGE_READWRITE, 0, 
				     sizeof (Indico_Shmem), 
				     INDICO_SHMEM_STRING);
    if (!ii->h_shmem) {
	fprintf (stderr, "Error opening shared memory for indico\n");
	exit (1);
    }
    ii->shmem = (Indico_Shmem*) 
	    MapViewOfFile (ii->h_shmem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!ii->shmem) {
	fprintf (stderr, "Error mapping shared memory for panel\n");
	exit (1);
    }
}
