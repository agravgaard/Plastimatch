/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmscript_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "lua_class_stage.h"
#include "lua_util.h"

/* Name of class as exposed to Lua */
#define THIS_CLASS LUA_CLASS_STAGE


/* Helpers */
void
init_stage_instance (lua_stage* lstage)
{
    lstage->active = false;
    lstage->foo = 0;
    lstage->bar = 0;
}


/*******************************************************/
/* Object Methods                                      */
/*******************************************************/
static int
stage_new (lua_State *L)
{
    lua_stage *lstage =
        (lua_stage*)lua_new_instance (L, LUA_CLASS_STAGE, sizeof(lua_stage));

    init_stage_instance (lstage);

    return 1;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Object Actions                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Action: garbage collection */
static int
stage_action_gc (lua_State *L)
{
    return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/



/* Super Class Creation */

static const luaL_reg
stage_methods[] = {
    {"new",   stage_new},
    {0, 0}
};

/* metatable of actions */
static const luaL_reg
stage_meta[] = {
    {"__gc",   stage_action_gc},
    {0, 0}
};

/* glue to C struct */
static const lua_sc_glue
getters[] = {
    {"foo",     sc_get_int,     offsetof (lua_stage_struct, foo)    },
    {"bar",     sc_get_int,     offsetof (lua_stage_struct, bar)    },
    {0, 0}
};

static const lua_sc_glue
setters[] = {
    {"foo",     sc_set_int,     offsetof (lua_stage_struct, foo)    },
    {"bar",     sc_set_int,     offsetof (lua_stage_struct, bar)    },
    {0, 0}
};


int
register_lua_class_stage (lua_State *L)
{
    return register_lua_super_class (L, LUA_CLASS_STAGE, stage_methods, stage_meta, getters, setters);
}
