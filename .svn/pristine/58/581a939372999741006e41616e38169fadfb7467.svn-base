/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _lua_class_structs_h_
#define _lua_class_structs_h_

#include "plmscript_config.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

class Plm_image;

typedef struct lua_ss_struct lua_ss;
struct lua_ss_struct {
    std::string fn;
    Plm_image* ss_img;
};

PLMSCRIPT_C_API int register_lua_class_ss (lua_State *L);
PLMSCRIPT_C_API void init_ss_instance (lua_ss* lss);

#endif
