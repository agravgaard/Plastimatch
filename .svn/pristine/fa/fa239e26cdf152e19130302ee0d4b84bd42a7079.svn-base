/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmscript_config.h"
#include "lua_tty_commands_pcmd.h"

#include "plmregister.h"

#include "pcmd_add.h"
#include "pcmd_adjust.h"
#include "pcmd_autolabel.h"
#include "pcmd_autolabel_train.h"
#include "pcmd_compare.h"
#include "pcmd_compose.h"
#include "pcmd_crop.h"
#include "pcmd_diff.h"
#include "pcmd_drr.h"
#include "pcmd_dvh.h"
#include "pcmd_xio_dvh.h"
#include "pcmd_mask.h"
#include "pcmd_header.h"
#include "pcmd_jacobian.h"
#include "pcmd_probe.h"
#include "pcmd_resample.h"
#include "pcmd_segment.h"
#include "pcmd_stats.h"
#include "pcmd_synth.h"
#include "pcmd_thumbnail.h"
#include "pcmd_warp.h"
#include "pcmd_xf_convert.h"

void
do_tty_command_pcmd (int argc, char** argv)
{
    char* command = argv[1];

    if (!strcmp (command, PCMD_ADD)) {
        do_command_add (argc, argv);
    }
    else if (!strcmp (command, PCMD_ADJUST)) {
        do_command_adjust (argc, argv);
    }
    else if (!strcmp (command, PCMD_AUTOLABEL)) {
        do_command_autolabel (argc, argv);
    }
    else if (!strcmp (command, PCMD_AUTOLABEL_TRAIN)) {
        do_command_autolabel_train (argc, argv);
    }
    else if (!strcmp (command, PCMD_COMPARE)) {
        do_command_compare (argc, argv);
    }
    else if (!strcmp (command, PCMD_COMPOSE)) {
        do_command_compose (argc, argv);
    }
    else if (!strcmp (command, PCMD_CONVERT)) {
        /* convert and warp are the same */
        do_command_warp (argc, argv);
    }
    else if (!strcmp (command, PCMD_CROP)) {
        do_command_crop (argc, argv);
    }
    else if (!strcmp (command, PCMD_DIFF)) {
        do_command_diff (argc, argv);
    }
    else if (!strcmp (command, PCMD_DRR)) {
        do_command_drr (argc, argv);
    }
    else if (!strcmp (command, PCMD_DVH)) {
        do_command_dvh (argc, argv);
    }
    else if (!strcmp (command, PCMD_HEADER)) {
        do_command_header (argc, argv);
    }
    else if (!strcmp (command, PCMD_JACOBIAN)) {
        do_command_jacobian (argc, argv);
    }
    else if (!strcmp (command, PCMD_FILL)) {
        /* fill and mask are the same */
        do_command_mask (argc, argv);
    }
    else if (!strcmp (command, PCMD_MASK)) {
        /* fill and mask are the same */
        do_command_mask (argc, argv);
    }
    else if (!strcmp (command, PCMD_PROBE)) {
        do_command_probe (argc, argv);
    }
#if 0
    else if (!strcmp (command, PCMD_REGISTER)) {
        do_command_register (argc, argv);
    }
#endif
    else if (!strcmp (command, PCMD_RESAMPLE)) {
        do_command_resample (argc, argv);
    }
    else if (!strcmp (command, PCMD_SEGMENT)) {
        do_command_segment (argc, argv);
    }
    else if (!strcmp (command, PCMD_SLICE)) {
        print_and_exit ("Error: slice command is now called thumbnail.\n");
    }
    else if (!strcmp (command, PCMD_THUMBNAIL)) {
        do_command_thumbnail (argc, argv);
    }
    else if (!strcmp (command, PCMD_STATS)) {
        do_command_stats (argc, argv);
    }
    else if (!strcmp (command, PCMD_SYNTH)) {
        do_command_synth (argc, argv);
    }
    else if (!strcmp (command, PCMD_WARP)) {
        /* convert and warp are the same */
        do_command_warp (argc, argv);
    }
    else if (!strcmp (command, PCMD_XF_CONVERT)) {
        do_command_xf_convert (argc, argv);
    }
    else if (!strcmp (command, PCMD_XIO_DVH)) {
        do_command_xio_dvh (argc, argv);
    }
}
