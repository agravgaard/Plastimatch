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
#include "lobject.h"
}
#include "lua_util.h"

/* utility functions */
int
lua_check_type (lua_State *L, const char* class_name, int index)
{
    void *p = lua_touserdata (L, index);
    if (p) {
        if (lua_getmetatable (L, index)) {
            lua_getfield (L, LUA_REGISTRYINDEX, class_name);
            if (lua_rawequal(L, -1, -2)) {
                lua_pop(L, 2);
                return 1;
            }
            lua_pop (L, 1);
        }
        lua_pop (L, 1);
    }
    return 0;
}

void*
lua_new_instance (lua_State *L, const char* class_name, size_t size)
{
    void *tmp = (void*)lua_newuserdata (L, size);
    luaL_getmetatable (L, class_name);
    lua_setmetatable (L, -2);
    return tmp;
}

void*
get_obj_ptr (lua_State *L, const char* class_name, int index)
{
    void* ptr;
    luaL_checktype (L, index, LUA_TUSERDATA);
    ptr = (void*)luaL_checkudata (L, index, class_name);
    if (ptr == NULL) {
        luaL_typerror (L, index, class_name);
    }
    return ptr;
}

#if 0
static void
stack_dump (lua_State *L)
{
    const char* str;
    const char* typ;
    float num;

    for (int i=-1; i>-21; i--) {
        num = lua_tonumber (L, i);
        str = lua_tostring (L, i);
        typ = lua_typename (L, lua_type(L, i));
        printf ("[%i] %s \t %s \t %f \n", i, typ, str, num);
    }
}
#endif

/* --- Super Class Helpers -------------------------- */
int
sc_get_int (lua_State *L, void *v)
{
    lua_pushnumber (L, *(int*)v);
    return 1;
}

int
sc_set_int (lua_State *L, void *v)
{
    *(int*)v = luaL_checkint (L, 3);
    return 0;
}

int
sc_get_number (lua_State *L, void *v)
{
    lua_pushnumber (L, *(lua_Number*)v);
    return 1;
}

int
sc_set_number (lua_State *L, void *v)
{
    *(lua_Number*)v = luaL_checknumber (L, 3);
    return 0;
}

int
sc_get_string (lua_State *L, void *v)
{
    lua_pushstring(L, (char*)v );
    return 1;
}

int
sc_set_string (lua_State *L, void *v)
{
    strcpy ((char*)v, luaL_checkstring (L, 3));
    return 0;
}

int
sc_get_ptr (lua_State *L, void *v)
{
    lua_pushlightuserdata (L, *(void**)v);
    return 1;
}

int
sc_set_ptr (lua_State *L, void *v)
{
    *(void**)v = (size_t*)lua_touserdata (L, 3);
    return 0;
}


static void
super_class_glue_init (lua_State *L, const lua_sc_glue *scg)
{
    for (; scg->name; scg++) {
        lua_pushstring (L, scg->name);
        lua_pushlightuserdata (L, (void*)scg);
        lua_settable (L, -3);
    }
}

static int
super_class_call (lua_State *L)
{
//    stack_dump (L);
    /* for get: stack has userdata, index, lightuserdata */
    /* for set: stack has userdata, index, value, lightuserdata */
    lua_sc_glue* m = (lua_sc_glue*)lua_touserdata (L, -1);  /* member info */
    lua_pop (L, 1);                               /* drop lightuserdata */
    luaL_checktype (L, 1, LUA_TUSERDATA);
    return m->func(L, (void *)((char *)lua_touserdata(L, 1) + m->offset));
}


static int
super_class_index (lua_State *L)
{
    /* stack has userdata, index */
    lua_pushvalue (L, 2);                        /* dup index */
    lua_rawget (L, lua_upvalueindex(1));         /* lookup member by name */
    if (!lua_islightuserdata (L, -1)) {
        lua_pop (L, 1);                          /* drop value */
        lua_pushvalue (L, 2);                    /* dup index */
        lua_gettable (L, lua_upvalueindex(2));   /* else try methods */

        if (lua_isnil (L, -1)) {                 /* invalid member */
            luaL_error (L, "warning -- invalid member '%s'", lua_tostring(L, 2));
        }
        return 1;
    }
    return super_class_call (L);                 /* call get function */
}

static int
super_class_newindex (lua_State *L)
{
    /* stack has userdata, index, value */
    lua_pushvalue (L, 2);                     /* dup index */
    lua_rawget (L, lua_upvalueindex(1));      /* lookup member by name */
    if (!lua_islightuserdata (L, -1)) {       /* invalid member */
        luaL_error(L, "warning -- invalid member '%s'", lua_tostring(L, 2));
    }
    return super_class_call (L);              /* call set function */
}
/* -------------------------------------------------- */



/* create a lua class with custom methods */
int
register_lua_class (
    lua_State *L,
    const char* class_name,
    const luaL_reg* methods,
    const luaL_reg* metatable
)
{
    /* create methods table, add it to the globals */
    luaL_openlib (L, class_name, methods, 0);

    /* create metatable and add it to the Lua registry */
    luaL_newmetatable (L, class_name);

    /* fill metatable */
    luaL_openlib (L, 0, metatable, 0);

    /* direct calls to non-existant members to methods table */
    lua_pushliteral (L, "__index");
    lua_pushvalue (L, -3);               /* push methods table */
    lua_rawset (L, -3);                  /* metatable.__index = methods */

    /* protect our metatable */
    lua_pushliteral (L, "__metatable");
    lua_pushvalue (L, -3);               /* push methods table */
    lua_rawset (L, -3);                  /* metatable.__metatable = methods */

    lua_pop (L, 1);                      /* pop metatable */
    lua_pop (L, 1);                      /* pop methods */
    return 0;
}

/* create a lua class with custom methods
 * that also has members linking directly
 * to C struct members */
int
register_lua_super_class (
    lua_State *L,
    const char* class_name,
    const luaL_reg* methods,
    const luaL_reg* metatable,
    const lua_sc_glue* getters,
    const lua_sc_glue* setters
)
{
    int metatable_idx;
    int methods_idx;

    /* create methods table, & add it to the table of globals */
    luaL_openlib (L, class_name, methods, 0);
    methods_idx = lua_gettop (L);

    /* create metatable for your_t, & add it to the registry */
    luaL_newmetatable (L, class_name);
    luaL_openlib (L, 0, metatable, 0);  /* fill metatable */
    metatable_idx = lua_gettop (L);

    lua_pushliteral (L, "__metatable");
    lua_pushvalue (L, methods_idx);    /* dup methods table*/
    lua_rawset (L, metatable_idx);     /* hide metatable:
                                          metatable.__metatable = methods */
    lua_pushliteral (L, "__index");
    lua_pushvalue (L, metatable_idx);  /* upvalue index 1 */
    super_class_glue_init (L, getters);     /* fill metatable with getters */
    lua_pushvalue (L, methods_idx);    /* upvalue index 2 */
    lua_pushcclosure (L, super_class_index, 2);
    lua_rawset (L, metatable_idx);     /* metatable.__index = super_class_index */

    lua_pushliteral (L, "__newindex");
    lua_newtable (L);              /* table for members you can set */
    super_class_glue_init (L, setters);     /* fill with setters */
    lua_pushcclosure (L, super_class_newindex, 1);
    lua_rawset(L, metatable_idx);     /* metatable.__newindex = super_class_newindex */

    lua_pop (L, 1);                /* pop metatable */
    lua_pop (L, 1);                /* pop methods */
    return 0;
}
