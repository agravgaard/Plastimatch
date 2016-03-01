/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmscript_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lua_tty.h"
#include "lua_tty_commands_util.h"
#include "lua_util.h"

void
print_command_table (
    const char* cmds[], /* command array   */
    unsigned int nc,    /* # of commands   */
    unsigned int tw,    /* table width     */
    unsigned int sp     /* minimum spacing */
)
{
    unsigned int i;
    unsigned int c,w,n;     /* col, word, command # */
    size_t lc;              /* longest command      */

    lc = 0;
    for (i=0; i<nc; i++) {
        if (strlen (cmds[i]) > lc) {
            lc = strlen (cmds[i]);
        }
    }

    c=0;w=0;n=0;
    while (n<nc) {
        while (c<tw) {
            while (w<strlen(cmds[n])) {
                fprintf (stdout, "%c", cmds[n][w++]);
                c++;
            }
            w=0;
            while (w<(lc-strlen(cmds[n])+sp)) {
                fprintf (stdout, " ");
                w++;c++;
            }
            n++; w=0;
            if (n>=nc) break;
        }
        printf ("\n"); c=0;
    }

}

void
build_args (int* argc, char*** argv, char* cmd)
{
    int i;
    char tmp[TTY_MAXINPUT];
    char* token;

    /* get argc */
    strcpy (tmp, cmd);
    token = strtok (tmp, " ");
    *argc=0;
    while (token) {
        token = strtok (NULL, " ");
        (*argc)++;
    }
    *argv = (char**)malloc (*argc * sizeof(char*));

    /* populate argv */
    token = strtok (cmd, " ");
    (*argv)[0] = (char*)malloc (strlen(token)*sizeof(char));
    strcpy ((*argv)[0], token);
    for (i=1; i<*argc; i++) {
        token = strtok (NULL, " ");
        (*argv)[i] = (char*)malloc (strlen(token)*sizeof(char));
        strcpy ((*argv)[i], token);
    }
}

void
list_vars_of_class (lua_State* L, const char* class_name)
{
    int argc;
    char** argv;

    char b[9001]; // it's over 9000...
    memset (b, '\0', 9001*sizeof (char));
    lua_pushvalue (L, LUA_GLOBALSINDEX);
    lua_pushnil (L);
    while (lua_next (L, -2)) {
        if (lua_check_type (L, class_name, 3)) {
            if (b[0] == '\0') {
                strcpy (b, lua_tostring (L, -2));
            }
            else {
                strcat (b, " ");
                strcat (b, lua_tostring (L, -2));
            }
        }
        lua_pop (L, 1);
    }
    lua_pop (L, 1);

    fprintf (stdout, "[%s] ", class_name);
    if (b[0] != '\0') {
        build_args (&argc, &argv, b);
        fprintf (stdout, "%i item(s)\n", argc);
        print_command_table ((const char**)argv, argc, 60, 3);
    } else {
        fprintf (stdout, "0 items\n");
    }
}

void
sort_list (char** c, int n)
{
    int i, j;
    char* tmp;
    for (j=0; j<(n-1); j++) {
        for (i=0; i<(n-(1+j)); i++) {
            if (c[i][0] > c[i+1][0]) {
                tmp = c[i+1];
                c[i+1] = c[i];
                c[i] = tmp;
            }
        }
    }
}

/* given a variable name, returns ptr */
void*
get_obj_ptr_from_name (lua_State* L, const char* name)
{
    void* ptr = NULL;
    lua_pushvalue (L, LUA_GLOBALSINDEX);
    lua_pushnil (L);
    while (lua_next (L, -2)) {
        if (!strcmp (lua_tostring (L, 2), name)) {
            ptr = lua_touserdata (L, 3);
        }
        lua_pop (L, 1);
    }
    lua_pop (L, 1);
    return ptr;
}
