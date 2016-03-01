# - Find RTK
# Find the RTK includes and library
#
#  RTK_SOURCE_DIR  - where to find RTK source directory

# JAS 2010.11.24
# I've grown tired for getting my build log filled with
# errors from RTK when I check for Windows compile sanity...
# Give an option to turn this off without excluding ITK.
SET (PLM_BUILD_RTK ON CACHE BOOL "Build RTK")

IF (NOT RTK_SOURCE_DIR)
  FIND_PATH (RTK_SOURCE_DIR RTKconfig.cmake.in
    "${CMAKE_SOURCE_DIR}/libs/RTK.git"
    "${CMAKE_SOURCE_DIR}/libs/RTK"
    "${CMAKE_SOURCE_DIR}/libs/RTK.svn"
    $ENV{RTK_SOURCE_DIR}
    DOC "directory containing RTK source files")
ENDIF (NOT RTK_SOURCE_DIR)

#IF (NOT RTK_SOURCE_DIR)
#  MESSAGE (ERROR "Sorry, I couldn't find the RTK directory")
#ENDIF (NOT RTK_SOURCE_DIR)

# JAS 2010.11.23
# Make life easier for people who only
# want to build plastimatch components
# that don't depend on ITK.
FIND_PACKAGE (ITK)


IF (PLM_BUILD_RTK)
    IF (NOT ITK_FOUND)
        MESSAGE (STATUS "NOT building RTK, could not find ITK")
    ELSE(NOT ITK_FOUND)
        MESSAGE (STATUS "Building RTK")
	##  GCS (Dec 2, 2010) - Don't do this.  RTK/github is unstable.
        ##ADD_SUBDIRECTORY (${RTK_SOURCE_DIR})
    ENDIF (NOT ITK_FOUND)
ELSE (PLM_BUILD_RTK)
    MESSAGE (STATUS "NOT Building RTK")
ENDIF (PLM_BUILD_RTK)



IF (NOT RTK_DIR)
  FIND_PATH (RTK_DIR RTKconfig.cmake
    $ENV{RTK_DIR}
    DOC "directory containing RTK build files")
ENDIF (NOT RTK_DIR)

IF (RTK_DIR AND ITK_FOUND AND PLM_BUILD_RTK)
  SET (RTK_FOUND 1)
  INCLUDE (${RTK_DIR}/RTKconfig.cmake)
ENDIF (RTK_DIR AND ITK_FOUND AND PLM_BUILD_RTK)
