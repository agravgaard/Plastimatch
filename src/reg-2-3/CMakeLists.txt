
# N-way 2D/3D Registration (reg23)
# A tool for GPU-powered N-way intensity-based 2D/3D registration based on the
# NReg2D/3D core enriched with a Qt-based GUI.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(COMMAND cmake_policy)
  IF("${CMAKE_VERSION}" VERSION_GREATER "2.6" OR "${CMAKE_VERSION}" VERSION_EQUAL "2.6")  
  	CMAKE_POLICY(SET CMP0003 NEW)
  ENDIF("${CMAKE_VERSION}" VERSION_GREATER "2.6" OR "${CMAKE_VERSION}" VERSION_EQUAL "2.6")
  IF("${CMAKE_VERSION}" VERSION_GREATER "2.8") 
  	CMAKE_POLICY(SET CMP0015 NEW) # NOT AVAILABLE IN 2.8.0 !!!
  ENDIF("${CMAKE_VERSION}" VERSION_GREATER "2.8")
ENDIF(COMMAND cmake_policy)

##########################
PROJECT(REG23 CXX)
##########################

SET(BINNAME reg23)

### EXTERNAL LIBRARIES ###

# Require at least ITK 3.20
SET(ITK_MIN_VERSION "3.20")
FIND_PACKAGE(ITK) 
IF(NOT ITK_FOUND)
  MESSAGE(SEND_ERROR "ITK ${ITK_MIN_VERSION} not found.  Please set the ITK_DIR variable.")
ELSE(NOT ITK_FOUND)
  # The version check fails with cmake 2.8.4+linux, manually perform check
  IF("${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}" VERSION_LESS ${ITK_MIN_VERSION})
    MESSAGE(SEND_ERROR "ITK ${ITK_MIN_VERSION} not found (version ${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR} provided).  Please set the ITK_DIR variable.")
  ELSE("${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}" VERSION_LESS ${ITK_MIN_VERSION})
    INCLUDE(${ITK_USE_FILE})
  ENDIF("${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}" VERSION_LESS ${ITK_MIN_VERSION})
ENDIF(NOT ITK_FOUND)

# Require at least VTK 5.6
SET(VTK_MIN_VERSION "5.6")
FIND_PACKAGE(VTK) # NOTE: CMake appears to fail on checking VTK minimum version (at least on win7); ${VTK_MIN_VERSION})
IF(NOT VTK_FOUND)
  # Only give an error if not building within plastimatch
  IF(NOT PLM_BUILD_ORAIFUTILS)
    MESSAGE(SEND_ERROR "VTK ${VTK_MIN_VERSION} not found.  Please set the VTK_DIR variable.")
  ENDIF(NOT PLM_BUILD_ORAIFUTILS)
ELSE(NOT VTK_FOUND)
  # The version check fails with cmake 2.8.4+linux/windows7, manually perform check
  IF("${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}" VERSION_LESS ${VTK_MIN_VERSION})
    MESSAGE(SEND_ERROR "VTK ${VTK_MIN_VERSION} not found (version ${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR} provided).  Please set the VTK_DIR variable.")
  ELSE("${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}" VERSION_LESS ${VTK_MIN_VERSION})
    INCLUDE(${VTK_USE_FILE})
  ENDIF("${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}" VERSION_LESS ${VTK_MIN_VERSION})  
ENDIF(NOT VTK_FOUND)

# Require Qt 4.6.0 or later
SET(QT_MIN_VERSION "4.6.0")
FIND_PACKAGE(Qt4 ${QT_MIN_VERSION})
IF(NOT QT4_FOUND)
  MESSAGE(SEND_ERROR "Qt ${QT_MIN_VERSION} or greater not found.  Please set the QT_QMAKE_EXECUTABLE variable.")
ELSE(NOT QT4_FOUND)
  INCLUDE(${QT_USE_FILE})
ENDIF(NOT QT4_FOUND)

