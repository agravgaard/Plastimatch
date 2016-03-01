/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _lua_class_xform_h_
#define _lua_class_xform_h_

#include "plmscript_config.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

class Xform;

typedef struct lua_xform_struct lua_xform;
struct lua_xform_struct {
    std::string fn;
    Xform* pxf;
};

PLMSCRIPT_C_API int register_lua_class_xform (lua_State *L);
PLMSCRIPT_C_API void init_xform_instance (lua_xform* lxf);

#endif
