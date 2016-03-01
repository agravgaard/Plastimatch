/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmcli_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "pcmd_script.h"
#include "lua_class_image.h"
#include "lua_class_register.h"
#include "lua_class_stage.h"
#include "lua_class_structs.h"
#include "lua_class_xform.h"
#include "lua_cli_glue.h"
#if 0
#include "lua_iface_add.h"
#include "lua_iface_crop.h"
#include "lua_iface_mask.h"     /* also contains fill() */
#include "lua_iface_register.h"
#include "lua_iface_resample.h"
#include "lua_iface_synth.h"
#endif
#include "lua_tty.h"
#include "lua_util.h"

#include "file_util.h"

void
print_usage ()
{
    printf ("Usage: plastimatch script [ script_file | -i | - ]\n\n" \
            " script_file    execute specified script_file\n"        \
            " -i             run in interactive mode\n"              \
            " -              execute commands piped from stdin\n"    \
            "\n");
    exit (1);
}


/* JAS 2012.04.27
 * interfaces depricated in favor of classes */
#if 0
/* Register your LUA interface here */
static void
register_lua_interfaces (lua_State* L)
{
    lua_register (L, "add",      LUAIFACE_add);
    lua_register (L, "crop",     LUAIFACE_crop);
    lua_register (L, "mask",     LUAIFACE_mask);
    lua_register (L, "fill",     LUAIFACE_fill);
//    lua_register (L, "register", LUAIFACE_register);
    lua_register (L, "resample", LUAIFACE_resample);
    lua_register (L, "synth",    LUAIFACE_synth);
}
#endif

static void
register_lua_objects (lua_State* L)
{
    register_lua_class_image (L);
    register_lua_class_register (L);
    register_lua_class_stage (L);
    register_lua_class_ss (L);
    register_lua_class_xform (L);
}


/* Hook into plastmatch commandline */
void
do_command_script (int argc, char *argv[])
{
    lua_State *L;
    char *script_fn = NULL;
    bool tty_mode   = false;
    bool stdin_mode = false;

    if (!strcmp (argv[1], "script")) {
        if (argc > 2) {
            if (!strcmp (argv[2], "-i")) {
                tty_mode = true;
            }
            else if (!strcmp (argv[2], "-")) {
                stdin_mode = true;
            }
            else {
                script_fn = argv[2];
            }
        } else {
            print_usage ();
        }
    }

    L = lua_open();
    luaL_openlibs(L);
//    register_lua_interfaces (L);
    register_lua_objects (L);

    if (tty_mode) {
        do_tty (L);
    } else if (stdin_mode) {
        do_stdin (L);
    } else if (script_fn) {
        if (file_exists (script_fn)) {
            printf ("-- running script : %s\n\n", script_fn);
            luaL_dofile (L, script_fn);
        } else {
            printf ("unable to load script: %s\n", script_fn);
        }
    } else {
        print_usage ();
    }

    lua_close (L);
    printf ("\n[Script Terminated]\n\n");
}
