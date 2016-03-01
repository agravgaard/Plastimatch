# - Wrapper around FindDCMTK

## If it is a modern version of DCMTK, such as found in Slicer build, 
## it will have a working version of DCMTKConfig.cmake.
## Otherwise we use the old hacked version of FindDCMTK.cmake.
if (DCMTK_DIR)
  if (EXISTS "${DCMTK_DIR}/DCMTKConfig.cmake")
    find_package (DCMTK NO_MODULE)
  endif ()
endif ()
if (NOT DCMTK_FOUND)
  find_package (DCMTK)
endif ()


## Get the version string (?)
if (DCMTK_FOUND)
  # Basic version information
  if (DCMTK_VERSION_MAJOR)
    set (DCMTK_VERSION_STRING "${DCMTK_VERSION_MAJOR}.${DCMTK_VERSION_MINOR}.${DCMTK_VERSION_PATCH}")
  endif ()

  if (NOT DCMTK_VERSION_STRING)
    if (EXISTS "${DCMTK_INCLUDE_DIR}/dcmtk/dcmdata/dcuid.h")
      file (STRINGS "${DCMTK_INCLUDE_DIR}/dcmtk/dcmdata/dcuid.h" 
	DCMTK_VERSION_STRING
	REGEX "^#define OFFIS_DCMTK_VERSION_STRING *\"([^\"]*)\"")
    endif ()
  endif ()
  if (NOT DCMTK_VERSION_STRING)
    if (EXISTS "${DCMTK_INCLUDE_DIR}/dcmtk/config/osconfig.h")
      file (STRINGS "${DCMTK_INCLUDE_DIR}/dcmtk/config/osconfig.h"
	DCMTK_VERSION_STRING
	REGEX "^#define PACKAGE_VERSION *\"([^\"]*)\"")
    endif ()
  endif ()
  if (NOT DCMTK_VERSION_STRING)
    if (EXISTS "${DCMTK_INCLUDE_DIR}/dcmtk/config/cfunix.h")
      file (STRINGS "${DCMTK_INCLUDE_DIR}/dcmtk/config/cfunix.h"
        DCMTK_VERSION_STRING
        REGEX "^#define PACKAGE_VERSION *\"([^\"]*)\"")
    endif ()
  endif ()

  if (DCMTK_VERSION_STRING)
    # GCS: The below doesn't seem to work on Mac CMake 2.6.4.
    #  SET (DCMTK_VERSION_STRING "${CMAKE_MATCH_1}")
    string (REGEX REPLACE "[^\"]*\"([^\"]*)\".*" "\\1"
      DCMTK_VERSION_STRING "${DCMTK_VERSION_STRING}")
  endif ()
endif ()


if (DCMTK_FOUND)
  message (STATUS "DCMTK version ${DCMTK_VERSION_STRING}")
else ()
  message (STATUS "DCMTK not found")
endif ()
