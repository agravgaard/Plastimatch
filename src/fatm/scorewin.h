/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef SCOREWIN_H
#define SCOREWIN_H

#include "fatm.h"
#include "image.h"

typedef struct scorewin_struct Scorewin_Struct;
struct scorewin_struct {
    int sco_pt[2];       /* The position of the point within the score array */
    int idx_pt[2];       /* The position of the point within the signal search window */
};

typedef void (*Score_Function) (FATM_Options*, Scorewin_Struct*);

void scorewin_clip_pattern (FATM_Options* options);
void scorewin_malloc (FATM_Options* options);
void scorewin (FATM_Options* options, Score_Function score_fn);

#endif
