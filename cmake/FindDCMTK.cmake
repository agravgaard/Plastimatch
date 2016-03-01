# - find DCMTK libraries
#

#  DCMTK_INCLUDE_DIR   - Directories to include to use DCMTK
#  DCMTK_LIBRARIES     - Files to link against to use DCMTK
#  DCMTK_FOUND         - If false, don't try to use DCMTK
#  DCMTK_DIR           - (optional) Source directory for DCMTK
#
#  DCMTK_VERSION_STRING - Like "3.5.4" or "3.6.0"
#
# DCMTK_DIR can be used to make it simpler to find the various include
# directories and compiled libraries if you've just compiled it in the
# source tree. Just set it to the root of the tree where you extracted
# the source.
#
# Written for VXL by Amitha Perera.
# 
# On debian, require the following packages:
#   libdcmtk1-dev
#   libpng12-dev
#   libtiff4-dev
#   libwrap0-dev

include(CheckLibraryExists)
include(FindThreads)

set (DCMTK_DIR "" CACHE PATH "Root of DCMTK install tree (optional).")

find_package (ZLIB)
if (ZLIB_FOUND)
  message (STATUS "Looking for ZLIB - found")
else ()
  message (STATUS "Looking for ZLIB - not found")
endif ()

find_package (PNG)
if (PNG_FOUND)
  message (STATUS "Looking for PNG - found")
else ()
  message (STATUS "Looking for PNG - not found")
endif ()

find_package (TIFF)
if (TIFF_FOUND)
  message (STATUS "Looking for TIFF - found")
else ()
  message (STATUS "Looking for TIFF - not found")
endif ()

find_library (SSL_LIBRARY ssl)
if (SSL_LIBRARY)
  message (STATUS "Looking for SSL - found: ${SSL_LIBRARY}")
else ()
  message (STATUS "Looking for SSL - not found")
endif ()

find_path (DCMTK_INCLUDE_DIR
  NAMES dcmtk/config/osconfig.h
  HINTS
  ${DCMTK_DIR}/include
  /usr/local/dicom/include
  )

if (UNIX)
  find_file (DCMTK_HAVE_CFUNIX_H dcmtk/config/cfunix.h
    ${DCMTK_DIR}/include
    /usr/local/dicom/include
    )
else ()
  set (DCMTK_HAVE_CFUNIX_H FALSE)
endif ()

find_library (DCMTK_dcmimgle_LIBRARY 
  NAMES dcmimgle
  HINTS
  ${DCMTK_DIR}/dcmimgle/libsrc
  ${DCMTK_DIR}/dcmimgle/libsrc/Release
  ${DCMTK_DIR}/dcmimgle/libsrc/Debug
  ${DCMTK_DIR}/dcmimgle/Release
  ${DCMTK_DIR}/dcmimgle/Debug
  ${DCMTK_DIR}/lib
  /usr/lib/dcmtk
  /usr/local/dicom/lib
)

# This is gone in 3.6
find_library (DCMTK_imagedb_LIBRARY 
  NAMES imagedb dcmimage
  HINTS
  ${DCMTK_DIR}/imagectn/libsrc/Release
  ${DCMTK_DIR}/imagectn/libsrc/
  ${DCMTK_DIR}/imagectn/libsrc/Debug
  ${DCMTK_DIR}/lib/
  /usr/lib/dcmtk
  /usr/local/dicom/lib
)

find_library (DCMTK_dcmtls_LIBRARY 
  NAMES dcmtls 
  HINTS
  ${DCMTK_DIR}/dcmnet/libsrc/Release
  ${DCMTK_DIR}/dcmnet/libsrc/Debug
  ${DCMTK_DIR}/dcmnet/libsrc
  ${DCMTK_DIR}/dcmtls/libsrc/Release
  ${DCMTK_DIR}/dcmtls/libsrc/Debug
  ${DCMTK_DIR}/dcmtls/libsrc
  ${DCMTK_DIR}/lib
  /usr/lib/dcmtk
  /usr/local/dicom/lib
)

find_library (DCMTK_dcmnet_LIBRARY 
  NAMES dcmnet 
  HINTS
  ${DCMTK_DIR}/dcmnet/libsrc/Release
  ${DCMTK_DIR}/dcmnet/libsrc/Debug
  ${DCMTK_DIR}/dcmnet/libsrc/
  ${DCMTK_DIR}/lib/
  /usr/lib/dcmtk
  /usr/local/dicom/lib
)

