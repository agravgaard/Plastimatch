/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _lua_tty_commands_h_
#define _lua_tty_commands_h_

#include "plmscript_config.h"

#if defined __cplusplus
extern "C" {
#endif

/* Valid TTY command names */
#define TTY_CMD_CD      "cd"
#define TTY_CMD_DIR     "dir"
#define TTY_CMD_EXIT    "exit"
#define TTY_CMD_HELP    "help"
#define TTY_CMD_LS      "ls"
#define TTY_CMD_LIST    "list"
#define TTY_CMD_PCMD    "pcmd"
#define TTY_CMD_PREVIEW "preview"
#define TTY_CMD_PWD     "pwd"
#define TTY_CMD_RUN     "run"

static const char* tty_cmds[] = {
    TTY_CMD_CD,
    TTY_CMD_DIR,
    TTY_CMD_HELP,
    TTY_CMD_LS,
    TTY_CMD_LIST,
    TTY_CMD_PCMD,
    TTY_CMD_PREVIEW,
    TTY_CMD_PWD,
    TTY_CMD_RUN
};
static const int num_tty_cmds = sizeof (tty_cmds)/sizeof(char*);

PLMSCRIPT_C_API void do_tty_command (lua_State *L);

#if defined __cplusplus
}
#endif

#endif
