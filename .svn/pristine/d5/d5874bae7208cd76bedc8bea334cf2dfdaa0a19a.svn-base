# - Find Thrust
# Find the native Thrust includes and library
#
#  THRUST_INCLUDE_DIR - where to find zlib.h, etc.
#  THRUST_FOUND       - True if zlib found.

if (NOT THRUST_DIR)
  find_path (THRUST_DIR THRUSTconfig.cmake
    $ENV{THRUST_DIR}
    DOC "The build directory, containing Thrustconfig.cmake")
endif (NOT THRUST_DIR)

if (THRUST_DIR)
  if (EXISTS (${THRUST_DIR}/Thrustconfig.cmake))
    include (${THRUST_DIR}/Thrustconfig.cmake)
  endif ()
endif (THRUST_DIR)

if (THRUST_INCLUDE_DIR)
  # Already in cache, be silent
  set (Thrust_FIND_QUIETLY TRUE)
endif (THRUST_INCLUDE_DIR)

find_path (THRUST_INCLUDE_DIR "thrust/uninitialized_fill.h"
  PATHS "${THRUST_DIR}")

# handle the QUIETLY and REQUIRED arguments and set THRUST_FOUND to TRUE if 
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (THRUST DEFAULT_MSG THRUST_INCLUDE_DIR)