# Require ORAIFUTILS 1.1.9
SET(ORAIFUTILS_MIN_VERSION "1.1.9")
FIND_PACKAGE(ORAIFUTILS ${ORAIFUTILS_MIN_VERSION} NO_MODULE REQUIRED HINTS ${ORAIFUTILS_BINARY_DIR})
IF(NOT ORAIFUTILS_FOUND)
  MESSAGE(SEND_ERROR "ORAIFUTILS library ${ORAIFUTILS_MIN_VERSION} or greater not found. Please set the ORAIFUTILS_DIR variable.")
ELSE(NOT ORAIFUTILS_FOUND)
  INCLUDE_DIRECTORIES(${ORAIFUTILS_INCLUDE_DIRS})
ENDIF(NOT ORAIFUTILS_FOUND)

message (STATUS "ORAIFUTILS_INCLUDE_DIRS = ${ORAIFUTILS_INCLUDE_DIRS}")

### DEFINITIONS ###

# Build static libraries by default
# Build ONLY static libraries in MSVC because no exports are defined yet
# FIXME: Add __declspec(dllexport) for MSVC SHARED library support
SET(REG23_LIBRARY_TYPE STATIC)
IF(BUILD_SHARED_LIBS AND NOT MSVC)
  SET(REG23_LIBRARY_TYPE SHARED)
ELSE(BUILD_SHARED_LIBS AND NOT MSVC)
  IF(MSVC)
    MESSAGE(STATUS "Building REG23 with SHARED libs in MSVC is currently not supported. -> Building STATIC libs.")
  ENDIF(MSVC)
ENDIF(BUILD_SHARED_LIBS AND NOT MSVC)

# Build unit tests
OPTION(REG23_BUILD_TESTING "Build REG23 unit tests for regression tests and automatic software verification." OFF)
IF(REG23_BUILD_TESTING)
  ENABLE_TESTING()
ENDIF(REG23_BUILD_TESTING)

# Directories
SET(REG23_CORE_DIR core)
SET(REG23_CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${REG23_CORE_DIR})
SET(REG23_CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${REG23_CORE_DIR})
SET(REG23_GUI_DIR gui)
SET(REG23_GUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${REG23_GUI_DIR})
SET(REG23_GUI_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${REG23_GUI_DIR})

INCLUDE_DIRECTORIES(
  ${REG23_CORE_SOURCE_DIR}
  ${REG23_GUI_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}         # For headers produced by UIC
  ${REG23_GUI_BINARY_DIR}  # For headers produced by UIC
)

# CORE library
ADD_SUBDIRECTORY(${REG23_CORE_SOURCE_DIR})
# GUI library
ADD_SUBDIRECTORY(${REG23_GUI_SOURCE_DIR})

### TRANSLATION ###

SET(REG23_FILES_TO_TRANSLATE "") 
SET(REG23_TRANS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/translation)
FILE(MAKE_DIRECTORY ${REG23_TRANS_SRC_DIR})

SET(REG23_FILES_TO_TRANSLATE 
  ${REG23_FILES_TO_TRANSLATE} 
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx) # TRANSLATION

