# - Find Octave
# Find the Octave includes, libraries, and executables
#
#  OCTAVE_INCLUDE_DIR
#  OCTAVE_LIBRARIES
#  OCTAVE_MKOCTFILE
#  OCTAVE_FOUND

IF (OCTAVE_INCLUDE_DIR)
  # Already in cache, be silent
  SET (Octave_FIND_QUIETLY TRUE)
ENDIF (OCTAVE_INCLUDE_DIR)

# Find some hints for directories using octave-config
FIND_PROGRAM (OCTAVE_CONFIG
  octave-config
  )
IF (OCTAVE_CONFIG)
  EXECUTE_PROCESS (
    COMMAND ${OCTAVE_CONFIG} -p OCTLIBDIR
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE ERROR
    )
  IF (${RESULT} EQUAL 0)
    STRING (STRIP ${OUTPUT} OCTLIBDIR)
  ENDIF (${RESULT} EQUAL 0)
  EXECUTE_PROCESS (
    COMMAND ${OCTAVE_CONFIG} -p OCTINCLUDEDIR
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE ERROR
    )
  IF (${RESULT} EQUAL 0)
    STRING (STRIP ${OUTPUT} OCTINCLUDEDIR)
  ENDIF (${RESULT} EQUAL 0)
ENDIF (OCTAVE_CONFIG)
  
# Find include dir
FIND_PATH (OCTAVE_INCLUDE_DIR octave/oct.h
  PATHS
  ${OCTINCLUDEDIR}
  ${OCTAVE_DIR}
  /usr/local/include
  /usr/include
  )
#MESSAGE (STATUS "OCTAVE_INCLUDE_DIR = ${OCTAVE_INCLUDE_DIR}")

# Find libraries
FIND_LIBRARY (OCTAVE_LIBRARIES octave
  PATHS 
  ${OCTLIBDIR}
  /usr/lib
  /usr/local/lib
  ${OCTAVE_DIR}
  )
#MESSAGE (STATUS "OCTAVE_LIBRARIES = ${OCTAVE_LIBRARIES}")

# Find mkoctfile
FIND_PROGRAM (OCTAVE_MKOCTFILE mkoctfile
  PATHS
  /usr/local/bin
  ${OCTAVE_DIR}
  )
#MESSAGE (STATUS "OCTAVE_MKOCTFILE = ${OCTAVE_MKOCTFILE}")

# Set OCTAVE_FOUND
IF (OCTAVE_INCLUDE_DIR AND OCTAVE_LIBRARIES AND OCTAVE_MKOCTFILE)
  SET (OCTAVE_FOUND TRUE)
ELSE (OCTAVE_INCLUDE_DIR AND OCTAVE_LIBRARIES AND OCTAVE_MKOCTFILE)
  SET (OCTAVE_FOUND FALSE)
ENDIF (OCTAVE_INCLUDE_DIR AND OCTAVE_LIBRARIES AND OCTAVE_MKOCTFILE)