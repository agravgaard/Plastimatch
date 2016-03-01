/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmscript_config.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lua_tty.h"
#include "lua_tty_commands.h"



static lua_State *globalL = NULL;
static const char *progname = TTY_PROGNAME;
static const char *tty_prompt  = TTY_PROMPT;
static const char *tty_prompt2 = TTY_PROMPT2;

/* SIGNAL HANDLERS */
static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}

/* SIGNAL HANDLERS */
static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook (globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

/* HELPER: error handling */
static void
l_message (const char *pname, const char *msg)
{
    if (pname) {
        fprintf(stderr, "%s: ", pname);
    }

    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}

static int
report (lua_State *L, int status)
{
    if (status && !lua_isnil (L, -1)) {
        const char *msg = lua_tostring (L, -1);

        if (msg == NULL) {
            msg = "(error object is not a string)";
        }

        l_message (progname, msg);
        lua_pop(L, 1);
    }
    return status;
}

static int
traceback (lua_State *L)
{
    /* if 'message' not a string, keep it intact */
    if (!lua_isstring(L, 1)) {
        return 1;
    }

    lua_getfield (L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable (L, -1)) {
        lua_pop (L, 1);
        return 1;
    }

    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop (L, 2);
        return 1;
    }

    lua_pushvalue (L, 1);    /* pass error message */
    lua_pushinteger (L, 2);  /* skip this function and traceback */
    lua_call (L, 2, 1);      /* call debug.traceback */
    return 1;
}


static int
do_call (lua_State *L, int narg, int clear)
{
    int status;
    int base = lua_gettop (L) - narg;  /* function index */

    /* push traceback function */
    lua_pushcfunction (L, traceback);

    /* put it under chunk and args */
    lua_insert (L, base);

    signal (SIGINT, laction);
    status = lua_pcall (L, narg, (clear ? 0 : TTY_MULTRET), base);
    signal (SIGINT, SIG_DFL);

    /* remove traceback function */
    lua_remove (L, base);

    /* force a complete garbage collection in case of errors */
    if (status != 0) {
        lua_gc (L, LUA_GCCOLLECT, 0);
    }

    return status;
}

static int
incomplete (lua_State *L, int status)
{
    if (status == LUA_ERRSYNTAX) {
        size_t lmsg;
        const char *msg = lua_tolstring (L, -1, &lmsg);
        const char *tp = msg + lmsg - (sizeof(TTY_QL("<eof>")) - 1);

        if (strstr (msg, TTY_QL("<eof>")) == tp) {
            lua_pop(L, 1);
            return 1;
        }
    }
    return 0;
}


static const char*
get_prompt (lua_State *L, int firstline)
{
    const char *p;

    lua_getfield (L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
    p = lua_tostring(L, -1);

    if (p == NULL) {
        p = (firstline ? tty_prompt : tty_prompt2);
    }

    lua_pop(L, 1);  /* remove global */
    return p;
}

static int
pushline (lua_State *L, int firstline)
{
    int i;
    char buffer[TTY_MAXINPUT];
    char *b = buffer;
    char tb[TTY_MAXINPUT];
    char *token;
    size_t l;
    const char *prmt = get_prompt (L, firstline);

    if (lua_readline (L, b, prmt) == 0) {
        return 0;  /* no input */
    }

    l = strlen (b);

    /* if line ends with newline, remove it */
    if (l > 0 && b[l-1] == '\n') {
        b[l-1] = '\0';
    }

    if (firstline && l>0) {
        /* check for TTY commands */
        strcpy (tb, b);
        token = strtok (tb, " ");
        for (i=0; i<num_tty_cmds; i++) {
            if (!strcmp (token, tty_cmds[i])) {
                lua_pushstring (L, b);
                return TTY_COMMAND;
            }
        }
        /* 'exit' command is special */
        if (!strcmp (b, TTY_CMD_EXIT)) {
            return 0;
        }
        /* make = a shortcut for 'return' */
        else if (b[0] == '=') {
            lua_pushfstring (L, "return %s", b+1);
        }
        /* normal lua command, just push it */
        else {
            lua_pushstring (L, b);
        }
    } else {
        lua_pushstring (L, b);
    }

    lua_freeline (L, b);
    return 1;
}

static int
loadline (lua_State *L)
{
    int status;
    lua_settop (L, 0);

    int ret = pushline (L, 1);
    if (!ret) {
        return -1;  /* no input */
    }
    else if (ret == TTY_COMMAND) {
        lua_saveline (L, 1);
        return TTY_COMMAND;
    }

    /* repeat until gets a complete line */
    for (;;) {
        status = luaL_loadbuffer (
                    L,
                    lua_tostring (L, 1),
                    lua_strlen (L, 1),
                    "=stdin"
                 );

        if (!incomplete (L, status)) {
            break;
        }

        if (!pushline (L, 0)) {
            return -1;
        }

        lua_pushliteral (L, "\n");  /* add a new line        */
        lua_insert (L, -2);         /* between the two lines */
        lua_concat (L, 3);          /* and join them         */
    }

    lua_saveline (L, 1);
    lua_remove (L, 1);  /* remove line */
    return status;
}

void
engine (lua_State *L, int status)
{
    switch (status) {
    case LUA_COMMAND:
        status = do_call (L, 0, 0);
        report (L, status);
        break;
    case TTY_COMMAND:
        do_tty_command (L);
        break;
    case LUA_ERRSYNTAX:
    default:
        report (L, status);
    }

    /* any result to print? */
    if (status == 0 && lua_gettop(L) > 0) {
        lua_getglobal(L, "print");
        lua_insert(L, 1);

        /* error handler */
        if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0) {
            l_message (
                progname,
                lua_pushfstring (
                    L,
                    "error calling " TTY_QL("print") " (%s)",
                    lua_tostring(L, -1)
                )
            );
        }
    }
}


void
do_tty (lua_State *L)
{
    int status;

    fprintf (stdout, TTY_BANNER);

    tty_prompt  = TTY_PROMPT;
    tty_prompt2 = TTY_PROMPT2;

    while ((status = loadline (L)) != -1) {
        engine (L, status);
    }

    /* clear stack */
    lua_settop(L, 0);
    fflush(stdout);
}

void
do_stdin (lua_State *L)
{
    /* hide prompts */
    tty_prompt  = "";
    tty_prompt2 = "";

    while (1) {
        engine (L, loadline (L));
    }

    /* clear stack */
    lua_settop(L, 0);
    fflush(stdout);
}
