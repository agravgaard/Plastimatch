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

#include "lua_classes.h"
#include "lua_class_structs.h"
#include "lua_util.h"
#include "plm_image.h"

/* Name of class as exposed to Lua */
#define THIS_CLASS LUA_CLASS_SS

/* Helpers */
void
init_ss_instance (lua_ss* lss)
{
    memset (lss->fn, '\0', sizeof(lss->fn));
    lss->ss_img = NULL;
}

/*******************************************************/
/* Object Methods                                      */
/*******************************************************/
static int
ss_info (lua_State *L)
{
    /* 1st arg should be "this" */
    lua_ss *lss = (lua_ss*)get_obj_ptr (L, THIS_CLASS, 1);

    lss->ss_img->print ();

    return 0;
}


static int
ss_load (lua_State *L)
{
    const char* fn = luaL_optlstring (L, 1, NULL, NULL);

    if (!fn) {
        fprintf (stderr, "error -- " LUA_CLASS_SS ".load() -- no file specified\n");
        return 0;
    }

    Plm_image* ss_img = plm_image_load (fn, PLM_IMG_TYPE_ITK_UCHAR);
    if (!ss_img) {
        fprintf (stderr, "error -- " LUA_CLASS_SS ".load() -- unable to load %s\n", fn);
        return 0;
    }

    lua_ss *lss = (lua_ss*)lua_new_instance (L, THIS_CLASS, sizeof(lua_ss));
    init_ss_instance (lss);

    strcpy (lss->fn, fn);
    lss->ss_img = ss_img;

    return 1;
}

static int
ss_save (lua_State *L)
{
    lua_ss *lss = (lua_ss*)get_obj_ptr (L, THIS_CLASS, 1);

    const char* fn = luaL_optlstring (L, 2, NULL, NULL);
    Plm_image *ss_img = lss->ss_img;

    if (!fn) {
        if (lss->fn[0] == '\0') {
            fprintf (stderr, "warning -- " LUA_CLASS_SS ":save() -- filename must be specified when saving derived images\n");
            return 0;
        } else {
            fn = lss->fn;
        }
    } else {
        /* "Save-As" new volume on disk */
        ss_img->save_image (fn);
        strcpy (lss->fn, fn);
    }

#if 0
    /* save using current format */
    if (lss->rtss->m_ss_img) {
        lss->rtss->save_ss_image (fn);
        lss->rtss->save_XXX (lss->fn);
    } 
#endif

    return 0;
}
/*******************************************************/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Object Actions                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Action: garbage collection */
static int
ss_action_gc (lua_State *L)
{
    lua_ss *lss = (lua_ss*)get_obj_ptr (L, THIS_CLASS, 1);
    delete lss->ss_img;
    return 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/






/* Object Creation */

/* methods table for object */
static const luaL_reg
ss_methods[] = {
  {"info",          ss_info},
  {"load",          ss_load},
  {"save",          ss_save},
  {0, 0}
};

/* metatable of actions */
static const luaL_reg
ss_meta[] = {
  {"__gc",       ss_action_gc},
  {0, 0}
};

int
register_lua_class_ss (lua_State *L)
{
    return register_lua_class (L, THIS_CLASS, ss_methods, ss_meta);
}
