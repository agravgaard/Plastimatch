# - Find NLopt
# Find the native NLopt includes and library
#
#  NLOPT_INCLUDE_DIR - where to find zlib.h, etc.
#  NLOPT_LIBRARIES   - List of libraries when using zlib.
#  NLOPT_FOUND       - True if zlib found.

set (NLOPT_DIR "" CACHE PATH "Root of NLopt install tree (optional).")

if (NLOPT_INCLUDE_DIR)
  # Already in cache, be silent
  set (nlopt_FIND_QUIETLY TRUE)
endif (NLOPT_INCLUDE_DIR)

find_path (NLOPT_INCLUDE_DIR nlopt.h
  ${NLOPT_DIR}/include)

set (NLOPT_NAMES nlopt nlopt_cxx)
find_library (NLOPT_LIBRARY NAMES ${NLOPT_NAMES}
  PATHS
  ${NLOPT_DIR}/lib)

# handle the QUIETLY and REQUIRED arguments and set NLOPT_FOUND to TRUE if 
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (NLOPT DEFAULT_MSG 
  NLOPT_LIBRARY 
  NLOPT_INCLUDE_DIR)

if (NLOPT_FOUND)
  set (NLOPT_LIBRARIES ${NLOPT_LIBRARY})
else ()
  set (NLOPT_LIBRARIES)
endif ()

mark_as_advanced (NLOPT_LIBRARY NLOPT_INCLUDE_DIR)
