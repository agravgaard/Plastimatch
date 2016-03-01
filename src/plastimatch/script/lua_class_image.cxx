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
#include "itkAddImageFilter.h"
#if ITK_VERSION_MAJOR >= 4
#include "itkMultiplyImageFilter.h"
#else
#include "itkMultiplyByConstantImageFilter.h"
#endif

#include "lua_classes.h"
#include "lua_class_image.h"
#include "lua_class_structs.h"
#include "lua_util.h"
#include "plm_image.h"
#include "segment_body.h"

/* Name of class as exposed to Lua */
#define THIS_CLASS LUA_CLASS_IMAGE


/* Helpers */
void
init_image_instance (lua_image* limg)
{
    memset (limg->fn, '\0', sizeof(limg->fn));
    limg->pli = NULL;
}

/*******************************************************/
/* Object Methods                                      */
/*******************************************************/
static int
image_automask (lua_State *L)
{
    Segment_body automask;

    /* 1st arg should be "this" */
    lua_image *limg = (lua_image*)get_obj_ptr (L, THIS_CLASS, 1);

    /* 2nd arg should be threshold */
    float thres = (float)luaL_optnumber (L, 2, automask.m_lower_threshold);

    lua_ss *lss = (lua_ss*)lua_new_instance (L, LUA_CLASS_SS, sizeof(lua_ss));
    init_ss_instance (lss);
    lss->ss_img = new Plm_image;

    automask.img_in  = limg->pli;
    automask.img_out = lss->ss_img;
    automask.m_lower_threshold = thres;
    automask.do_segmentation ();

    lss->ss_img = automask.img_out;

    return 1;
}

static int
image_info (lua_State *L)
{
    /* 1st arg should be "this" */
    lua_image *limg = (lua_image*)get_obj_ptr (L, THIS_CLASS, 1);

    limg->pli->print ();

    return 0;
}

static int
image_load (lua_State *L)
{
    const char* fn = luaL_optlstring (L, 1, NULL, NULL);
    
    if (!fn) {
        fprintf (stderr, "error -- image.load() -- no file specified\n");
        return 0;
    }

    Plm_image* pli = plm_image_load (fn, PLM_IMG_TYPE_ITK_FLOAT);
    if (!pli) {
        fprintf (stderr, "error -- image.load() -- unable to load %s\n", fn);
        return 0;
    }

    lua_image *tmp = (lua_image*)lua_new_instance (L,
                                    THIS_CLASS,
                                    sizeof(lua_image));
    init_image_instance (tmp);

    strcpy (tmp->fn, fn);
    tmp->pli = pli;

    return 1;
}

static int
image_save (lua_State *L)
{
    lua_image *limg = (lua_image*)get_obj_ptr (L, THIS_CLASS, 1);

    const char* fn = luaL_optlstring (L, 2, NULL, NULL);
    Plm_image *pli = limg->pli;
    
    if (!fn) {
        if (limg->fn[0] == '\0') {
            fprintf (stderr, "warning -- image:save() -- filename must be specified when saving derived images\n");
            return 0;
        } else {
            /* Save over current volume on disk */
            pli->save_image (limg->fn);
        }
    } else {
        /* "Save-As" new volume on disk */
        pli->save_image (fn);
        strcpy (limg->fn, fn);
    }

    return 0;
}
/*******************************************************/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Object Actions                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* Action: garbage collection */
static int
image_action_gc (lua_State *L)
{
    lua_image *tmp = (lua_image*)get_obj_ptr (L, THIS_CLASS, 1);
//    fprintf (stderr, "debug -- releasing %s [%p]\n", tmp->fn, tmp);
    delete tmp->pli;
    return 0;
}

/* Action: multiplication on volumes */
static int
image_action_mul (lua_State *L)
{
    float factor;
    lua_image* limg;

    if (lua_isnumber (L, 1) && lua_isuserdata (L, 2)) {
        factor = lua_tonumber (L, 1);
        limg = (lua_image*)get_obj_ptr (L, THIS_CLASS, 2);
    } else if (lua_isnumber (L, 2) && lua_isuserdata (L, 1)) {
        factor = lua_tonumber (L, 2);
        limg = (lua_image*)get_obj_ptr (L, THIS_CLASS, 1);
    } else {
        fprintf (stderr, "warning -- image.__mul() -- cannot multiply two images: returning (nil)\n");
        return 0;
    }


#if ITK_VERSION_MAJOR >= 4
    typedef itk::MultiplyImageFilter<
        FloatImageType, FloatImageType, FloatImageType > MulFilterType;
#else
    typedef itk::MultiplyByConstantImageFilter< 
        FloatImageType, float, FloatImageType > MulFilterType;
#endif
    MulFilterType::Pointer multiply = MulFilterType::New();

    lua_image *out =
        (lua_image*)lua_new_instance (L, THIS_CLASS, sizeof(lua_image));

    init_image_instance (out);

    out->pli = limg->pli->clone();
    multiply->SetConstant (factor);
    multiply->SetInput (out->pli->itk_float());
    multiply->Update();
    out->pli->m_itk_float = multiply->GetOutput();

    return 1;
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++*/






/* Object Creation */

/* methods table for object */
static const luaL_reg
image_methods[] = {
  {"automask",      image_automask},
  {"info",          image_info},
  {"load",          image_load},
  {"save",          image_save},
  {0, 0}
};

/* metatable of actions */
static const luaL_reg image_meta[] = {
  {"__gc",       image_action_gc},
  {"__mul",      image_action_mul},
  {0, 0}
};

int
register_lua_class_image (lua_State *L)
{
    return register_lua_class (L, THIS_CLASS, image_methods, image_meta);
}
