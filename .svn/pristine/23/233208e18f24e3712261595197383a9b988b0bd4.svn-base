##-----------------------------------------------------------------------------
##  Search for fortran compiler.
##  Set the following variables:
##    CMAKE_Fortran_COMPILER_WORKS
##    Fortran_COMPILER_NAME
##-----------------------------------------------------------------------------

## The OPTIONAL keyword is broken for most versions of CMake 2.6
## This is a workaround.
##    http://public.kitware.com/Bug/view.php?id=9220
workaround_9220 (Fortran Fortran_WORKS)
IF (Fortran_WORKS)
  ENABLE_LANGUAGE (Fortran OPTIONAL)
ENDIF (Fortran_WORKS)

IF (CMAKE_Fortran_COMPILER_WORKS)
  SET (FORTRAN_COMPILER_FOUND TRUE)
  GET_FILENAME_COMPONENT (Fortran_COMPILER_NAME ${CMAKE_Fortran_COMPILER} NAME)
  MESSAGE (STATUS "Looking for fortran compiler - found.")
ELSE (CMAKE_Fortran_COMPILER_WORKS)
  SET (FORTRAN_COMPILER_FOUND FALSE)
  MESSAGE (STATUS "Looking for fortran compiler - not found.")
ENDIF (CMAKE_Fortran_COMPILER_WORKS)
