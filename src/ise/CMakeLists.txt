##-----------------------------------------------------------------------------
##  Welcome to the Plastimatch "ISE" CMakeLists.txt file
##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
cmake_minimum_required (VERSION 2.6)
project (ise)

find_package (Advantech)
find_package (Bitflow)
find_package (MIL)
find_package (Paxscan)

##-----------------------------------------------------------------------------
##  Include directories
##-----------------------------------------------------------------------------
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (BEFORE ${CMAKE_BINARY_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/../fatm)

##-----------------------------------------------------------------------------
## QT configuration
##-----------------------------------------------------------------------------
if (QT4_FOUND AND ADVANTECH_FOUND)
  set (BUILD_ACQUIRE_4030E TRUE)
else ()
  set (BUILD_ACQUIRE_4030E TRUE)
endif ()
if (QT4_FOUND AND QT_QTSQL_FOUND)
  #if (VTK_FOUND AND VTK_USE_QT)
  set (BUILD_ISE_QT TRUE)
else ()
  set (BUILD_ISE_QT FALSE)
endif ()

if (${QT_QTNETWORK_FOUND})
  include_directories (${QT_QTNETWORK_INCLUDE_DIR})
endif ()


##-----------------------------------------------------------------------------
##  Source files
##-----------------------------------------------------------------------------
set (ISE_COMMON_SRC
  bitflow.c bitflow.h
  cbuf.c cbuf.h
  debug.c debug.h
  fileload.c fileload.h
  filewrite.c filewrite.h
  frame.c frame.h
  igpax.h
  indico_info.c indico_info.h
  ise_framework.c ise_framework.h
  ise.rc
  ise_error.h
  ise.c ise.h
  ise_gdi.c ise_gdi.h
  ise_gl.c ise_gl.h
  ise_gl_shader.c ise_gl_shader.h
  ise_globals.c ise_globals.h
  ise_igpax.c ise_igpax.h
  ise_main.c
  ise_ontrak.c ise_ontrak.h
  ise_rsc.h
  ise_sw_config.h
  ise_structs.h
  isewatch_client.c isewatch_client.h
  kill.cxx kill.h
  matrox_source.c matrox_source.h
  synthetic_source.c synthetic_source.h
  tracker.c tracker.h
  )

set (ISE_SRC
  ${ISE_COMMON_SRC}
  ise_resize.c ise_resize.h
  )

set (ACQUIRE_4030E_SRC
  acquire_4030e_main.cxx
  acquire_4030e_child.cxx acquire_4030e_child.h 
  acquire_4030e_parent.cxx acquire_4030e_parent.h
  acquire_4030e_window.cxx acquire_4030e_window.h
  Acquire_4030e_DlgControl.cxx Acquire_4030e_DlgControl.h
  advantech.cxx advantech.h
  aqprintf.cxx aqprintf.h 
  dips_panel.cxx dips_panel.h
  YK16GrayImage.cxx YK16GrayImage.h  
  dips_if.h
  kill.cxx kill.h
  varian_4030e.cxx varian_4030e.h 
  YKOptionSetting.cxx YKOptionSetting.h
  acquire_4030e_define.h
  )
if (BUILD_ACQUIRE_4030E)
  qt4_wrap_cpp (ACQUIRE_4030E_SRC 
    acquire_4030e_child.h
    acquire_4030e_parent.h
    acquire_4030e_window.h
    acquire_4030e_DlgControl.h
    )
  qt4_wrap_ui (ACQUIRE_4030E_SRC 
    acquire_4030e_window.ui
    acquire_4030e_panel.ui
    )
  qt4_add_resources (ACQUIRE_4030E_SRC 
    acquire_4030e.qrc
    )
endif ()

set (ISE_QT_SRC
  autosense.cxx
  cbuf.cxx
  debug.cxx
  fluoro_source.cxx
  frame.cxx
  his_io.cxx
  iqt_application.cxx
  iqt_main.cxx
  iqt_main_window.cxx 
  iqt_synth_settings.cxx
  iqt_tracker.cxx
  iqt_video_widget.cxx
  synthetic_source.cxx 
  synthetic_source_thread.cxx
  tracker_thread.cxx
  )
if (BUILD_ISE_QT)
  qt4_wrap_cpp (ISE_QT_SRC 
    iqt_application.h
    iqt_main_window.h 
    tracker_thread.h
    iqt_video_widget.h
    iqt_synth_settings.h
    synthetic_source_thread.h
    )
  qt4_wrap_ui (ISE_QT_SRC 
    iqt_main_window.ui
    iqt_synth_settings.ui
    )
endif ()

##-----------------------------------------------------------------------------
##  Configuration files
##-----------------------------------------------------------------------------
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ise_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/ise_config.h)

##-----------------------------------------------------------------------------
##  Include directories
##-----------------------------------------------------------------------------
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})
if (MIL_FOUND)
  include_directories (AFTER ${MIL_INCLUDE_DIR})