find_library (DCMTK_dcmdata_LIBRARY 
  NAMES dcmdata
  HINTS
  ${DCMTK_DIR}/dcmdata/libsrc
  ${DCMTK_DIR}/dcmdata/libsrc/Release
  ${DCMTK_DIR}/dcmdata/libsrc/Debug
  ${DCMTK_DIR}/dcmdata/Release
  ${DCMTK_DIR}/dcmdata/Debug
  ${DCMTK_DIR}/lib
  /usr/lib/dcmtk
  /usr/local/dicom/lib
)

# Quick hack: dcmtk 3.6.0
find_library (DCMTK_oflog_LIBRARY
  NAMES oflog
  HINTS
  ${DCMTK_DIR}/lib
  /usr/lib/dcmtk
  /usr/local/dicom/lib
)

find_library (DCMTK_ofstd_LIBRARY 
  NAMES ofstd
  HINTS
  ${DCMTK_DIR}/ofstd/libsrc
  ${DCMTK_DIR}/ofstd/libsrc/Release
  ${DCMTK_DIR}/ofstd/libsrc/Debug
  ${DCMTK_DIR}/ofstd/Release
  ${DCMTK_DIR}/ofstd/Debug
  ${DCMTK_DIR}/lib
  /usr/lib/dcmtk
  /usr/local/dicom/lib
)

if (DCMTK_INCLUDE_DIR 
    AND DCMTK_dcmnet_LIBRARY
    AND DCMTK_ofstd_LIBRARY
    AND DCMTK_dcmdata_LIBRARY
    AND DCMTK_dcmimgle_LIBRARY)

  set (DCMTK_FOUND "YES")

  set (DCMTK_LIBRARIES "")

  ## The libraries dcmtls, dcmnet, dcmimgle are not used by plastimatch.  
  ## Including them increases the number of required dependencies
  ## for debian.
  set (DCMTK_LIBRARIES 
    ${DCMTK_LIBRARIES}
#    ${DCMTK_dcmtls_LIBRARY}
#    ${DCMTK_dcmnet_LIBRARY}
#    ${DCMTK_dcmimgle_LIBRARY}
    ${DCMTK_dcmdata_LIBRARY}
    )

  if (DCMTK_oflog_LIBRARY)
    set (DCMTK_LIBRARIES
      ${DCMTK_LIBRARIES}
      ${DCMTK_oflog_LIBRARY}
      )
  endif ()

  set (DCMTK_LIBRARIES 
    ${DCMTK_LIBRARIES}
    ${DCMTK_ofstd_LIBRARY}
    )

  if (SSL_LIBRARY)
    set (DCMTK_LIBRARIES
      ${DCMTK_LIBRARIES}
      ${SSL_LIBRARY}
      )
  endif ()

  if (PNG_FOUND)
    set (DCMTK_LIBRARIES
      ${DCMTK_LIBRARIES}
      ${PNG_LIBRARIES})
  endif ()

  if (TIFF_FOUND)
    set (DCMTK_LIBRARIES
      ${DCMTK_LIBRARIES}
      ${TIFF_LIBRARIES})
  endif ()

  if (ZLIB_FOUND)
    set (DCMTK_LIBRARIES
      ${DCMTK_LIBRARIES}
      ${ZLIB_LIBRARIES})
  endif ()

  if (CMAKE_THREAD_LIBS_INIT)
    set (DCMTK_LIBRARIES
      ${DCMTK_LIBRARIES}
      ${CMAKE_THREAD_LIBS_INIT})
  endif ()

  # Dcmtk must be linked with libwrap if the original package was 
  # built with libwrap support.  
  set (NEED_LIBWRAP_CHECK TRUE)
  if (EXISTS "${DCMTK_INCLUDE_DIR}/dcmtk/config/osconfig.h")
    file (STRINGS "${DCMTK_INCLUDE_DIR}/dcmtk/config/osconfig.h"
      DCMTK_UNDEF_TCPWRAPPER
      REGEX "#undef WITH_TCPWRAPPER")
    if (NOT ${DCMTK_UNDEF_TCPWRAPPER} STREQUAL "")
      message (STATUS "DCMTK was built without libwrap")
      set (NEED_LIBWRAP_CHECK FALSE)
    else ()
      message (STATUS "DCMTK may have been built with libwrap")
    endif ()
  endif ()

  if (NEED_LIBWRAP_CHECK)
    find_library (LIBWRAP_LIBRARY NAMES wrap libwrap PATHS /lib)
    if (LIBWRAP_LIBRARY)
      set (DCMTK_LIBRARIES
	${DCMTK_LIBRARIES}
	${LIBWRAP_LIBRARY})
    endif ()
  endif ()

  if (WIN32)
    set (DCMTK_LIBRARIES ${DCMTK_LIBRARIES} netapi32 ws2_32)
  endif ()
endif ()
