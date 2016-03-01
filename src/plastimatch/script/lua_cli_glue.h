/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _lua_cli_glue_h_
#define _lua_cli_glue_h_

#include "plmscript_config.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

/* Old CLI Glue */
PLMSCRIPT_C_API void lua_cli_glue_init (lua_State* L, int* argc, char*** argv);
PLMSCRIPT_C_API void lua_cli_glue_grow (lua_State* L, int n, int* argc, char*** argv);
PLMSCRIPT_C_API void lua_cli_glue_add (lua_State* L, char* arg, char** argv);
PLMSCRIPT_C_API void lua_cli_glue_solvent (lua_State* L, char** argv, int argn);

/* Helpers */
PLMSCRIPT_C_API int from_lua_count_struct_members (lua_State* L);
PLMSCRIPT_C_API int from_lua_getstring (lua_State* L, char* dest, const char* var_name);
PLMSCRIPT_C_API int from_lua_getint (lua_State* L, int* dest, const char* var_name);
PLMSCRIPT_C_API int from_lua_getfloat (lua_State* L, float* dest, const char* var_name);
PLMSCRIPT_C_API int from_lua_getint3 (lua_State* L, int* dest, const char* var_name);
PLMSCRIPT_C_API int from_lua_getfloat3 (lua_State* L, float* dest, const char* var_name);
PLMSCRIPT_C_API void replace_char (char from, char to, char* str);

#endif
