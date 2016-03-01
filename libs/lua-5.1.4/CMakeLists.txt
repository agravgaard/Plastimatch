# Added by: James Shackleford
#     Date: Feb 28th, 2012
#
# Based on CMakeLists.txt from http://trac.assembla.com/lua
#  adapted for Plastimatch's need of only a static link lib
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(lua)

INCLUDE(CMakeDependentOption)
SET(CPACK_SET_DESTDIR TRUE)
INCLUDE(CPack)

SET(LUA_MAJOR_VERSION 5)
SET(LUA_MINOR_VERSION 1)
SET(LUA_PATCH_VERSION 4)

SET(LUA_VERSION ${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}.${LUA_PATCH_VERSION})
SET(LUA_COMPATIBILITY_VERSION ${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}.0)
SET(LUA_SOVERSION ${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}.0)

SET(WANTS_BUILD_STATIC_LIBRARY ON)
SET(WANTS_BUILD_FRAMEWORK OFF)
SET(LUA_LIBRARY_NAME lua)

SET(LUA_CORE
    ./src/lapi.c
    ./src/lcode.c
    ./src/ldebug.c
    ./src/ldo.c
    ./src/ldump.c
    ./src/lfunc.c
    ./src/lgc.c
    ./src/llex.c
    ./src/lmem.c
    ./src/lobject.c
    ./src/lopcodes.c
    ./src/lparser.c
    ./src/lstate.c
    ./src/lstring.c
    ./src/ltable.c 
    ./src/ltm.c 
    ./src/lundump.c
    ./src/lvm.c
    ./src/lzio.c
)

SET(LUA_STANDARD_LIBRARY
    ./src/lauxlib.c
    ./src/lbaselib.c
    ./src/ldblib.c
    ./src/liolib.c
    ./src/lmathlib.c
    ./src/loslib.c
    ./src/ltablib.c
    ./src/lstrlib.c
    ./src/loadlib.c
    ./src/linit.c
)

SET(PUBLIC_HEADERS
    ./src/lua.h 
    ./src/lualib.h
    ./src/lauxlib.h
    ./src/luaconf.h
    ./etc/lua.hpp
)
SET(RESOURCE_FILES
    ./etc/strict.lua
    ./etc/lua.ico
)

IF(APPLE)
    # Enable dyld for loading external libraries at runtime.
    #SET(LUA_C_FLAGS "${LUA_C_FLAGS} -DLUA_USE_MACOSX=1")
    # If you don't need pre-10.3 compatibility, DLOPEN is recommended instead.
    SET(LUA_C_FLAGS "${LUA_C_FLAGS} -DLUA_USE_POSIX -DLUA_USE_DLOPEN")
ENDIF(APPLE)

IF(CMAKE_SYSTEM MATCHES "Linux.*")
    # enable dlopen() and mkstemp() support, etc.
    SET(LUA_C_FLAGS "${LUA_C_FLAGS} -DLUA_USE_LINUX")
    # Lua uses libm's math functions and libdl's dlopen.
    SET(LUA_LINKER_FLAGS "m;dl;${LUA_LINKER_FLAGS}")
ENDIF(CMAKE_SYSTEM MATCHES "Linux.*")

### GCS 2012-10-07: The plastimatch plm_add_library macro expects the 
### target name to match the library name
#ADD_LIBRARY(lua_library_static STATIC ${PUBLIC_HEADERS} ${LUA_STANDARD_LIBRARY} ${LUA_CORE} ${RESOURCE_FILES})

#SET_TARGET_PROPERTIES(lua_library_static PROPERTIES OUTPUT_NAME "lua")
#SET_TARGET_PROPERTIES(lua_library_static PROPERTIES COMPILE_FLAGS "${LUA_C_FLAGS}")

## ADD_LIBRARY(lua STATIC ${PUBLIC_HEADERS} ${LUA_STANDARD_LIBRARY} ${LUA_CORE} ${RESOURCE_FILES})
## SET_TARGET_PROPERTIES(lua PROPERTIES COMPILE_FLAGS "${LUA_C_FLAGS}")

set (LUA_LIBRARY_SRC ${LUA_STANDARD_LIBRARY} ${LUA_CORE})
plm_add_static_library (lua "${LUA_LIBRARY_SRC}" 
    "" "" "")

SET_TARGET_PROPERTIES(lua PROPERTIES COMPILE_FLAGS "${LUA_C_FLAGS}")

# Setup properties for static library
IF(APPLE)
  #    SET_TARGET_PROPERTIES(lua_library_static PROPERTIES
  SET_TARGET_PROPERTIES (lua PROPERTIES
        VERSION ${LUA_VERSION}
        SOVERSION ${LUA_COMPATIBILITY_VERSION}
    )
ELSEIF(UNIX)
  #    SET_TARGET_PROPERTIES(lua_library_static PROPERTIES
  SET_TARGET_PROPERTIES (lua PROPERTIES
        VERSION ${LUA_VERSION}
        SOVERSION ${LUA_SOVERSION}
    )
ELSE(APPLE)
  #    SET_TARGET_PROPERTIES(lua_library_static PROPERTIES
  SET_TARGET_PROPERTIES (lua PROPERTIES
        VERSION ${LUA_VERSION}
        SOVERSION ${LUA_COMPATIBILITY_VERSION}
    )
ENDIF(APPLE)

# GCS 2012-08-28 This script used to have a section which tried to 
# set gcc flags for Mac universal binaries.  Universal binaries are 
# no longer supported by Apple, so I deleted this from the CMake script.

# This needs to be run very last so other parts of the scripts can take
# advantage of this.
IF(NOT LUA_CONFIG_HAS_BEEN_RUN_BEFORE)
    SET(LUA_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
ENDIF(NOT LUA_CONFIG_HAS_BEEN_RUN_BEFORE)
