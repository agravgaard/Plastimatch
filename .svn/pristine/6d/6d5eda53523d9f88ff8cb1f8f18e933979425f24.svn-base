# JAS 08.19.2010
# I have tested this working under Linux using gcc-4.4
# The OSX/Darwin case is untested, but should work in theory.

# LINUX: We check for SSE extensions using a RegEx on /proc/cpuinfo
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")

  # Store /proc/cpuinfo output into CPUINFO
  EXEC_PROGRAM (cat ARGS "/proc/cpuinfo" OUTPUT_VARIABLE CPUINFO)

  # Check for SSE2
  STRING (REGEX REPLACE "^.*(sse2).*$" "\\1" SSE_THERE ${CPUINFO})
  STRING (COMPARE EQUAL "sse2" "${SSE_THERE}" SSE2_TRUE)
  IF (SSE2_TRUE)
    SET (SSE2_FOUND true CACHE BOOL "SSE2 Available?")
    SET (SSE2_FLAGS "-msse2 -mfpmath=sse")
  ELSE (SSE2_TRUE)
    SET (SSE2_FOUND false CACHE BOOL "SSE2 Available?")
  ENDIF (SSE2_TRUE)
    
# OSX/DARWIN: We check for SSE extensions using a RegEx on sysctl output
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  EXEC_PROGRAM ("/usr/sbin/sysctl -n machdep.cpu.features" 
    OUTPUT_VARIABLE CPUINFO)
    
  # Check for SSE2
  STRING (REGEX REPLACE "^.*(SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
  STRING (COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
  IF (SSE2_TRUE)
    SET (SSE2_FOUND true CACHE BOOL "SSE2 Available?")
    SET (SSE2_FLAGS "-msse2 -mfpmath=sse")  # Darwin uses gcc, right?
  ELSE (SSE2_TRUE)
    SET (SSE2_FOUND false CACHE BOOL "SSE2 Available?")
  ENDIF (SSE2_TRUE)

# WINDOWS: Currently, no SSE detection method.  Disabled.
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set (SSE2_FOUND false CACHE BOOL "SSE2 Available?")

# Something else... BSD or BeOS, perhaps?  Disabled.
ELSE (CMAKE_SYSTEM_NAME MATCHES "Linux")
  set (SSE2_FOUND false CACHE BOOL "SSE2 Available?")

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")


# Report SSE2 compiler flags or failure.
IF (SSE2_FOUND)
  MESSAGE (STATUS "SSE2_FLAGS \"${SSE2_FLAGS}\"")
ELSE (SSE2_FOUND)
  MESSAGE (STATUS "CPU does not support SSE2.")
ENDIF (SSE2_FOUND)
    

# Put this in the advanced toggles
mark_as_advanced (SSE2_FOUND)
