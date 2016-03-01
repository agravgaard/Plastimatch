PROJECT(fatm)

SET (PLM_BUILD_MEXFATM OFF CACHE BOOL "Enable mex fatm")

######################################################
##  RULES FOR BUILDING FATM
######################################################
# Lists of sources for executables
SET(LIBFATM_SRC
  clip_pat.cpp clip_pat.h
  config.h
  integral_img.cpp integral_img.h
  image.cpp image.h
  fatm.cpp fatm.h
  s_fancc.cpp s_fancc.h
  s_fncc.cpp s_fncc.h
  s_ncc.cpp s_ncc.h
  s_rssd.cpp s_rssd.h
  s_utils.cpp s_utils.h
  scorewin.h
  timer.cpp timer.h
  )

IF (FFTW_FOUND)
  SET(LIBFATM_SRC
    ${LIBFATM_SRC}
    s_ncc_fft.cpp s_ncc_fft.h
    )
ENDIF (FFTW_FOUND)

SET(FATM_BENCH_SRC
  fatm_bench.cpp
  )
SET(MEXFATM_SRC
  mexmatch.cpp
  mexmatch.def
  mexutils.cpp mexutils.h
  )

## CHECK_FUNCTION_EXISTS(getopt_long HAVE_GETOPT_LONG)
## IF(NOT HAVE_GETOPT_LONG)
##     SET(MHA_TO_RTOG_DOSE_SRC ${MHA_TO_RTOG_DOSE_SRC} getopt_long.c getopt.h)
## ENDIF(NOT HAVE_GETOPT_LONG)

## This makes build/config.h, which is like autoconf's config.h
## CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

IF (FFTW_FOUND)
  INCLUDE_DIRECTORIES(AFTER ${FFTW_INCLUDE_DIR})
ENDIF (FFTW_FOUND)

SET (FATM_LIBS fatm devillard)
IF (FFTW_FOUND)
  SET (FATM_LIBS ${FATM_LIBS} ${FFTW_LIBRARIES})
ENDIF (FFTW_FOUND)

## Matlab interface only works on Windows (so far)
IF(WIN32 AND NOT UNIX)
  IF (PLM_BUILD_MEXFATM)
    INCLUDE_DIRECTORIES(AFTER "C:/Program Files/MATLAB/R2006a/extern/include")
    LINK_DIRECTORIES("C:/Program Files/MATLAB/R2006a/extern/lib/win32/microsoft" ${FFTWDIR})
    ADD_LIBRARY(mexfatm SHARED ${MEXFATM_SRC})
    SET (MEXFATM_LIBS ${FATM_LIBS} libmx libmex)
    TARGET_LINK_LIBRARIES(mexfatm ${MEXFATM_LIBS})
    INSTALL(TARGETS mexfatm DESTINATION lib)
  ENDIF (PLM_BUILD_MEXFATM)
ENDIF(WIN32 AND NOT UNIX)

##-----------------------------------------------------------------------------
##  Include directories
##-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES (BEFORE ${CMAKE_BINARY_DIR})

# Build
ADD_EXECUTABLE(fatm_bench ${FATM_BENCH_SRC})
ADD_LIBRARY(fatm STATIC ${LIBFATM_SRC})

# Link
TARGET_LINK_LIBRARIES(fatm_bench ${FATM_LIBS})

# Install
##
## JAS 2011.01.24
## I have disabled this INSTALL since it would be inappropriate
## for our CPack packages to install libfatm.a
##

# INSTALL(TARGETS fatm_bench DESTINATION bin)
#IF (PLM_OLD_INSTALL_TARGETS)
#  INSTALL(TARGETS fatm DESTINATION lib)
#ENDIF (PLM_OLD_INSTALL_TARGETS)
#INSTALL(FILES fatm.h DESTINATION include)
