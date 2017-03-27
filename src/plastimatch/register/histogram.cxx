/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "file_util.h"
#include "histogram.h"
#include "logfile.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"

Histogram::Histogram (
    Mi_hist_type type,
    plm_long bins)
{
    this->type = type;
    this->bins = bins;
    this->offset = 0.f;
    this->big_bin = 0;
    this->delta = 0.f;
    this->keys = 0;
    this->key_lut = 0;
}

Histogram::~Histogram ()
{
    if (this->key_lut) {
        free (this->key_lut);
    }
}
