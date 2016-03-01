/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmscript_config.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lua_classes.h"
#include "lua_class_image.h"
#include "lua_tty.h"
#include "lua_tty_commands.h"
//#include "lua_tty_commands_pcmd.h"
#include "lua_tty_commands_util.h"
#include "lua_tty_preview.h"
#include "lua_util.h"

#include "file_util.h"

static void do_tty_command_pwd (lua_State* L, int argc, char** argv);

/* JAS 2012.04.27
 *   pcmd interface has been disabled since the
 *   new plastimatch architecture places the
 *   plastimatch command line at the top of the link
 *   dependencies tree.
 */ 


/* displays commands available to user */
static void
do_tty_command_help (lua_State* L, int argc, char** argv)
{
    if (argc == 1) {
        print_command_table (tty_cmds, num_tty_cmds, 60, 3);
    } else {
#if 0
        if (!strcmp (argv[1], TTY_CMD_PCMD)) {
            print_command_table (pcmds, num_pcmds, 60, 3);
        }
#endif
        if (!strcmp (argv[1], TTY_CMD_CD)) {
            fprintf (stdout, "change current working directory\n");
        }
        else if (!strcmp (argv[1], TTY_CMD_RUN)) {
            fprintf (stdout, "execute a script from disk\n");
            fprintf (stdout, "  Usage: " TTY_CMD_RUN " script_name\n");
        }
        else if ( (!strcmp (argv[1], TTY_CMD_DIR)) ||
                  (!strcmp (argv[1], TTY_CMD_LS)) ) {
            fprintf (stdout, "get listing for current directory\n");
        }
        else if (!strcmp (argv[1], TTY_CMD_LIST)) {
            fprintf (stdout, "display allocated Plastimatch objects.\n");
            fprintf (stdout, "  Usage: " TTY_CMD_LIST " [type]\n\n");
            fprintf (stdout, "valid types:\n");
            print_command_table (lua_classes, num_lua_classes, 60, 3);
        }
    }

    fprintf (stdout, "\n");
}

/* change current working directory */
static void
do_tty_command_cd (lua_State* L, int argc, char** argv)
{
    int ret;
    char* path;

    /* if no arguments, just pwd */
    if (argc < 2) {
        do_tty_command_pwd (L, argc, argv);
        return;
    } else {
        path = argv[1];
    }

    ret = plm_chdir (path);

    if (ret == -1) {
        switch (errno) {
        case EACCES:
            fprintf (stdout, "cd -- permission denied\n");
            break;
        case ENAMETOOLONG:
            fprintf (stdout, "cd -- specified path exceeds allowed length\n");
            break;
        case ENOENT:
            fprintf (stdout, "cd -- specified directory not found\n");
            break;
        case ENOTDIR:
            fprintf (stdout, "cd -- specified path not a directory\n");
            break;
#if (UNIX)
        case ELOOP:
            fprintf (stdout, "cd -- encountered too many symbolic links\n");
            break;
#endif
        }
    }

}


/* run a lua script from within the tty environment */
static void
do_tty_command_run (lua_State* L, int argc, char** argv)
{
    /* run lua script */
    if (argc < 2) return;
    if (file_exists (argv[1])) {
        printf ("-- running script : %s\n\n", argv[1]);
        luaL_dofile (L, argv[1]);
    } else {
        printf ("unable to load script: %s\n", argv[1]);
    }
}

/* dispaly count and names of allocated Plastimatch objects */
static void
do_tty_command_list (lua_State* L, int argc, char** argv)
{
    if (argc < 2) {
        /* no arguments -- list everything */
        for (int i=0; i<num_lua_classes; i++) {
            list_vars_of_class (L, lua_classes[i]);
            printf ("\n");
        }
    } else {
        char* ct = argv[1]; /* class type */
        for (int i=0; i<num_lua_classes; i++) {
            if (!strcmp (ct, lua_classes[i])) {
                list_vars_of_class (L, lua_classes[i]);
                printf ("\n");
            }
        }
    }

}

/* print a directory listing */
static void
do_tty_command_ls (lua_State* L, int argc, char** argv)
{
    int n;
    const char** f_list;

    n = plm_get_dir_list (&f_list);

    if (n == -1) {
        fprintf (stdout, "unable to get directory listing\n");
        return;
    }
    sort_list ((char**)f_list, n);

    printf ("%i items\n", n);
    print_command_table (f_list, n, 60, 3);
}

static void
do_tty_command_preview (lua_State* L, int argc, char** argv)
{
#if (QT4_FOUND)
    lua_image* limg;
    char* img_obj_name;

    if (argc < 2) {
        return;
    } else {
        img_obj_name = argv[1];
    }

    limg = (lua_image*)get_obj_ptr_from_name (L, img_obj_name);
    if (limg) {
        preview_portal (limg->pli);
    } else {
        fprintf (stdout, "%s does not exist\n", img_obj_name);
    }
#else
    fprintf (stdout, "preview not available -- plastimatch not compiled against QT4.\n");
#endif
}

static void
do_tty_command_pwd (lua_State* L, int argc, char** argv)
{
    char* b = NULL;

    b = plm_getcwd (NULL, 0);

    printf ("%s\n", b);
    free (b);
}

/* main tty command parser. if you hit this, then
 * the Lua 'interpreter' is bypassed entirely */
void
do_tty_command (lua_State *L)
{
    char cmd[TTY_MAXINPUT];
    char** argv;
    int argc;

    memset (cmd, '\0', TTY_MAXINPUT * sizeof(char));
    strcpy (cmd, lua_tolstring (L, 1, NULL));
    lua_pop (L, 1);

    build_args (&argc, &argv, cmd);

    if (!strcmp (argv[0], TTY_CMD_HELP)) {
        do_tty_command_help (L, argc, argv);
    }
    else if (!strcmp (argv[0], TTY_CMD_CD)) {
        do_tty_command_cd (L, argc, argv);
    }
    else if (!strcmp (argv[0], TTY_CMD_DIR) ||
             !strcmp (argv[0], TTY_CMD_LS)) {
        do_tty_command_ls (L, argc, argv);
    }
    else if (!strcmp (argv[0], TTY_CMD_LIST)) {
        do_tty_command_list (L, argc, argv);
    }
#if 0
    else if (!strcmp (argv[0], TTY_CMD_PCMD)) {
        do_tty_command_pcmd (argc, argv);
    }
#endif
    else if (!strcmp (argv[0], TTY_CMD_PREVIEW)) {
        do_tty_command_preview (L, argc, argv);
    }
    else if (!strcmp (argv[0], TTY_CMD_PWD)) {
        do_tty_command_pwd (L, argc, argv);
    }
    else if (!strcmp (argv[0], TTY_CMD_RUN)) {
        do_tty_command_run (L, argc, argv);
    }
}
