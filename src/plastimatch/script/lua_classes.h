/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */

/* This file just keeps a registry of all the Lua classes */

#ifndef _lua_classes_h_
#define _lua_classes_h_


#define LUA_CLASS_IMAGE    "Image"
#define LUA_CLASS_REGISTER "Register"
#define LUA_CLASS_SS       "Structs"
#define LUA_CLASS_XFORM    "XForm"

static const char* lua_classes[] = {
    LUA_CLASS_IMAGE,
    LUA_CLASS_REGISTER,
    LUA_CLASS_SS,
    LUA_CLASS_XFORM
};
static const int num_lua_classes = sizeof (lua_classes)/sizeof(char*);


#endif
