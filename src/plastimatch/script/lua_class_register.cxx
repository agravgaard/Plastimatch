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

#include "plmregister.h"

#include "lua_classes.h"
#include "lua_class_register.h"
#include "lua_class_xform.h"
#include "lua_util.h"

/* Name of class as exposed to Lua */
#define THIS_CLASS LUA_CLASS_REGISTER


/* Helpers */
void
init_register_instance (lua_register* lregister)
{
    memset (lregister->fn, '\0', sizeof (lregister->fn));
    lregister->regp = NULL;
    lregister->moving = NULL;
    lregister->fixed = NULL;
    lregister->moving_roi = NULL;
    lregister->fixed_roi = NULL;
}

/*******************************************************/
/* Object Methods                                      */
/*******************************************************/
static int
register_load (lua_State *L)
{
    const char* fn = luaL_optlstring (L, 1, NULL, NULL);

    if (!fn) {
        fprintf (stderr, "error -- register.load() -- no file specified\n");
        return 0;
    }

    Registration_parms *regp = new Registration_parms; //(Registration_parms*)malloc (sizeof(Registration_parms));
    if (regp->parse_command_file (fn) < 0) {
        fprintf (stderr, "error -- register.load() -- unable to load %s\n", fn);
        return 0;
    }

    lua_register *lreg = (lua_register*)lua_new_instance (L,
                                    THIS_CLASS,
                                    sizeof(lua_register));

    init_register_instance (lreg);

    strcpy (lreg->fn, fn);
    lreg->regp = regp;

    return 1;
}

/* returns a lua_xform to Lua */
static int
register_go (lua_State *L)
{
    lua_register *lreg = (lua_register*)get_obj_ptr (L, THIS_CLASS, 1);

    Registration_data regd;
    Registration_parms* regp;

    if (!lreg->moving) {
        fprintf (stderr, "warning -- register:go() -- moving image not specified\n");
        return 0;
    }

    if (!lreg->fixed) {
        fprintf (stderr, "warning -- register:go() -- fixed image not specified\n");
        return 0;
    }

    regp = lreg->regp;
    regd.moving_image = lreg->moving->pli;
    regd.fixed_image  = lreg->fixed->pli;
    if (lreg->moving_roi) { regd.moving_roi = lreg->moving_roi->ss_img; }
    if (lreg->fixed_roi)  { regd.fixed_roi  = lreg->fixed_roi->ss_img;  }
    /* Landmarks not yet implemented  - no class */

    lua_xform *lxf = (lua_xform*)lua_new_instance (L,
                                    LUA_CLASS_XFORM,
                                    sizeof(lua_xform));

    do_registration_pure (&(lxf->pxf), &regd, regp);

    return 1;
}
/*******************************************************/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Object Actions                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Action: garbage collection */
static int
register_action_gc (lua_State *L)
{
    lua_register *lreg = (lua_register*)get_obj_ptr (L, THIS_CLASS, 1);
    delete lreg->regp;
    return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/






/* Super Class Creation */

/* methods table for object */
static const luaL_reg
register_methods[] = {
    {"load",   register_load},
    {"go",     register_go},
    {0, 0}
};

/* metatable of actions */
static const luaL_reg
register_meta[] = {
    {"__gc",       register_action_gc},
    {0, 0}
};

/* glue to C struct */
static const lua_sc_glue
getters[] = {
    {"moving",      sc_get_ptr,  offsetof (lua_register, moving)       },
    {"fixed",       sc_get_ptr,  offsetof (lua_register, fixed)        },
    {"moving_mask", sc_get_ptr,  offsetof (lua_register, moving_roi)  },
    {"fixed_mask",  sc_get_ptr,  offsetof (lua_register, fixed_roi)   },
    {0, 0}
};

static const lua_sc_glue
setters[] = {
    {"moving",      sc_set_ptr,  offsetof (lua_register, moving)      },
    {"fixed",       sc_set_ptr,  offsetof (lua_register, fixed)       },
    {"moving_mask", sc_set_ptr,  offsetof (lua_register, moving_roi) },
    {"fixed_mask",  sc_set_ptr,  offsetof (lua_register, fixed_roi)  },
    {0, 0}
};


int
register_lua_class_register (lua_State *L)
{
    return register_lua_super_class (L, THIS_CLASS, register_methods, register_meta, getters, setters);
}
