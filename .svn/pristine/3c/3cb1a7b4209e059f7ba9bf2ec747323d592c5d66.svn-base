# - Find Fann
# Find the native Fann includes and library
#
#  FANN_INCLUDE_DIR - where to find zlib.h, etc.
#  FANN_LIBRARIES   - List of libraries when using zlib.
#  FANN_FOUND       - True if zlib found.


IF (FANN_INCLUDE_DIR)
  # Already in cache, be silent
  SET (fann_FIND_QUIETLY TRUE)
ENDIF (FANN_INCLUDE_DIR)

FIND_PATH(FANN_INCLUDE_DIR fann.h)

SET (FANN_NAMES fann)
FIND_LIBRARY (FANN_LIBRARY NAMES ${FANN_NAMES})

# handle the QUIETLY and REQUIRED arguments and set FANN_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (FANN DEFAULT_MSG 
  FANN_LIBRARY 
  FANN_INCLUDE_DIR)

IF(FANN_FOUND)
  SET (FANN_LIBRARIES ${FANN_LIBRARY})
ELSE (FANN_FOUND)
  SET (FANN_LIBRARIES)
ENDIF (FANN_FOUND)

MARK_AS_ADVANCED (FANN_LIBRARY FANN_INCLUDE_DIR)
