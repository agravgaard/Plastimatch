# MIL = Matrox Imaging Library
# Sets the following variables
#
#   MIL_FOUND
#   MIL_INCLUDE_DIR
#   MIL_LIBRARIES
#
# Only works on Windows

if (MIL_INCLUDE_DIR)
  # Already in cache, be silent
  set (MIL_FIND_QUIETLY TRUE)
endif ()

FIND_PATH (MIL_SDK_DIR "include/mil.h" 
  "C:/Program Files/Matrox Imaging/mil"
  DOC "Path to MIL SDK")

FIND_PATH (MIL_INCLUDE_DIR "mil.h" 
  "${MIL_SDK_DIR}/include"
  "C:/Program Files/Matrox Imaging/mil/include"
  DOC "Path to MIL include files")
FIND_PATH (MIL_LIBRARY_PATH "BFD.lib"
  "${MIL_SDK_DIR}/library/winnt/msc/dll"
  "C:/Program Files/Matrox Imaging/mil/library/winnt/msc/dll"
  DOC "Path to MIL libraries")

FIND_LIBRARY(MIL_MIL_LIB mil ${MIL_LIBRARY_PATH})
FIND_LIBRARY(MIL_MILVGA_LIB milvga ${MIL_LIBRARY_PATH})
FIND_LIBRARY(MIL_MILMET2D_LIB milmet2d ${MIL_LIBRARY_PATH})

if (MIL_MIL_LIB AND MIL_MILVGA_LIB AND MIL_MILMET2D_LIB)
  set (MIL_LIBRARIES 
    ${MIL_MIL_LIB} ${MIL_MILVGA_LIB} ${MIL_MILMET2D_LIB})
else ()
  set (MIL_LIBRARIES)
endif ()

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (MIL DEFAULT_MSG 
  MIL_LIBRARIES MIL_INCLUDE_DIR)

MARK_AS_ADVANCED (
  MIL_INCLUDE_DIR
  MIL_LIBRARY_PATH
  MIL_MIL_LIB
  MIL_MILVGA_LIB
  MIL_MILMET2D_LIB)