# CORE library
SET(REG23_FILES_TO_TRANSLATE 
  ${REG23_FILES_TO_TRANSLATE} 
  ${REG23_CORE_SOURCE_DIR}/*.h) # TRANSLATION
SET(REG23_FILES_TO_TRANSLATE 
  ${REG23_FILES_TO_TRANSLATE} 
  ${REG23_CORE_SOURCE_DIR}/*.cxx) # TRANSLATION  

# GUI library
SET(REG23_FILES_TO_TRANSLATE 
  ${REG23_FILES_TO_TRANSLATE} 
  ${REG23_GUI_SOURCE_DIR}/*.h) # TRANSLATION
SET(REG23_FILES_TO_TRANSLATE 
  ${REG23_FILES_TO_TRANSLATE} 
  ${REG23_GUI_SOURCE_DIR}/*.cxx) # TRANSLATION
SET(REG23_FILES_TO_TRANSLATE 
  ${REG23_FILES_TO_TRANSLATE} 
  ${REG23_GUI_SOURCE_DIR}/*.ui) # TRANSLATION    

# Set REG23_TRANSL_UPDATE option to OFF WHEN NOT NEEDED (POTENTIAL DANGER of deleting 
# *.ts files in translation-subfolder when 'make clean' is involved!

# the supported language files:
# de ... German
# en ... English
# sk ... Slovakian
SET(REG23_TSFILES 
  ${REG23_TRANS_SRC_DIR}/reg23_de.ts
  ${REG23_TRANS_SRC_DIR}/reg23_en.ts
  ${REG23_TRANS_SRC_DIR}/reg23_sk.ts
  )

OPTION(REG23_TRANSL_UPDATE "REG23 translation update (turn OFF as soon as possible - danger when involving 'make clean'!)." OFF)
IF(REG23_TRANSL_UPDATE)
  QT4_CREATE_TRANSLATION(
    REG23_TS_FILES 
    ${REG23_FILES_TO_TRANSLATE} # collected above
    ${REG23_TSFILES}
  )
ENDIF(REG23_TRANSL_UPDATE)

QT4_ADD_TRANSLATION(REG23_QM_FILES ${REG23_TSFILES})

### Platform-specific builds ###

SET(WINBUILDSTR "")
IF(WIN32 AND MINGW)
  SET(WINBUILDSTR "WIN32") # --> the console window won't pop up at startup
ELSE(WIN32 AND MINGW)
  SET(WINBUILDSTR "")
ENDIF(WIN32 AND MINGW) 

# File icon on win32
if(WIN32)   
  if(MINGW)
    # Compile the short-path-converter-tool
    # (this is needed because windres.exe cannot handle long paths!!! see below)
    SET(SPEDIR ${CMAKE_CURRENT_SOURCE_DIR}/shortpathexec) 
    ADD_SUBDIRECTORY(${SPEDIR})
  
    # rc-updater tool
    ADD_EXECUTABLE(reg23-rc-updater reg23-rc-updater.cxx reg23info.h reg23info.cxx)
    TARGET_LINK_LIBRARIES(reg23-rc-updater ORAIFTools)
  
    # resource compilation for mingw   
    ADD_CUSTOM_COMMAND(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/reg23_rc.o ${CMAKE_CURRENT_BINARY_DIR}/dummy.o
      # first be sure that the RC-file is updated:
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/reg23-rc-updater.exe "${CMAKE_CURRENT_SOURCE_DIR}/reg23.rc"
      # compile RC-file info into binary (later):
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/shortpathexec/shortpathexec.exe
        --generate-empty-file "${CMAKE_CURRENT_BINARY_DIR}/reg23_rc.o" 
        windres.exe -I "${CMAKE_CURRENT_SOURCE_DIR}" -i "${CMAKE_CURRENT_SOURCE_DIR}/reg23.rc"
        -o "${CMAKE_CURRENT_BINARY_DIR}/reg23_rc.o"
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shortpathexec/shortpathexec.exe reg23-rc-updater
    )
    SET(RC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/reg23_rc.o") 
  else(MINGW)
    SET(RC_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/reg23.rc") 
  endif(MINGW)
else(WIN32)
  SET(RC_SRCS "") 
endif(WIN32)

### BINARY ###

ADD_EXECUTABLE(${BINNAME} ${WINBUILDSTR} 
  ${REG23_QM_FILES}
  ${RC_SRCS}
  reg23info.h
  reg23info.cxx
  reg23.cxx
)
SET_TARGET_PROPERTIES(${BINNAME} PROPERTIES LINKER_LANGUAGE CXX)
IF(MSVC)
  IF(NOT REG23_CMAKE_CXX_FLAGS_SET)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj" CACHE STRING "Debug options." FORCE)
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj" CACHE STRING "Debug options." FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /bigobj" CACHE STRING "Release options." FORCE)
    SET(REG23_CMAKE_CXX_FLAGS_SET "1" CACHE INTERNAL "")
  ENDIF(NOT REG23_CMAKE_CXX_FLAGS_SET)
ENDIF(MSVC)
TARGET_LINK_LIBRARIES(${BINNAME} 
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  
  REG23Core 
  REG23GUI
  
  ORAIFTools
  ORAIFModel
)

# RESULTS COLLECTOR TOOL
ADD_EXECUTABLE(resultscollector
  resultscollector.cxx
)
TARGET_LINK_LIBRARIES(resultscollector  
  REG23Core
  ORAIFTools
)

# Generate Doxygen API documentation
OPTION(REG23_BUILD_API_DOCS "Use Doxygen to create the HTML based REG23 API documentation" OFF) 
IF(REG23_BUILD_API_DOCS) 
  FIND_PACKAGE(Doxygen) 
  IF(NOT DOXYGEN_FOUND) 
    MESSAGE(SEND_ERROR "Doxygen is needed to build the REG23 API documentation. Please install it correctly") 
  ELSE(NOT DOXYGEN_FOUND) 
    # If Graphviz is installed use the 'dot' tool to generate graphs
    # NOTE: CMake is unable to find DOT on Windows.
    # NOTE: On windows the DOT_PATH must be set in the doxyfile otherwise errors occur. 
    IF(DOXYGEN_DOT_FOUND OR (DOXYGEN_DOT_EXECUTABLE AND DOXYGEN_DOT_PATH))
      SET(HAVEDOT "YES")
      SET(DOTPATH ${DOXYGEN_DOT_PATH})
    ELSE(DOXYGEN_DOT_FOUND OR (DOXYGEN_DOT_EXECUTABLE AND DOXYGEN_DOT_PATH))
      SET(HAVEDOT "NO")
    ENDIF(DOXYGEN_DOT_FOUND OR (DOXYGEN_DOT_EXECUTABLE AND DOXYGEN_DOT_PATH))
    
    # Link to external library documentations through tag-files generated with
    # doxytag -t doc.tag path_to_html_doc.
    # NOTE: The tag files are not contained in the SVN repository.
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/gdcm-2.0.14.tag)
      SET(TAGFILES "${TAGFILES} \"${CMAKE_CURRENT_SOURCE_DIR}/doc/gdcm-2.0.14.tag=http://gdcm.sourceforge.net/2.0/html\"")
    ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/gdcm-2.0.14.tag)
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/itk-3.20.tag)
      SET(TAGFILES "${TAGFILES} \"${CMAKE_CURRENT_SOURCE_DIR}/doc/itk-3.20.tag=http://www.itk.org/Doxygen320/html\"")
    ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/itk-3.20.tag)
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/vtk-5.6.tag)
      SET(TAGFILES "${TAGFILES} \"${CMAKE_CURRENT_SOURCE_DIR}/doc/vtk-5.6.tag=http://www.vtk.org/doc/release/5.6/html\"")
    ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/vtk-5.6.tag)
    # NOTE: Starting with Qt 4.x the html doc was changed to qch files 
    # (not possible to generate tag files for Qt from qch)
    # One must download the separate html documentation for tag file generation
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/qt-4.6.3.tag)
      SET(TAGFILES "${TAGFILES} \"${CMAKE_CURRENT_SOURCE_DIR}/doc/qt-4.6.3.tag=http://doc.qt.nokia.com/4.6\"")
    ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/qt-4.6.3.tag)
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/oraifutils-1.1.10.tag)
      SET(TAGFILES "${TAGFILES} \"${CMAKE_CURRENT_SOURCE_DIR}/doc/oraifutils-1.1.10.tag=file://///192.168.141.15/RAServer/IT/open-radART/docu/api-doc/oraifutils-1.1.10\"")
    ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc/oraifutils-1.1.10.tag)
    
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in 
                   ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile  @ONLY) 
    ADD_CUSTOM_TARGET(doc ALL
                   ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
                   COMMENT "Generating REG23 API documentation with doxygen"
                   VERBATIM) 
  ENDIF(NOT DOXYGEN_FOUND)
ENDIF(REG23_BUILD_API_DOCS) 
