/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (defined(_WIN32) || defined(WIN32))
#include <io.h>
#endif

#include "plm_int.h"
#include "plm_fwrite.h"
#include "print_and_exit.h"

#define LINELEN 128
#define MIN_SHORT -32768
#define MAX_SHORT 32767

#define WRITE_BLOCK (1024*1024)

/* GCS Jun 18, 2008.  When using MSVC 2005, large fwrite calls to files over 
    samba mount fails.  This seems to be a bug in the C runtime library.
    This function works around the problem by breaking up the large write 
    into many "medium-sized" writes. */
/* GCS Apr 4, 2014.  It seems this is a problem for writes > 4GB, 
   even 64-bit builds and MSVC 2010.  Sigh.
 */
#if _MSC_VER
static size_t 
fwrite_internal (void* buf, size_t size, size_t count, FILE* fp)
{
    size_t left_to_write = count * size;
    size_t cur = 0;
    char* bufc = (char*) buf;

    while (left_to_write > 0) {
	size_t this_write, rc;

	this_write = left_to_write;
	if (this_write > WRITE_BLOCK) this_write = WRITE_BLOCK;

	rc = fwrite (&bufc[cur], 1, this_write, fp);
	cur += rc;
	left_to_write -= rc;
	if (rc != this_write) {
	    break;
	}
    }
    return (count * size - left_to_write) / size;
}
#else
static size_t
fwrite_internal (void* buf, size_t size, size_t count, FILE* fp)
{
    return fwrite (buf, size, count, fp);
}
#endif

void 
plm_fwrite (void* buf, size_t size, size_t count, FILE* fp, 
    bool force_little_endian)
{
#if PLM_BIG_ENDIAN
    /* If we need to swap bytes, do it while writing into OS fwrite buffer */
    if (force_little_endian && size > 1) {
	uint8_t *cbuf = (uint8_t*) buf;
	if (size == 2) {
	    for (size_t i = 0; i < count; i++) {
		char tmp[2] = { cbuf[2*i+1], cbuf[2*i+1] };
		size_t rc = fwrite (tmp, 1, 2, fp);
		if (rc != 2) {
		    print_and_exit ("plm_fwrite error (rc = %u)\n", rc);
		}
	    }
	    return;
	} else if (size == 4) {
	    for (size_t i = 0; i < count; i++) {
		char tmp[4] = { cbuf[2*i+3], cbuf[2*i+1], 
				cbuf[2*i+1], cbuf[2*i+0] };
		size_t rc = fwrite (tmp, 1, 4, fp);
		if (rc != 4) {
		    print_and_exit ("plm_fwrite error (rc = %u)\n", rc);
		}
	    }
	    return;
	} else {
	    print_and_exit (
		"Error, plm_write encountered an unexpected input\n");
	}
    }
#endif

    /* Otherwise, we don't need to swap bytes.  Just do regular. */
    {
	size_t rc = fwrite_internal (buf, size, count, fp);
	if (rc != count) {
	    print_and_exit (
		"Error, plm_write write error (rc = %u)\n", rc);
	}
    }
}
