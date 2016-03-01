/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _lua_tty_commands_pcmd_h_
#define _lua_tty_commands_pcmd_h_

#include "plmscript_config.h"

#if defined __cplusplus
extern "C" {
#endif

/* Exposed plastimatch command line commands */
#define PCMD_ADD             "add"
#define PCMD_ADJUST          "adjust"
#define PCMD_AUTOLABEL       "autolabel"
#define PCMD_AUTOLABEL_TRAIN "autolabel-train"
#define PCMD_COMPARE         "compare"
#define PCMD_COMPOSE         "compose"
#define PCMD_CONVERT         "convert"
#define PCMD_CROP            "crop"
#define PCMD_DIFF            "diff"
#define PCMD_DRR             "drr"
#define PCMD_DVH             "dvh"
#define PCMD_HEADER          "header"
#define PCMD_JACOBIAN          "jacobian"
#define PCMD_FILL            "fill"
#define PCMD_MASK            "mask"
#define PCMD_PROBE           "probe"
//#define PCMD_REGISTER        "register"
#define PCMD_RESAMPLE        "resample"
#define PCMD_SEGMENT         "segment"
#define PCMD_SLICE           "slice"
#define PCMD_THUMBNAIL       "thumbnail"
#define PCMD_STATS           "stats"
#define PCMD_SYNTH           "synth"
#define PCMD_WARP            "warp"
#define PCMD_XF_CONVERT      "xf-convert"
#define PCMD_XIO_DVH         "xio-dvh"

static const char* pcmds[] = {
    PCMD_ADD,
    PCMD_ADJUST,
    PCMD_AUTOLABEL,
    PCMD_AUTOLABEL_TRAIN,
    PCMD_COMPARE,
    PCMD_COMPOSE,
    PCMD_CONVERT,
    PCMD_CROP,
    PCMD_DIFF,
    PCMD_DRR,
    PCMD_DVH,
    PCMD_HEADER,
    PCMD_JACOBIAN,
    PCMD_FILL,
    PCMD_MASK,
    PCMD_PROBE,
//    PCMD_REGISTER,
    PCMD_RESAMPLE,
    PCMD_SEGMENT,
    PCMD_SLICE,
    PCMD_THUMBNAIL,
    PCMD_STATS,
    PCMD_SYNTH,
    PCMD_WARP,
    PCMD_XF_CONVERT,
    PCMD_XIO_DVH
};
static const int num_pcmds = sizeof (pcmds)/sizeof(char*);

PLMSCRIPT_C_API void do_tty_command_pcmd (int argc, char** argv);

#if defined __cplusplus
}
#endif

#endif
