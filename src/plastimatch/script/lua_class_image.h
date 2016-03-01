/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _lua_class_image_h_
#define _lua_class_image_h_

#include "plmscript_config.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

class Plm_image;

typedef struct lua_image_struct lua_image;
struct lua_image_struct {
    std::string fn;
    Plm_image* pli;
};

PLMSCRIPT_C_API int register_lua_class_image (lua_State *L);
PLMSCRIPT_C_API void init_image_instance (lua_image* limg);

#endif
