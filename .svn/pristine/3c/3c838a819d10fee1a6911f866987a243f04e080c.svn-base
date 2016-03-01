##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_script)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmscript_config.h.in
    ${PLM_BUILD_ROOT}/plmscript_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (AFTER ${LUA_INCLUDE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLMSCRIPT_LIBRARY_SRC
  lua_class_image.cxx lua_class_image.h
  lua_class_register.cxx lua_class_register.h
  lua_class_stage.cxx lua_class_stage.h
  lua_class_structs.cxx lua_class_structs.h
  lua_class_xform.cxx lua_class_xform.h
  lua_tty.cxx lua_tty.h
  lua_tty_commands.cxx lua_tty_commands.h
#  lua_tty_commands_pcmd.cxx lua_tty_commands_pcmd.h
  lua_tty_commands_util.cxx lua_tty_commands_util.h
  lua_util.cxx lua_util.h
  )

set (PLMSCRIPT_LIBRARY_DEPENDENCIES
  plmbase
  plmregister
  plmsegment
  plmsys
  plmutil
  ${QT_LIBRARIES}
  lua
  )
if (READLINE_FOUND)
  set (PLMSCRIPT_LIBRARY_DEPENDENCIES
    ${PLMSCRIPT_LIBRARY_DEPENDENCIES}
    ${READLINE_LIBRARY}
    )
endif()
if (TERMCAP_LIBRARY)
  set (PLMSCRIPT_LIBRARY_DEPENDENCIES
    ${PLMSCRIPT_LIBRARY_DEPENDENCIES}
    ${TERMCAP_LIBRARY}
    )
endif()
if (LIBDL_FOUND)
  set (PLMSCRIPT_LIBRARY_DEPENDENCIES ${PLMSCRIPT_LIBRARY_DEPENDENCIES} -ldl)
endif ()


##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: Qt
##-----------------------------------------------------------------------------
# JAS 2012.04.27
#   This Qt4 mess here is simply horrible ...note to self,
#   remember to fix this once we have all the qt code
#   building as plmqt
if (QT4_FOUND)
    qt4_wrap_cpp (PORTAL_MOC_SRC_HACK
      ../qt/cview_portal.h
    )
    set (PLMSCRIPT_LIBRARY_SRC
      ${PLMSCRIPT_LIBRARY_SRC}
      ${PORTAL_MOC_SRC_HACK}
      ../qt/cview_portal.cxx
      lua_tty_preview.cxx
    )
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmscript
  "${PLMSCRIPT_LIBRARY_SRC}" 
  "${PLMSCRIPT_LIBRARY_DEPENDENCIES}"
  ""
  "")
