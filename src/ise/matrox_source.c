/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#include "config.h"
#if (MIL_FOUND)
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

#define M_PI 3.14159265358979323846

//#define lores_dcf_fn "irisgrab_lores_15fps.dcf"
#define lores_dcf_fn "irisgrab_lores_30fps.dcf"

#define hires_dcf_fn_7_5 "irisgrab_hires_7.5fps.dcf"
#define hires_dcf_fn_1 "irisgrab_hires_1fps.dcf"


static void matrox_clear_probe (MatroxInfo* matrox);

/* -------------------------------------------------------------------------*
   Public functions
 * -------------------------------------------------------------------------*/
Ise_Error
matrox_init (MatroxInfo* matrox, unsigned int mode)
{
    memset (matrox, 0, sizeof(MatroxInfo));
    if (!IS_REAL_FLUORO(mode)) {
	return ISE_SUCCESS;
    }
    MappAlloc(M_DEFAULT, &matrox->milapp);
    if (matrox->milapp == M_NULL) {
	return ISE_MATROX_MAPPALLOC_FAILED;
    } else {
        return ISE_SUCCESS;
    }
}

Ise_Error
matrox_open (MatroxInfo* matrox, unsigned int idx, 
	   unsigned int board_no, unsigned int mode,
	   Framerate fps)
{
    MIL_ID milsys;
    MIL_ID mildig;
    MIL_ID milimg0, milimg1;
    long dig_num;
    Ise_Error rc;

    if (board_no == 0) {
	dig_num = M_DEV0;
    } else if (board_no == 1) {
	dig_num = M_DEV1;
    } else {
	return ISE_ERR_INVALID_PARM;
    }

    if (IS_REAL_FLUORO(mode)) {
	MsysAlloc(M_DEF_SYSTEM_TYPE, board_no, M_DEFAULT, &milsys);
	if (milsys == M_NULL) return 1;
    }

    switch (mode) {
    case ISE_IMAGE_SOURCE_MATROX_LORES_FLUORO:
        MdigAlloc(milsys, board_no, lores_dcf_fn, M_DEFAULT, &mildig);
        if (mildig == M_NULL) goto error_1;
	break;
    case ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO:
	switch (fps) {
	case ISE_FRAMERATE_1_FPS:
	    MdigAlloc(milsys, board_no, hires_dcf_fn_1, M_DEFAULT, &mildig);
	    break;
	case ISE_FRAMERATE_7_5_FPS:
	    MdigAlloc(milsys, board_no, hires_dcf_fn_7_5, M_DEFAULT, &mildig);
	    break;
	default:
	    rc = ISE_ERR_INVALID_PARM;
	    goto error_1;
	}
        if (mildig == M_NULL) {
	    rc = ISE_MATROX_MDIGALLOC_FAILED;
	    goto error_1;
	}
	break;
    default:
	return ISE_ERR_INVALID_PARM;
    }

    /* Set to double-buffered grabbing */
    MdigControl(mildig, M_GRAB_MODE, M_ASYNCHRONOUS);

    matrox->mtx_size_x = (unsigned long) MdigInquire(mildig, M_SIZE_X, M_NULL);
    matrox->mtx_size_y = (unsigned long) MdigInquire(mildig, M_SIZE_Y, M_NULL);
    MbufAlloc2d(milsys, matrox->mtx_size_x, matrox->mtx_size_y, M_UNSIGNED+16L, 
	M_IMAGE+M_GRAB, &milimg0);
    if (milimg0 == M_NULL) goto error_2;
    MbufAlloc2d(milsys, matrox->mtx_size_x, matrox->mtx_size_y, M_UNSIGNED+16L, 
	M_IMAGE+M_GRAB, &milimg1);
    if (milimg1 == M_NULL) goto error_3;

    matrox->milsys[idx] = milsys;
    matrox->mildig[idx] = mildig;
    matrox->milimg[idx][0] = milimg0;
    matrox->milimg[idx][1] = milimg1;
    return ISE_SUCCESS;

error_3:
    MbufFree(milimg0);
error_2:
    MdigFree(mildig);
error_1:
    MsysFree(milsys);
    return rc;
}

/* Return 1 if matrox hardware exists */
int
matrox_probe (void)
{
    Ise_Error rc;
    MatroxInfo matrox;
    unsigned int mode = ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO;
    if (matrox_init (&matrox, mode) != ISE_SUCCESS) {
	return 0;
    }

    MappControl (M_ERROR, M_PRINT_DISABLE);
    rc = matrox_open (&matrox, 0, 0, ISE_IMAGE_SOURCE_MATROX_HIRES_FLUORO, ISE_FRAMERATE_7_5_FPS);
    MappControl (M_ERROR, M_PRINT_ENABLE);
    if (rc == ISE_SUCCESS) {
	matrox_clear_probe (&matrox);
	return 1;
    } else {
	MappFree(matrox.milapp);
	return 0;
    }
}

