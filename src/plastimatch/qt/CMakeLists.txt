##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_qt)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmqt_config.h.in
    ${PLM_BUILD_ROOT}/plmqt_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLASTIMATCH_QT_SRC
  # pqt_application.cxx pqt_application.h
  pqt_data_source_dialog.cxx pqt_data_source_dialog.h
  pqt_data_source_list_model.cxx pqt_data_source_list_model.h
  pqt_database.cxx pqt_database.h
  pqt_findscu.cxx pqt_findscu.h
  pqt_main.cxx
  pqt_main_window.cxx pqt_main_window.h
  pqt_patient_list_model.cxx pqt_patient_list_model.h
  )

set (CRYSTALVIEW_SRC
  cview_portal.cxx cview_portal.h
  cview_main.cxx cview_main.h
  )

if (PLM_CONFIG_BUILD_QT_PLUGINS)
  set (QT_DESIGNER_PLUGINS_SRC
    cview_portal.cxx cview_portal.h
    cview_portal_plugin.cxx cview_portal_plugin.h
    )
endif()

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: Qt
##-----------------------------------------------------------------------------
if (QT4_FOUND)
  qt4_wrap_cpp (PORTAL_MOC_SRC
    cview_portal.h
    )
  qt4_wrap_cpp (CRYSTALVIEW_MOC_SRC
    cview_main.h
    )
  if (PLM_CONFIG_BUILD_QT_PLUGINS)
    qt4_wrap_cpp (QT_DESIGNER_PLUGINS_MOC_SRC
      cview_portal.h
      cview_portal_plugin.h
    )
  endif()
  set (CRYSTALVIEW_SRC
    ${CRYSTALVIEW_SRC}
    ${CRYSTALVIEW_MOC_SRC}
    ${PORTAL_MOC_SRC}
    )

  if (QT_QTSQL_FOUND)
    qt4_wrap_cpp (PLASTIMATCH_QT_SRC 
      #    pqt_application.h
      pqt_data_source_dialog.h
      pqt_data_source_list_model.h
      pqt_main_window.h 
      pqt_patient_list_model.h
      )
    qt4_wrap_ui (PLASTIMATCH_QT_SRC 
      pqt_data_source_dialog.ui
      pqt_main_window.ui
      )
  endif ()
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
if (ITK_FOUND AND QT4_FOUND)
  set (CRYSTALVIEW_LIBRARIES ${PLASTIMATCH_LIBS} 
    ${QT_LIBRARIES} ${QT_QTSQL_LIBRARIES})
  plm_add_executable (cview "${CRYSTALVIEW_SRC}"
    "${CRYSTALVIEW_LIBRARIES}" "${PLASTIMATCH_LDFLAGS}"
    ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
endif ()
if (ITK_FOUND AND QT4_FOUND AND QT_QTSQL_FOUND)
  if (PLM_CONFIG_ENABLE_PLASTIMATCH_QT)
    set (PLASTIMATCH_QT_LIBRARIES ${PLASTIMATCH_LIBS} 
      ${QT_LIBRARIES} ${QT_QTSQL_LIBRARIES})
    plm_add_executable (plastimatch_qt "${PLASTIMATCH_QT_SRC}"
      "${PLASTIMATCH_QT_LIBRARIES}" "${PLASTIMATCH_LDFLAGS}" 
      ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
  endif ()
endif ()

##-----------------------------------------------------------------------------
##  BUILD QT DESIGNER PLUGINS
##-----------------------------------------------------------------------------
if (PLM_CONFIG_BUILD_QT_PLUGINS)
  add_definitions(-DQT_PLUGIN)
  add_definitions(-DQT_NO_DEBUG)
  add_definitions(-DQT_SHARED)
  add_definitions(-DQDESIGNER_EXPORT_WIDGETS)
  
  add_library(cview_portal_plugin SHARED
      ${QT_DESIGNER_PLUGINS_SRC}
      ${QT_DESIGNER_PLUGINS_MOC_SRC}
  )
  set (QT_DESIGNER_PLUGINS_LIBRARIES ${PLASTIMATCH_LIBS} ${QT_LIBRARIES})
  target_link_libraries(cview_portal_plugin ${QT_DESIGNER_PLUGINS_LIBRARIES})
  
  set_target_properties (cview_portal_plugin PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY designer)

  install (TARGETS cview_portal_plugin 
#      DESTINATION ${QT_PLUGINS_DIR}/designer
      DESTINATION ${PLM_INSTALL_LIB_DIR}/designer)

endif()
