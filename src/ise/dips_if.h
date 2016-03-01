/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _dips_if_h_
#define _dips_if_h_

#define _USE_32BIT_TIME_T 1
#include <time.h>

#ifndef _MSC_VER
#pragma pack(push)
#pragma pack(4)
#endif

typedef struct PANEL {
	int status;
	time_t time;
	int ale;
	int xs, ys, depth;
	short *pixel;
} *PANEL_PTR;

#ifndef _MSC_VER
#pragma pack(push)
#endif

/* panel status */
#define VALID	0x01	/* panel data is valid */
#define READ	0x02	/* panel data is read by other application */
#define DARK	0x04	/* other application request acquring dark field */

#endif
