######################################################
##  RULES FOR FINDING FFTW
##   FFTW_INCLUDE_DIR - where to find fftw3.h, etc.
##   FFTW_LIBRARIES   - List of libraries when using fftw
##   FFTW_FOUND       - True if fftw found.
######################################################
IF (FFTW_INCLUDE_DIR)
  # Already in cache, be silent
  SET (FFTW_FIND_QUIETLY TRUE)
ENDIF (FFTW_INCLUDE_DIR)

IF (NOT FFTW_DIR)
  # I think this only applies to windows
  FIND_PATH(FFTW_DIR fftw3.h
    PATH $ENV{FFTWDIR} $ENV{FFTW_DIR} "${FFTWDIR}"
    DOC "Root directory of fftw.")
ENDIF (NOT FFTW_DIR)

# Convert from old name (without underscore) to new name (with underscore)
IF (FFTWDIR AND FFTW_DIR)
  UNSET (FFTWDIR CACHE)
ENDIF (FFTWDIR AND FFTW_DIR)

FIND_PATH (FFTW_INCLUDE_DIR fftw3.h
  ${FFTW_DIR}
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(FFTW_LIBRARY_FLOAT
  NAMES fftw3f libfftw3f-3
  PATHS /usr/lib /usr/local/lib ${FFTW_DIR}
)
FIND_LIBRARY(FFTW_LIBRARY_DOUBLE
  NAMES fftw3 libfftw3-3
  PATHS /usr/lib /usr/local/lib ${FFTW_DIR}
)
FIND_LIBRARY(FFTW_LIBRARY_LONG_DOUBLE
  NAMES fftw3l libfftw3l-3
  PATHS /usr/lib /usr/local/lib ${FFTW_DIR}
)

# NOTE: OpenSUSE doesn't provide long double implementation
IF (FFTW_INCLUDE_DIR AND FFTW_LIBRARY_FLOAT AND FFTW_LIBRARY_DOUBLE)
  SET (FFTW_FOUND TRUE)
  SET (FFTW_LIBRARIES ${FFTW_LIBRARY_FLOAT} ${FFTW_LIBRARY_DOUBLE})
ELSE (FFTW_INCLUDE_DIR AND FFTW_LIBRARY_FLOAT AND FFTW_LIBRARY_DOUBLE)
  SET (FFTW_FOUND FALSE)
  SET (FFTW_LIBRARIES)
ENDIF (FFTW_INCLUDE_DIR AND FFTW_LIBRARY_FLOAT AND FFTW_LIBRARY_DOUBLE)

# IF (FFTW_FOUND)
#    IF (NOT FFTW_FIND_QUIETLY)
#       MESSAGE(STATUS "Found FFTW: ${FFTW_LIBRARY}")
#    ENDIF (NOT FFTW_FIND_QUIETLY)
# ELSE (FFTW_FOUND)
#    IF (FFTW_FIND_REQUIRED)
#       MESSAGE(STATUS "Looked for FFTW libraries named ${FFTW_NAMES}.")
#       MESSAGE(FATAL_ERROR "Could NOT find FFTW library")
#    ENDIF (FFTW_FIND_REQUIRED)
# ENDIF (FFTW_FOUND)

MARK_AS_ADVANCED(
  FFTW_LIBRARY
  FFTW_INCLUDE_DIR
  )


