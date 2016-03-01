/* -------------------------------------------------------------------------*
    See COPYRIGHT for copyright information.
 * -------------------------------------------------------------------------*/
#ifndef __MATROX_SOURCE_H__
#define __MATROX_SOURCE_H__

#include "config.h"

#if (MIL_FOUND)
Ise_Error matrox_init (MatroxInfo* matrox, unsigned int mode);
Ise_Error matrox_open (MatroxInfo* matrox, unsigned int idx, 
		 unsigned int board_no, unsigned int mode,
		 unsigned long fps);
void matrox_grab_image (Frame* f, MatroxInfo* matrox, int idx, 
			int rotate_flag, int done);
void matrox_shutdown (MatroxInfo* matrox, int num_idx);
void matrox_prepare_grab (MatroxInfo* matrox, int idx);
Ise_Error matrox_probe (void);
#endif

#endif
