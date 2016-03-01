##-----------------------------------------------------------------------------
##  Welcome to the Plastimatch CMakeLists.txt file
##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_mondoshot)

##-----------------------------------------------------------------------------
##  Include directories
##-----------------------------------------------------------------------------
include_directories (BEFORE ${src_plastimatch_SOURCE_DIR})
include_directories (BEFORE ${src_plastimatch_SOURCE_DIR}/base)
include_directories (BEFORE ${src_plastimatch_SOURCE_DIR}/sys)
include_directories (BEFORE ${CMAKE_BINARY_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})
include_directories (AFTER ${MSINTTYPES_INCLUDE_DIR})
include_directories (BEFORE ${wxWidgets_INCLUDE_DIRS})
include_directories (AFTER ${DCMTK_INCLUDE_DIR})
include_directories (AFTER ${SQLITE_INCLUDE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES (EXECUTABLES)
##-----------------------------------------------------------------------------
set (MONDOSHOT_SRC
#  dcmtk_uid.cxx dcmtk_uid.h
  mondoshot_dicom.cpp mondoshot_dicom.h
  mondoshot_main.cpp mondoshot_main.h
  )

if (wxWidgets_CXX_FLAGS)
  string (REGEX REPLACE "-D" "" wxWidgets_DEFINITIONS 
    "${wxWidgets_DEFINITIONS}")
  set_source_files_properties (mondoshot_main.cpp
    PROPERTIES COMPILE_FLAGS 
    ${wxWidgets_CXX_FLAGS}
    )
endif ()

##-----------------------------------------------------------------------------
##  Executable targets
##-----------------------------------------------------------------------------
# mondoshot requires WIN32, wx, dcmtk, sqlite3
if (WIN32 AND NOT CYGWIN AND wxWidgets_FOUND AND DCMTK_FOUND
    AND NOT DCMTK_VERSION_STRING VERSION_LESS 3.6.0)
  add_executable (mondoshot WIN32 ${MONDOSHOT_SRC})
  target_link_libraries (mondoshot 
    ${wxWidgets_LIBRARIES} ${DCMTK_LIBRARIES} ${SQLITE_LIBRARIES}
    ${PLASTIMATCH_LIBS})
  if (NOT BUILD_AGAINST_SLICER3)
    install (TARGETS mondoshot DESTINATION bin)
  endif ()
endif ()