endif ()
if(BITFLOW_FOUND)
  include_directories (AFTER ${BITFLOW_INCLUDE_DIR})
endif ()
if (PAXSCAN_FOUND)
  include_directories (AFTER ${PAXSCAN_INCLUDE_DIR})
endif ()
if (ADVANTECH_FOUND)
  include_directories (AFTER ${ADVANTECH_INCLUDE_DIR})
endif ()

include_directories (AFTER "${CMAKE_SOURCE_DIR}/src/fatm/include")

##-----------------------------------------------------------------------------
##  Linking
##-----------------------------------------------------------------------------
if (MSVC80)
  link_directories ("C:/Program Files/Microsoft Visual Studio 8/VC/lib")
endif ()
if (MIL_FOUND)
  link_directories (${MIL_ROOT_DIR}/library/winnt/msc/dll)
endif ()
if (BITFLOW_FOUND)
  link_directories (${BITFLOW_ROOT_DIR}/lib)
endif ()

set (ACQUIRE_4030E_LDFLAGS "/subsystem:windows /ENTRY:mainCRTStartup")
 # Set compiler options.
set(variables
    CMAKE_C_FLAGS_DEBUG      
    CMAKE_C_FLAGS_RELEASE      
    CMAKE_CXX_FLAGS_DEBUG      
    CMAKE_CXX_FLAGS_RELEASE      
    )

link_directories ("${CMAKE_BINARY_DIR}/src/fatm")

##-----------------------------------------------------------------------------
##  Executable targets
##-----------------------------------------------------------------------------
if (COMMENTOUT)
  add_executable (ise WIN32 ${ISE_SRC})
  target_link_libraries (ise
    ${MIL_LIBRARIES}
    ${BITFLOW_LIBRARIES}
    AduHid
    cg cggl
    opengl32 glu32
    )
  install (TARGETS ise DESTINATION bin)
endif ()

if (BUILD_ISE_QT)
  set (ISE_QT_LIBRARIES ${QT_LIBRARIES} ${QT_QTSQL_LIBRARIES})
##  set (ISE_QT_LIBRARIES QVTK)  ## Hard-coded "for now"...
  if (ADVANTECH_FOUND)
    set (ISE_QT_LIBRARIES ${ISE_QT_LIBRARIES} ${ADVANTECH_LIBRARIES})
  endif ()
  if (BITFLOW_FOUND)
    set (ISE_QT_LIBRARIES ${ISE_QT_LIBRARIES} ${BITFLOW_LIBRARIES})
  endif ()
  if (MIL_FOUND)
    set (ISE_QT_LIBRARIES ${ISE_QT_LIBRARIES} ${MIL_LIBRARIES})
  endif ()
  if (PAXSCAN_FOUND)
    set (ISE_QT_LIBRARIES ${ISE_QT_LIBRARIES} ${PAXSCAN_LIBRARIES})
  endif ()
  if (NOT PLM_CONFIG_DISABLE_FATM)
    set (ISE_QT_LIBRARIES ${ISE_QT_LIBRARIES} fatm)
    if (FFTW_FOUND)
      set (ISE_QT_LIBRARIES ${ISE_QT_LIBRARIES} ${FFTW_LIBRARIES})
    endif ()
  endif ()
  plm_add_executable (ise_qt "${ISE_QT_SRC}" "${ISE_QT_LIBRARIES}" "" 
    ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
endif ()

set (ACQUIRE_4030E_LIBRARIES
  ${PAXSCAN_LIBRARIES} ${ADVANTECH_LIBRARIES} ${QT_LIBRARIES})

if (PAXSCAN_FOUND AND ADVANTECH_FOUND AND QT_FOUND)

	foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
    endforeach()
    
  plm_add_executable (acquire_4030e "${ACQUIRE_4030E_SRC}" 
    "${ACQUIRE_4030E_LIBRARIES}" "${ACQUIRE_4030E_LDFLAGS}"
    ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
endif ()

if (WIN32)
  set (DIPS_TEST_SRC 
    aqprintf.cxx aqprintf.h 
    dips_test.cxx
    dips_panel.cxx dips_panel.h)
  plm_add_executable (dips_test "${DIPS_TEST_SRC}" "" ""
    ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
endif ()

if (ADVANTECH_FOUND)
  set (ADVANTECH_TEST_SRC 
    advantech.cxx advantech.h 
    advantech_test.cxx)
  plm_add_executable (advantech_test "${ADVANTECH_TEST_SRC}" 
    "${ADVANTECH_LIBRARIES}" ""
    ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
endif ()

if (BITFLOW_FOUND)
  set (BITFLOW_TEST_SRC 
    bitflow.c bitflow.h
    bitflow_test.cxx
    debug.cXX debug.h)
  plm_add_executable (bitflow_test "${BITFLOW_TEST_SRC}" 
    "${BITFLOW_LIBRARIES}" ""
    ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
endif ()
