/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _lua_util_h_
#define _lua_util_h_

#include "plmscript_config.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef int (*Xet_func) (lua_State *L, void *v);

typedef struct lua_sc_glue_struct lua_sc_glue;
struct lua_sc_glue_struct {
    const char *name;   /* member name */
    Xet_func func;      /* set/get function for member's type */
    size_t offset;      /* offset of member in the interfacing C struct */
};

PLMSCRIPT_C_API int sc_get_int (lua_State *L, void *v);
PLMSCRIPT_C_API int sc_set_int (lua_State *L, void *v);
PLMSCRIPT_C_API int sc_get_number (lua_State *L, void *v);
PLMSCRIPT_C_API int sc_set_number (lua_State *L, void *v);
PLMSCRIPT_C_API int sc_get_string (lua_State *L, void *v);
PLMSCRIPT_C_API int sc_set_string (lua_State *L, void *v);
PLMSCRIPT_C_API int sc_get_ptr (lua_State *L, void *v);
PLMSCRIPT_C_API int sc_set_ptr (lua_State *L, void *v);

PLMSCRIPT_C_API int lua_check_type (lua_State *L, const char* class_name, int index);
PLMSCRIPT_C_API void* lua_new_instance (lua_State *L, const char* class_name, size_t size);
PLMSCRIPT_C_API void* get_obj_ptr (lua_State *L, const char* class_name, int index);
PLMSCRIPT_C_API int register_lua_class (
    lua_State *L,
    const char* class_name,
    const luaL_reg* methods,
    const luaL_reg* metatable
);
PLMSCRIPT_C_API int
register_lua_super_class (
    lua_State *L,
    const char* class_name,
    const luaL_reg* methods,
    const luaL_reg* metatable,
    const lua_sc_glue* getters,
    const lua_sc_glue* setters
);



#endif
