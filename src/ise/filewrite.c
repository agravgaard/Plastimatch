/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#endif
#include <stdlib.h>
#include "ise.h"
#include "ise_framework.h"
#include "filewrite.h"
#include "debug.h"

/* ---------------------------------------------------------------------------- *
    Global variables
 * ---------------------------------------------------------------------------- */
static char* output_dir_base_1 = "C:\\temp\\iris\\";
static char* output_dir_base_2 = "G:\\temp\\iris\\";

/* ---------------------------------------------------------------------------- *
    Function declarations
 * ---------------------------------------------------------------------------- */
static void filewrite_thread (void* v);
static void filewrite_write (FileWrite* fw, Frame* f, char* fn);
static void compose_output_dirs (FileWrite *fw);

/* ---------------------------------------------------------------------------- *
    Functions
 * ---------------------------------------------------------------------------- */
FileWrite* 
filewrite_init (IseFramework* ig)
{
    FileWrite *fw;

    fw = (FileWrite*) malloc(sizeof(FileWrite));
    if (!fw) return 0;

    /* Start up a single thread */
    fw->ig = ig;
    fw->thread_data[0].fw = fw;
    fw->end = 0;
    debug_printf ("FILEWRITE launching thread %d\n", 0);
    fw->threads[0] = (HANDLE) _beginthread (filewrite_thread, 0, (void*) &fw->thread_data[0]);

    return fw;
}

static void
compose_output_dir (char* target, char* base)
{
    int i, rc;

    rc = _access (base, 00);
    if (rc == -1) {
	rc = _mkdir (base);
	if (rc == -1) {
	    printf ("Error a\n");
	    exit (-1);
	}
    }

    for (i=0; i<10000; i++) {
	sprintf (target, "%s\\%04d", base, i);
	rc = _access (target, 00);
	if (rc == -1) {
	    break;
	}
    }
    if (i == 10000) {
	printf ("Error.\n");
	exit (-1);
    }
    rc = _mkdir (target);
    if (rc == -1) {
	printf ("Error too\n");
	exit (-1);
    }
}

static void
compose_output_dirs (FileWrite *fw)
{
    compose_output_dir (fw->output_dir_1, output_dir_base_1);
#if defined (PINGPONG)
    compose_output_dir (fw->output_dir_2, output_dir_base_2);
#endif
}

static void
compose_filename (char* fnbuf, FileWrite *fw, int idx, unsigned long id, double timestamp)
{
#if defined (PINGPONG)
    static int ping = 0;
    if (ping) {
        _snprintf (fnbuf, _MAX_PATH, "%s\\%d_%06d_%014.3f.raw", fw->output_dir_1, idx, id, timestamp);
    } else {
        _snprintf (fnbuf, _MAX_PATH, "%s\\%d_%06d_%014.3f.raw", fw->output_dir_2, idx, id, timestamp);
    }
    ping = !ping;
#else
    _snprintf (fnbuf, _MAX_PATH, "%s\\%d_%06d_%014.3f.raw", fw->output_dir_1, idx, id, timestamp);
#endif
}

static void
filewrite_thread (void* v)
{
    FWThreadData* data = (FWThreadData*) v;
    FileWrite* fw = data->fw;
    IseFramework* ig = fw->ig;
    int h, w;
    Frame* frame = 0;
    int i = 0;
    int did_compose_dirs = 0;

    ise_grab_get_resolution (ig, &h, &w);
    fw->imgsize = h * w * sizeof(unsigned short);

    while (1) {
	int idx, did_write;
	char fnbuf[_MAX_PATH];

	did_write = 0;
	for (idx = 0; idx < globals.ig.num_idx; idx++) {
	    if (fw->end) break;
	    frame = cbuf_get_next_writable_frame (&ig->cbuf[idx]);

	    if (fw->end) break;
	    if (frame) {
		if (!did_compose_dirs) {
		    did_compose_dirs = 1;
		    compose_output_dirs (fw);
		}
		globals.is_writing = 1;
		compose_filename (fnbuf, fw, idx, frame->id, frame->timestamp);
		filewrite_write (fw, frame, fnbuf);
		cbuf_mark_frame_written (&ig->cbuf[idx], frame);
		did_write = 1;
	    }
	}

	if (!did_write) {
	    globals.is_writing = 0;
	    globals.notify[0] = 1;
	    globals.notify[1] = 1;
	    Sleep (200);
	}
    }
}

int
filewrite_stop (FileWrite* fw)
{
    /* Wait for threads to finish */
    debug_printf ("FILEWRITE waiting for threads\n");

    fw->end = 1;
    WaitForMultipleObjects (1,fw->threads,TRUE,300);
#if defined (commentout)
    WaitForMultipleObjects (2,ig->threads,TRUE,300);
#endif
    debug_printf ("FILEWRITE threads done!\n");
    free (fw);
    return 0;
}

static void
filewrite_write (FileWrite* fw, Frame* f, char* fn)
{
    HANDLE h;
    DWORD caching_attribute;
    DWORD num_written;
    BOOL brc;
    unsigned short* buf = f->img;

    caching_attribute = FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH; /* 16.7 MB/sec */
    caching_attribute = FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED;    /* This fails */
    caching_attribute = FILE_FLAG_NO_BUFFERING;     /* 16.1 MB/sec */
    caching_attribute = 0;			    /* 9.8 MB/sec */
    caching_attribute = FILE_FLAG_WRITE_THROUGH;    /* 18.5 MB/sec */

    h = CreateFile(fn, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 
	FILE_ATTRIBUTE_NORMAL | caching_attribute, 0);
    if (h == INVALID_HANDLE_VALUE) {
	printf ("Had a problem with CreateFile().\n");
	exit (1);
    }
    brc = WriteFile (h, f->img, fw->imgsize, &num_written,  0);
    if (!brc) {
	printf ("Had a problem with WriteFile()\n");
	fflush (stdout);
	exit (1);
    }
    CloseHandle (h);
}