/* Copy a frame from the kernel buffer into the user buffer */
void 
matrox_copy_image (Frame* f, MatroxInfo* matrox, unsigned long idx, int rotate_flag)
{
    int i, j;
    unsigned int sr, dr, r, sc, dc, c;
    int rc;
    int active = matrox->active[idx];
    MIL_ID milimg = matrox->milimg[idx][active];
    unsigned short* host_address;
    int n = matrox->mtx_size_x*matrox->mtx_size_y;

    rc = MbufInquire (milimg, M_HOST_ADDRESS, (void*)&host_address);
    switch (rotate_flag) {
    case 3:
	/* Flip top to bottom */
	for (sr = 0, dr = matrox->mtx_size_y - 1; sr < matrox->mtx_size_y; sr++, dr--) {
	    for (c = 0; c < matrox->mtx_size_x; c++) {
		f->img[dr*matrox->mtx_size_x+c] = host_address[sr*matrox->mtx_size_x+c];
	    }
	}
	break;
    case 2:
	/* Flip left to right */
	for (r = 0; r < matrox->mtx_size_y; r++) {
	    for (sc = 0, dc = matrox->mtx_size_x - 1; sc < matrox->mtx_size_x; sc++, dc--) {
		f->img[r*matrox->mtx_size_x+dc] = host_address[r*matrox->mtx_size_x+sc];
	    }
	}
	break;
    case 1:
	/* Do 180 degree rotation */
	for (i = 0, j = n-1; i < n; i++, j--) {
	    f->img[i] = host_address[j];
	    //f->img[i] = ((long) host_address[j] << 4 & 0x0FFFF);
	}
	break;
    case 0:
    default:
	/* Just copy */
	memcpy (f->img, host_address, 
		sizeof(unsigned short)*matrox->mtx_size_x*matrox->mtx_size_y);
	break;
    }
}

void
matrox_prepare_grab (MatroxInfo* matrox, int idx)
{
    MIL_ID dig = matrox->mildig[idx];
    MIL_ID* img = &matrox->milimg[idx][0];

#if defined (commentout)
    /* For some reason this fails if I do this after allocating a large CBUF */
    MdigControl(dig, M_GRAB_MODE, M_ASYNCHRONOUS);
#endif
    MdigGrab(dig, img[0]);
    matrox->active[idx] = 0;
}

void
matrox_grab_image (Frame* f, MatroxInfo* matrox, int idx, int rotate_flag, int done)
{
    MIL_ID dig = matrox->mildig[idx];
    MIL_ID* img = &matrox->milimg[idx][0];
    int next_active = !(matrox->active[idx]);
    long rc;
    char error_string[M_ERROR_MESSAGE_SIZE];

    if (done) {
	MdigGrabWait (dig, M_GRAB_END);
    } else {
	MdigGrab (dig, img[next_active]);   // need next active...
	rc = MappGetError (M_CURRENT+M_THREAD_CURRENT, error_string);
	if (rc != M_NULL_ERROR) {
	    debug_printf ("MIL ERROR (%d) %s\n", rc, error_string);
	}
    }
    matrox_copy_image (f, matrox, idx, rotate_flag);
    matrox->active[idx] = next_active;
}

void
matrox_clear_probe (MatroxInfo* matrox)
{
    int idx = 0;
    debug_printf ("Gonna matrox_shutdown()\n");
    debug_printf ("Gonna MbufFree(%d,%d)\n",idx,0);
    MbufFree(matrox->milimg[idx][0]);
    debug_printf ("Gonna MbufFree(%d,%d)\n",idx,1);
    MbufFree(matrox->milimg[idx][1]);
    debug_printf ("Gonna MdigFree(%d)\n",idx);
    MdigFree(matrox->mildig[idx]);
    debug_printf ("Gonna MsysFree(%d)\n",idx);
    MsysFree(matrox->milsys[idx]);
    debug_printf ("Gonna MappFree\n");
    MappFree(matrox->milapp);
    debug_printf ("Done with matrox_shutdown()\n");
}

void
matrox_shutdown (MatroxInfo* matrox, int num_idx)
{
    int idx;

    debug_printf ("Gonna matrox_shutdown()\n");
    for (idx = 0; idx < num_idx; idx++) {
	debug_printf ("Gonna MbufFree(%d,%d)\n",idx,0);
	MbufFree(matrox->milimg[idx][0]);
	debug_printf ("Gonna MbufFree(%d,%d)\n",idx,1);
	MbufFree(matrox->milimg[idx][1]);
	debug_printf ("Gonna MdigFree(%d)\n",idx);
	MdigFree(matrox->mildig[idx]);
	debug_printf ("Gonna MsysFree(%d)\n",idx);
	MsysFree(matrox->milsys[idx]);
    }
    debug_printf ("Gonna MappFree\n");
    MappFree(matrox->milapp);
    debug_printf ("Done with matrox_shutdown()\n");
}
#endif /* MIL_FOUND */
