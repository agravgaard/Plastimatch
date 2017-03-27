##-----------------------------------------------------------------------------
##  Welcome to the Plastimatch CMakeLists.txt file
##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (plastimatch)

## The version here should be equal to the "next release"
set (PLM_VERSION_MAJOR "1")
set (PLM_VERSION_MINOR "6")
set (PLM_VERSION_PATCH "5")
#set (PLM_RELEASE_VERSION FALSE)
set (PLM_RELEASE_VERSION TRUE)

##-----------------------------------------------------------------------------
##  Set up CMake defaults
##-----------------------------------------------------------------------------
cmake_minimum_required (VERSION 2.8.12)

# CMP0003: Libraries linked via full path no longer produce linker search
#  paths.
if (POLICY CMP0003)
  cmake_policy (SET CMP0003 NEW)
endif ()
# CMP0012: if() recognizes numbers and boolean constants.
# GCS 2012-03-04: This is added to suppress ITK warning
if (POLICY CMP0012)
  cmake_policy (SET CMP0012 NEW)
endif ()
# Note: it is ok to use CMake FindZLIB for 2.8.4.  Therefore setting 
# policy CMP0017 to NEW is safe.  But we do want the OLD policy for 
# older CMake versions.
cmake_policy (SET CMP0017 NEW)

# Default to release
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release
      RelWithDebInfo MinSizeRel." FORCE)
endif ()

##-----------------------------------------------------------------------------
##  Define plastimatch configuration variables
##-----------------------------------------------------------------------------
option (PLM_CONFIG_DISABLE_CUDA "Set to ON to build without CUDA" OFF)
option (PLM_CONFIG_DISABLE_DCMTK "Set to ON to build without DCMTK" OFF)
option (PLM_CONFIG_DISABLE_FATM "Set to ON to build without FATM" ON)
option (PLM_CONFIG_DISABLE_ISE "Build ISE fluoro program" ON)
option (PLM_CONFIG_DISABLE_MONDOSHOT "Disable building mondoshot" ON)
option (PLM_CONFIG_DISABLE_OPENCL "Set to ON to build without OpenCL" OFF)
option (PLM_CONFIG_DISABLE_OPENMP "Set to ON to build without OpenMP" OFF)
option (PLM_CONFIG_DISABLE_PLASTIMATCH "Disable building plastimatch" OFF)
option (PLM_CONFIG_DISABLE_QT "Set to ON to build without QT" OFF)
option (PLM_CONFIG_DISABLE_REG23 "Disable building REG-2-3" ON)
option (PLM_CONFIG_DISABLE_SSE2 "Set to ON to build without SSE" OFF)
option (PLM_CONFIG_ENABLE_MATLAB "Set to ON to build Matlab plugins" OFF)

option (PLM_CONFIG_BUILD_QT_PLUGINS "Build QT4 Designer Plugins?" OFF)
option (PLM_CONFIG_DEBIAN_BUILD "Set to ON to configure build for debian" OFF)
option (PLM_CONFIG_LIBRARY_BUILD "Set to ON to build only libraries" OFF)
option (PLM_CONFIG_NOMANIFEST
    "Set to ON to build windows DLLs without manifests" OFF)

# Compile and link options
option (PLM_SUPERBUILD "Download missing external libraries" OFF)
option (PLM_PREFER_EXTERNAL_ITK "Prefer external ITK to local one" OFF)
option (BUILD_SHARED_LIBS "Build plastimatch as shared library" OFF)

# GCS 2016-08-18: Debian now ships dlib, give option to prefer local copy
option (PLM_PREFER_SYSTEM_DLIB
    "Prefer the system dlib over the included dlib" ON)

# Testing
option (PLM_BUILD_TESTING "Enable regression testing" ON)

# Installer Options
option (PLM_INSTALL_RPATH "Add full RPATH to install" OFF)
option (PLM_CONFIG_INSTALL_LIBRARIES "Include libraries in install" ON)

# Packaging
option (PLM_PACKAGE_32BIT
    "Set this when building 32-bit packages on a 64-bit machine" OFF)
option (PLM_PACKAGE_NSIS "Set to ON when packaging binaries with NSIS" OFF)
option (PLM_PACKAGE_WIX "Set to ON when packaging binaries with WIX" OFF)
option (PLM_PACKAGE_LEGACY_CMAKE_CONFIG
    "Use the old code for creating PlastimatchConfig.cmake and friends" OFF)

# Use legacy packaging if cmake version is < 3.0
if (CMAKE_VERSION VERSION_LESS "3.0")
  set (PLM_PACKAGE_LEGACY_CMAKE_CONFIG ON CACHE BOOL "Use legacy packaging routines")
endif ()

# Override some options if library build is selected
if (PLM_CONFIG_LIBRARY_BUILD)
  set (PLM_CONFIG_DISABLE_ISE ON)
  set (PLM_CONFIG_DISABLE_FATM ON)
  set (PLM_CONFIG_DISABLE_MONDOSHOT ON)
  set (PLM_CONFIG_DISABLE_REG23 ON)
endif ()
  
##-----------------------------------------------------------------------------
##  SETUP IMPORTANT LOCATIONS
##-----------------------------------------------------------------------------
set (PLM_BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}")
set (PLM_SRC_ROOT   "${CMAKE_CURRENT_SOURCE_DIR}")

# Offer the user the choice of overriding the installation directories
set (PLM_INSTALL_LIB_DIR lib CACHE PATH 
  "Installation directory for libraries")
set (PLM_INSTALL_BIN_DIR bin CACHE PATH 
  "Installation directory for executables")
set (PLM_INSTALL_INCLUDE_DIR include/plastimatch CACHE PATH
  "Installation directory for header files")
if (WIN32 AND NOT CYGWIN)
  set (DEF_INSTALL_CMAKE_DIR CMake)
else()
  set (DEF_INSTALL_CMAKE_DIR lib/cmake/plastimatch)
endif()
set (PLM_INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
  "Installation directory for CMake files")
mark_as_advanced (
  PLM_INSTALL_LIB_DIR 
  PLM_INSTALL_BIN_DIR 
  PLM_INSTALL_INCLUDE_DIR 
  PLM_INSTALL_CMAKE_DIR)

# GCS 2015-07-08.  The below workflow is described here:
#   http://www.cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file
# However, it creates install targets with absolute paths, which is 
# not allowed for NSIS targets.
# It is possible this is resolved by newer documentation, which is referred
# by the above link:
#   http://www.cmake.org/cmake/help/git-master/manual/cmake-packages.7.html#creating-packages
# Needs further investigation

# Make relative install paths absolute (needed later on)
if (PLM_PACKAGE_LEGACY_CMAKE_CONFIG)
foreach (p LIB BIN INCLUDE CMAKE)
  set (var PLM_INSTALL_${p}_DIR)
  if (NOT IS_ABSOLUTE "${${var}}")
    set (${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif ()
endforeach ()
endif ()

##-----------------------------------------------------------------------------
##  CMake include files
##-----------------------------------------------------------------------------
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include (cmake/PlmMacros.cmake)
include (CTest)
include (CheckFunctionExists)
include (CheckIncludeFiles)
include (CheckTypeSize)
include (TestBigEndian)

# http://www.cmake.org/pipermail/cmake/2008-December/025886.html
# http://www.cmake.org/Bug/view.php?id=15117
set (CMAKE_INSTALL_OPENMP_LIBRARIES ON)
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 3.1)
    include (InstallRequiredSystemLibrariesPatched)
else ()
    include (InstallRequiredSystemLibraries)
endif ()

# Superbuild
if (PLM_SUPERBUILD)
  include (ExternalProject)
endif ()

# Machine precision
include (cmake/CheckEpsilon)

# Is char signed or unsigned?
include (cmake/CheckCharSign)

##-----------------------------------------------------------------------------
##  Processor and OS characteristics
##    32-bit or 64-bit machine
##    Endian-ness
##    Machine precision
##    Processor type
##-----------------------------------------------------------------------------
if (UNIX) # OS X and Cygwin are both considered UNIX
  if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set (GNU TRUE)
  endif ()
endif()

check_type_size ("unsigned int"   CMAKE_SIZEOF_UINT)
check_type_size ("unsigned long"  CMAKE_SIZEOF_ULONG)
check_type_size ("size_t"         CMAKE_SIZEOF_SIZE_T)
if (NOT APPLE)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (MACHINE_IS_32_BIT TRUE)
    set (MACHINE_IS_64_BIT FALSE)
    message (STATUS "Machine is 32-bit")
  else ()
    set (MACHINE_IS_32_BIT FALSE)
    set (MACHINE_IS_64_BIT TRUE)
    message (STATUS "Machine is 64-bit")
  endif ()
endif ()

test_big_endian (PLM_BIG_ENDIAN)
check_epsilon (MACHINE_EPS)
check_char_sign (CHAR_SIGN)
message (STATUS "Checking host processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message (STATUS "Checking target processor: ${CMAKE_SYSTEM_PROCESSOR}")
message (STATUS "Checking epsilon: ${MACHINE_EPS}")
message (STATUS "Checking sign of char: ${CHAR_SIGN}")

##-----------------------------------------------------------------------------
##  Disable spurious warnings on MSVC (version 8 & higher)
##-----------------------------------------------------------------------------
if (WIN32 AND NOT CYGWIN AND NOT MINGW)
  add_definitions(
    -D_CRT_FAR_MAPPINGS_NO_DEPRECATE
    -D_CRT_IS_WCTYPE_NO_DEPRECATE
    -D_CRT_MANAGED_FP_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_SECURE_NO_DEPRECATE_GLOBALS
    -D_CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
    -D_CRT_TIME_FUNCTIONS_NO_DEPRECATE
    -D_CRT_VCCLRIT_NO_DEPRECATE
    -D_SCL_SECURE_NO_DEPRECATE
    # _SCL_SECURE_NO_DEPRECATE became _SCL_SECURE_NO_WARNINGS in VC9
    -D_SCL_SECURE_NO_WARNINGS
    )
endif ()

##-----------------------------------------------------------------------------
##  Compile with -fPIC is needed for amd64 and other platforms
##-----------------------------------------------------------------------------
plm_set_pic_flags ()

##-----------------------------------------------------------------------------
##  Extra paths
##-----------------------------------------------------------------------------
set (PLM_TESTING_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Testing")
set (PLM_TESTING_DATA_DIR "${CMAKE_SOURCE_DIR}/Testing/Data")
set (PLM_TESTING_DOWNLOAD_DATA_DIR "${CMAKE_BINARY_DIR}/TestData")
set (PLM_BUILD_TESTING_DIR "${CMAKE_BINARY_DIR}/Testing")

##-----------------------------------------------------------------------------
##  Search for include files
##-----------------------------------------------------------------------------
check_include_files ("stdint.h" HAVE_STDINT_H)
check_include_files ("sys/stat.h" HAVE_SYS_STAT_H)

##-----------------------------------------------------------------------------
##  A few constants used to control build & install
##-----------------------------------------------------------------------------
set (INSTALL_NEVER 0)
set (INSTALL_ALWAYS 1)
if (PLM_CONFIG_DEBIAN_BUILD)
  set (INSTALL_IF_NOT_DEBIAN 0)
else ()
  set (INSTALL_IF_NOT_DEBIAN 1)
endif ()

set (BUILD_NEVER 0)
set (BUILD_ALWAYS 1)
set (BUILD_IF_NOT_SLICER_EXT 1)

##-----------------------------------------------------------------------------
##  Set the math library
##-----------------------------------------------------------------------------
if (UNIX)
  set (MATH_LIB -lm)
else ()
  set (MATH_LIB)
endif ()

##-----------------------------------------------------------------------------
##  Libdl
##-----------------------------------------------------------------------------
# Some systems such as pcBSD don't have libdl
find_library (LIBDL_FOUND dl)

##-----------------------------------------------------------------------------
##  Search for libraries
##-----------------------------------------------------------------------------
if (PLM_CONFIG_DISABLE_DCMTK)
  set (DCMTK_FOUND FALSE)
else ()
  find_package (DCMTK_wrap)
  if (DCMTK_FOUND AND DCMTK_VERSION_STRING VERSION_LESS 3.6.0)
    set (DCMTK_FOUND FALSE)
  endif ()
endif ()
if (PLM_PREFER_SYSTEM_DLIB)
  # Use this version if you want to use the internal cmake find. 
  # The internal version has issue finding the correct BLAS library
  # for Debian, so we prefer Debian version.
  # find_package (Dlib)
  
  # However, the Debian cmake find is broken.  Its breakage can
  # be worked around by setting dlib_BINARY_DIR.
  set (dlib_BINARY_DIR 1)
  find_package (dlib QUIET)
endif ()
find_package (Etags)
#find_package (Fann)
find_package (FFTW)
find_package (Git)
find_package (Liblbfgs)
find_package (NLopt)
if (PLM_CONFIG_ENABLE_MATLAB)
  find_package (Matlab)
else ()
  set (MATLAB_FOUND false)
endif ()
find_package (Octave)
if (NOT PLM_CONFIG_DISABLE_OPENCL)
  find_package (OpenCL)
endif ()
## find_package (OpenGL)
if (NOT PLM_CONFIG_DISABLE_OPENMP)
  find_package (OpenMP)
endif ()
if (PLM_CONFIG_DEBIAN_BUILD)
  set (SQLITE_FOUND false)
else ()
  find_package (SQLite)
endif ()
if (NOT PLM_CONFIG_DISABLE_SSE2)
  find_package (SSE)           # SSE Extensions for CPU
else ()
  set (SSE2_FOUND false)
endif ()
find_package (TR1)
find_package (wxWidgets)

##-----------------------------------------------------------------------------
##  Use local libsbfgs if not found
##-----------------------------------------------------------------------------
if (NOT LIBLBFGS_FOUND)
  add_subdirectory (libs/liblbfgs-1.9)
  set (LIBLBFGS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/liblbfgs-1.9/include")
  set (LIBLBFGS_LIBRARIES lbfgs)
  link_directories (${CMAKE_CURRENT_BINARY_DIR}/libs/liblbfgs-1.9)
  set (LIBLBFGS_FOUND TRUE)
endif ()

##-----------------------------------------------------------------------------
##  Use local sqlite if not found
##-----------------------------------------------------------------------------
if (NOT SQLITE_FOUND AND NOT PLM_CONFIG_DEBIAN_BUILD)
  add_subdirectory (libs/sqlite-3.6.21)
  set (SQLITE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/sqlite-3.6.21")
  set (SQLITE_LIBRARIES sqlite3)
  set (SQLITE_FOUND TRUE)
endif ()

#-----------------------------------------------------------------------------
##  Only use local devillard
##-----------------------------------------------------------------------------
set (DEVILLARD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/devillard")
add_subdirectory (libs/devillard)

##-----------------------------------------------------------------------------
##  If using local dlib
##-----------------------------------------------------------------------------
if (dlib_FOUND)
  set (DLIB_INCLUDE_DIR ${dlib_INCLUDE_DIR})
  set (DLIB_LIBRARIES ${dlib_LIBRARIES})
  set (DLIB_FOUND TRUE)
endif ()
if (DLIB_FOUND)
  set (DLIB_HAVE_LIBRARY TRUE)
else ()
  set (DLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/dlib-19.1")
  set (DLIB_LIBRARIES "")
  set (DLIB_HAVE_LIBRARY FALSE)
endif ()

##-----------------------------------------------------------------------------
##  Only use local inih
##-----------------------------------------------------------------------------
set (INIH_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/inih-r29")
add_subdirectory (libs/inih-r29)

##-----------------------------------------------------------------------------
##  LUA 5.1.4
##-----------------------------------------------------------------------------
# We can't use debian's lua 5.1 because it doesn't contain lobject.h
#add_subdirectory (libs/lua-5.1.4)
#set (LUA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/lua-5.1.4/src")
#link_directories (${CMAKE_CURRENT_BINARY_DIR}/libs/lua-5.1.4)

# for history, etc in plastimatch tty mode
#find_package (Readline)
#find_library (TERMCAP_LIBRARY termcap)

##-----------------------------------------------------------------------------
##  Only use local specfun
##-----------------------------------------------------------------------------
set (SPECFUN_FOUND FALSE)
add_subdirectory (libs/specfun)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/libs/specfun)
set (SPECFUN_FOUND TRUE)

##-----------------------------------------------------------------------------
##  Only use local msinttypes
##-----------------------------------------------------------------------------
set (MSINTTYPES_INCLUDE_DIR 
  "${CMAKE_SOURCE_DIR}/libs" 
  )

##-----------------------------------------------------------------------------
##  Only use local nkidecompress
##-----------------------------------------------------------------------------
set (NKIDECOMPRESS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/nkidecompress")
add_subdirectory (libs/nkidecompress)

##-----------------------------------------------------------------------------
##  Special CUDA processing
##-----------------------------------------------------------------------------
if (PLM_CONFIG_DISABLE_CUDA)
  set (CUDA_FOUND false)
else ()
  find_package (CUDA_wrap)
  set (CUDA_FOUND ${CUDA_FOUND} CACHE BOOL "Did we find cuda?")
  if (CUDA_FOUND)
    cuda_include_directories (${CMAKE_CURRENT_SOURCE_DIR})
  endif ()
endif ()

##-----------------------------------------------------------------------------
##  Thrust
##-----------------------------------------------------------------------------
find_package (Thrust)

##-----------------------------------------------------------------------------
## JAS 10.28.2010
## nvcc automatically passed the flag -malign-double to gcc when compiling .cu
## files.  This can be a problem when structs are shared between 
## .c & .cu files on 32-bit machines.  So, we pass -malign-double to gcc 
## for .c files as well so that everybody is using the same alignment.  
## -malign-double is automatically passed to gcc for 64-bit architectures.
## 
## GCS 2010-10-30
## -malign-double should only be enabled for 32-bit machines.  gcc 4.1.2 
## gives an error if it is used on x86_64.
## 
## GCS 2011-07-21
## -malign-double is not compatible with ITK.  Need a fix soon...
##
## JAS 2011-07-23
## I have disabled -malign-double passing to gcc/g++ compiled objects and
## have taken to manually byte aligning structures that are passed to
## nvcc compiled objects.  (See double_align8 typedef in bspline.h)
## Hopefully this will solve the problem.
##-----------------------------------------------------------------------------
#if (CUDA_FOUND AND CMAKE_COMPILER_IS_GNUCC AND MACHINE_IS_32_BIT)
#  if (CMAKE_C_FLAGS)
#    set (CMAKE_C_FLAGS "-malign-double")
#  else ()
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -malign-double")
#  endif ()
#  if (CMAKE_CXX_FLAGS)
#    set (CMAKE_CXX_FLAGS "-malign-double")
#  else ()
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -malign-double")
#  endif ()
#endif ()

##-----------------------------------------------------------------------------
##  Doxygen
##-----------------------------------------------------------------------------
find_package (Doxygen)

##-----------------------------------------------------------------------------
##  For shared libraries, we enable dynamic loading of cuda, opencl
##  Note: we can't use BUILD_SHARED_LIBS directly, because it conflicts 
##  with the ITK header files
##-----------------------------------------------------------------------------
set (PLM_USE_GPU_PLUGINS OFF)
if (BUILD_SHARED_LIBS)
  set (PLM_USE_GPU_PLUGINS ON)
endif ()

##-----------------------------------------------------------------------------
##  Special linking instructions on unix
##  http://www.cmake.org/Wiki/CMake_RPATH_handling
##-----------------------------------------------------------------------------
if (PLM_INSTALL_RPATH)
  # use, i.e. don't skip the full RPATH for the build tree
  set (CMAKE_SKIP_BUILD_RPATH  FALSE)

  # when building, don't use the install RPATH already
  # (but later on when installing)
  set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

  # the RPATH to be used when installing
  set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

##-----------------------------------------------------------------------------
##  User feedback on which languages and libraries were found
##-----------------------------------------------------------------------------
if (wxWidgets_FOUND)
  message (STATUS "Looking for wxWidgets - found.")
else ()
  message (STATUS "Looking for wxWidgets - not found.")
endif ()
if (Matlab_FOUND)
  message (STATUS "Looking for Matlab - found.")
else ()
  message (STATUS "Looking for Matlab - not found.")
endif ()
if (OCTAVE_FOUND)
  message (STATUS "Looking for Octave - found.")
else ()
  message (STATUS "Looking for Octave - not found.")
endif ()

##-----------------------------------------------------------------------------
##  Set version string
##-----------------------------------------------------------------------------
set (PLM_BASIC_VERSION_STRING
  "${PLM_VERSION_MAJOR}.${PLM_VERSION_MINOR}.${PLM_VERSION_PATCH}")
set (PLASTIMATCH_VERSION_STRING
  "${PLM_VERSION_MAJOR}.${PLM_VERSION_MINOR}.${PLM_VERSION_PATCH}")
if (NOT PLM_RELEASE_VERSION)
  set (PLASTIMATCH_VERSION_STRING "${PLASTIMATCH_VERSION_STRING}-dev")
endif ()
if (EXISTS "${CMAKE_SOURCE_DIR}/.git" AND GIT_FOUND)
  execute_process (
    COMMAND 
    #${GIT_EXECUTABLE} rev-list --first-parent --count HEAD
    ${GIT_EXECUTABLE} describe --always
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE git_result
    OUTPUT_VARIABLE git_version
    ERROR_VARIABLE git_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if (${git_result} EQUAL 0)
    message (STATUS "Looking for version in git - found ${git_version}")
    set (PLASTIMATCH_VERSION_STRING "${PLASTIMATCH_VERSION_STRING} (${git_version})")
  else ()
    message (STATUS "Looked for version in git but failed")
  endif ()
endif ()

##-----------------------------------------------------------------------------
##  ITK
##-----------------------------------------------------------------------------
find_package (ITK)
if (PLM_SUPERBUILD)
  if (NOT ITK_FOUND OR PLM_PREFER_EXTERNAL_ITK)
    message (STATUS "ITK_DIR = ${ITK_DIR}")
    include (cmake/ExternalITK.cmake)
    message (STATUS "ITK_DIR = ${ITK_DIR}")
    find_package (ITK)
  endif ()
endif ()
if (ITK_FOUND)
  # GCS 2016-10-20
  # The GDCM USE file spews out copious extraneous warnings on Debian.
  # If we are not using GDCM, this might be avoided by setting
  # ITK_USE_SYSTEM_GDCM to FALSE.  However, this has the undesirable
  # effect of removing gdcm includes from include list and gdcm libraries
  # from library list.
  #if (DCMTK_FOUND)
  # set (ITK_USE_SYSTEM_GDCM FALSE)
  #endif ()
  include (${ITK_USE_FILE})
  set (ITK_VERSION 
    "${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR}.${ITK_VERSION_PATCH}")
  if (${ITK_VERSION} VERSION_LESS "3.16.0")
    message (FATAL_ERROR 
      "Fatal Error. ITK must be version 3.16.0 or higher")
  endif ()
  if (${ITK_VERSION_MAJOR} VERSION_EQUAL "3")
    if (NOT ITK_USE_REVIEW)
      message (FATAL_ERROR 
	"Fatal Error. ITK must be compiled with ITK_USE_REVIEW set to ON")
    endif ()
    set (ITK_LIBRARIES ${ITK_LIBRARIES} ITKIOReview)
  elseif (${ITK_VERSION_MAJOR} VERSION_EQUAL "4")
    if (${ITK_VERSION} VERSION_LESS "4.1")
      message (FATAL_ERROR 
	"Fatal Error. ITK 4 must be 4.1 or greater")
    endif ()
  else ()
    message (FATAL_ERROR 
      "Fatal Error. ITK version should be either 3.X or 4.X")
  endif ()
endif ()
message (STATUS "ITK_VERSION = ${ITK_VERSION} found")

##-----------------------------------------------------------------------------
##  GDCM
##-----------------------------------------------------------------------------
if (NOT DCMTK_FOUND AND ITK_FOUND)
  # The default case is that ITK_USE_SYSTEM_GDCM was used, 
  # the gdcm use file was already loaded from UseITK.cmake, and therefore
  # GDCM_VERSION is already known.  If this is not the case, we look 
  # in the ITK build dir for the header file which contains the version.
  if (NOT GDCM_VERSION)
    unset (GDCM_CONFIGURE_H CACHE)
    find_file (GDCM_CONFIGURE_H gdcmConfigure.h PATHS ${ITK_INCLUDE_DIRS})
    if (GDCM_CONFIGURE_H)
      file (STRINGS "${GDCM_CONFIGURE_H}"
	GDCM_VERSION
	REGEX "^#define GDCM_VERSION *\"([^\"]*)\"")
      string (REGEX REPLACE "[^\"]*\"([^\"]*)\".*" "\\1"
	GDCM_VERSION "${GDCM_VERSION}")
    endif ()
    # When GDCM config file does not configure correctly, the value of 
    # the version might be "@GDCM_VERSION@".  
    if ("${GDCM_VERSION}" STREQUAL "\@GDCM_VERSION@")
      set (GDCM_VERSION "2.0")
    endif ()
    set (GDCM_FOUND 1)
  endif ()

  if (GDCM_VERSION)
    message (STATUS "GDCM version ${GDCM_VERSION} found")
  else ()
    message (FATAL_ERROR "Found ITK, but didn't find GDCM")
  endif ()

  if (GDCM_VERSION VERSION_LESS "2.0")
    set (GDCM_VERSION_1 1)
  else ()
    set (GDCM_VERSION_2 1)
  endif ()
else ()
  # No itk?  Use dcmtk instead
endif ()

##-----------------------------------------------------------------------------
##  Figure out if we should use gdcm or dcmtk
##-----------------------------------------------------------------------------
set (PLM_DCM_USE_DCMTK 0)
set (PLM_DCM_USE_GDCM1 0)
set (PLM_DCM_USE_GDCM2 0)

if (DCMTK_FOUND)
    set (PLM_DCM_USE_DCMTK 1)
elseif (GDCM_VERSION_2)
    set (PLM_DCM_USE_GDCM2 1)
else ()
    set (PLM_DCM_USE_GDCM1 1)
endif ()

##-----------------------------------------------------------------------------
##  VTK
##-----------------------------------------------------------------------------
if (NOT PLM_CONFIG_DISABLE_REG23)
  find_package (VTK)
  if (VTK_FOUND)
    message (STATUS "VTK version ${VTK_VERSION}")
  else ()
    message (STATUS "VTK not found")
  endif ()
  # GCS 2012-05-14: Including VTK_USE_FILE causes VTK to pollute the 
  # include directory list, which causes cuda to fail to compile
  #if (VTK_FOUND)
  #  include (${VTK_USE_FILE})
  #endif ()
endif ()

##-----------------------------------------------------------------------------
##  QT
##-----------------------------------------------------------------------------
if (PLM_CONFIG_DISABLE_QT)
  set (QT4_FOUND false)
else ()
  # In VS2013 with win7, QtWebkit was not able to be compiled.(.lib file
  # is not generated.) QtWebkit is only used by Reg23.
  if (PLM_CONFIG_DISABLE_REG23)
    find_package (Qt4 4.6.3 COMPONENTS QtCore QtGui QtDesigner)
  else ()
    message (STATUS "In VS2013 w/ Qt4.8.6, QtWebkit couldn't be compiled. Check PLM_CONFIG_DISABLE_REG23 if you don't want REG23 and there is compiling error.")
    find_package (Qt4 4.6.3 COMPONENTS QtCore QtGui QtWebKit QtDesigner)
  endif ()
endif ()

if (QT4_FOUND)
   # Test Qt install to make sure it can build and run a test program
   include (cmake/CheckQt)
   check_qt (QT_TEST_COMPILE_SUCCEEDED)
   if (NOT QT_TEST_COMPILE_SUCCEEDED)
       message (STATUS "Qt failed to compile a test program")
       set (QT4_FOUND false)
   endif ()
endif ()

if (QT4_FOUND)
    # Debian qtchooser is broken, so we disable.
    # http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=730423
    execute_process (
	COMMAND 
	rcc
	RESULT_VARIABLE rcc_result
	OUTPUT_VARIABLE rcc_output
	ERROR_VARIABLE rcc_error)
    if (${rcc_error} MATCHES "could not find a Qt installation")
	message (STATUS "Sorry, your Qt build environment is broken")
	set (QT4_FOUND FALSE)
    else ()
	message (STATUS "Looking for Qt4 - found")
	include (${QT_USE_FILE})
    endif ()
else ()
    message (STATUS "Looking for Qt4 - not found")
endif ()

##-----------------------------------------------------------------------------
##  Only use local ransac
##-----------------------------------------------------------------------------
if (ITK_FOUND)
  add_subdirectory (libs/ransac)
  set (RANSAC_INCLUDE_DIRS 
    "${CMAKE_SOURCE_DIR}/libs/ransac" 
    "${CMAKE_SOURCE_DIR}/libs/ransac/Common")
endif ()

##-----------------------------------------------------------------------------
##  Only use local rapidjson
##-----------------------------------------------------------------------------
set (RAPIDJSON_DIR "${CMAKE_SOURCE_DIR}/libs/rapidjson-2015-03-22")
if (EXISTS "${RAPIDJSON_DIR}" AND IS_DIRECTORY "${RAPIDJSON_DIR}")
    set (RAPIDJSON_INCLUDE_DIR 
	"${RAPIDJSON_DIR}/include"
	)
    set (RAPIDJSON_FOUND true)
endif ()

##-----------------------------------------------------------------------------
##  libyaml
##-----------------------------------------------------------------------------
find_package (LibYAML)

##-----------------------------------------------------------------------------
##  Configure include files
##-----------------------------------------------------------------------------
set (PLM_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

##-----------------------------------------------------------------------------
##  Subdirectories
##-----------------------------------------------------------------------------
add_subdirectory (src)

##-----------------------------------------------------------------------------
##  Additional install files
##-----------------------------------------------------------------------------
if (ITK_FOUND)
    file (GLOB DLLFILES "${ITK_DIR}/bin/release/*.dll")
    if (DLLFILES)
      install (FILES ${DLLFILES} DESTINATION bin)
    endif ()
endif ()

if (CUDA_FOUND)
  if (UNIX)
    set (CUDART_LINUX_VERSION "libcudart.so.3.0.14")
    set (CUDART_LINUX32 
      "${CUDA_TOOLKIT_ROOT_DIR}/lib/${CUDART_LINUX_VERSION}")
    set (CUDART_LINUX64 
      "${CUDA_TOOLKIT_ROOT_DIR}/lib64/${CUDART_LINUX_VERSION}")

    if (MACHINE_IS_32_BIT AND EXISTS "${CUDART_LINUX32}")
      set (CUDART_FILE_SRC "${CUDART_LINUX32}")
    endif ()

    if (MACHINE_IS_64_BIT AND EXISTS "${CUDART_LINUX64}")
      set (CUDART_FILE_SRC "${CUDART_LINUX64}")
    endif ()

    # Override for packagers building 32-bit packages on 64-bit machine
    if (PLM_PACKAGE_32BIT AND EXISTS "${CUDART_LINUX32}")
      set (CUDART_FILE_SRC "${CUDART_LINUX32}")
    endif ()

    # Hard code to cuda 3.0 (runtime 3.0.14).  Note, we copy it first, because
    # otherwise CMake 2.6 will install a broken symbolic link
    set (CUDART_FILE "${CMAKE_BINARY_DIR}/${CUDART_LINUX_VERSION}")
    if (EXISTS "${CUDART_FILE_SRC}")
      execute_process (COMMAND
        ${CMAKE_COMMAND} "-E" "copy" 
	    "${CUDART_FILE_SRC}" "${CUDART_FILE}"
      )
    endif ()
  else ()
#    set (CUDART_WIN32 "${CUDA_TOOLKIT_ROOT_DIR}/bin/cudart32_30_14.dll")
#    set (CUDART_WIN64 "${CUDA_TOOLKIT_ROOT_DIR}/bin/cudart64_30_14.dll")
#05/27/2016 YKP (for window users only): will copy dll files to bin for packaging.
    set (CUDART_WIN32 "${CUDA_TOOLKIT_ROOT_DIR}/bin/cudart32_65.dll")
    set (CUDART_WIN64 "${CUDA_TOOLKIT_ROOT_DIR}/bin/cudart64_65.dll")
    set (CUFFT_WIN32 "${CUDA_TOOLKIT_ROOT_DIR}/bin/cufft32_65.dll")
    set (CUFFT_WIN64 "${CUDA_TOOLKIT_ROOT_DIR}/bin/cufft64_65.dll")

    if (MACHINE_IS_32_BIT AND EXISTS "${CUDART_WIN32}")
      set (CUDART_FILE "${CUDART_WIN32}")
    endif ()
    if (MACHINE_IS_32_BIT AND EXISTS "${CUFFT_WIN32}")
      set (CUFFT_FILE "${CUFFT_WIN32}")
    endif ()
    if (MACHINE_IS_64_BIT AND EXISTS "${CUDART_WIN64}")	
      set (CUDART_FILE "${CUDART_WIN64}")	  
    endif ()
    if (MACHINE_IS_64_BIT AND EXISTS "${CUFFT_WIN64}")		
      set (CUFFT_FILE "${CUFFT_WIN64}")	  
    endif ()

    # Override for packagers building 32-bit packages on 64-bit machine
    if (PLM_PACKAGE_32BIT AND EXISTS "${CUDART_WIN32}")
      set (CUDART_FILE_SRC "${CUDART_WIN32}")
    endif ()
  endif ()

  if (EXISTS "${CUDART_FILE}")
    if (UNIX)
      install (FILES "${CUDART_FILE}" DESTINATION lib)
    else ()
      install (FILES "${CUDART_FILE}" DESTINATION bin)
    endif ()
  endif ()

  if (EXISTS "${CUFFT_FILE}")
    if (UNIX)
      install (FILES "${CUFFT_FILE}" DESTINATION lib)
    else ()
      install (FILES "${CUFFT_FILE}" DESTINATION bin)
    endif ()
  endif ()
endif ()

#Add QT dlls to Install
if (QT4_FOUND)
  if (UNIX)
    # YKP: should be implemented soon
  else ()	
    set (QT4_CORE_DLL_WIN "${QT_LIBRARY_DIR}/QtCore4.dll")
    set (QT4_GUI_DLL_WIN "${QT_LIBRARY_DIR}/QtGui4.dll")
    if (EXISTS "${QT4_CORE_DLL_WIN}")
		install (FILES "${QT4_CORE_DLL_WIN}" DESTINATION bin)
    endif ()
	if (EXISTS "${QT4_GUI_DLL_WIN}")
		install (FILES "${QT4_GUI_DLL_WIN}" DESTINATION bin)
    endif ()
  endif ()
endif ()

#Add FFT dlls to Install
if (FFTW_FOUND)
  if (EXISTS "${FFTW_DIR}/libfftw3-3.dll")
    install (FILES "${FFTW_DIR}/libfftw3-3.dll" DESTINATION bin)	
  endif ()
    #YKP 05/27/2016: no need of libfftw3f-3.dll and libfftw3l-3.dll?
endif ()

# Add sample directory/files to Install Only for windows users
if (WIN32 OR WIN64)
  set (PLM_WINDOWS_INSTALL_DIR "${CMAKE_SOURCE_DIR}/extra/windows-install")
  install (DIRECTORY "${PLM_WINDOWS_INSTALL_DIR}/sample" DESTINATION bin)  
  install (FILES "${PLM_WINDOWS_INSTALL_DIR}/launch_cmd_prompt.bat" DESTINATION bin)
endif ()

# JAS 2011.01.24
# I have moved the documentation DESTINATION from
# /usr/doc to /usr/share/doc/plastimatch/ for Linux/UNIX
if (UNIX)
  # FHS compliant path for Linux/UNIX
  set (DOC_DESTINATION "share/doc/plastimatch")
else ()
  # Just throw TXTs into a doc folder for Windows
  set (DOC_DESTINATION "doc")
endif ()

install (FILES 
  README.TXT
  src/plastimatch/COPYRIGHT.TXT
  DESTINATION ${DOC_DESTINATION}
  )

# Debian doesn't like INSTALL, LICENSE included in documentation
if (NOT PLM_CONFIG_DEBIAN_BUILD)
  install (FILES 
    INSTALL.TXT
    src/plastimatch/LICENSE.TXT
    DESTINATION ${DOC_DESTINATION}
    )
endif ()

##-----------------------------------------------------------------------------
##  MAKE UNINSTALL
##-----------------------------------------------------------------------------
if (UNIX)
  configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target (uninstall COMMAND 
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif ()

##-----------------------------------------------------------------------------
##  Regression testing
##-----------------------------------------------------------------------------
##  * Tests need to be launched from a script, because CTest is not able 
##    to add the ITK DLL directory to the path.  We use CMAKE as scripting 
##    language to avoid requirement of outside package (e.g. python)
##  * "Release" is hard coded.  There is (apparently) no easy way to 
##    avoid this (i.e. no good suggestions on CMake/CTest email list.  
##    This is the purpose of the PATH_HACK code below.
##-----------------------------------------------------------------------------
if (ITK_FOUND)
if (WIN32 AND NOT CYGWIN AND NOT MINGW)
  set (PLM_PLASTIMATCH_PATH 
    ${CMAKE_CURRENT_BINARY_DIR}/Release)
  set (PLM_PLASTIMATCH_TESTING_PATH 
    ${CMAKE_CURRENT_BINARY_DIR}/Testing/Release)
  set (PLM_FFTW_PATH ${FFTW_DIR})
  # At some point in time (presumably around ITK 4.1), ITK stopped 
  # creating the variable ITK_LIBRARY_DIRS.  Therefore, we have to 
  # use another method to find the ITK directory needed to run tests.
  if (${ITK_VERSION} VERSION_LESS "4.1")
    set (PLM_ITK_LIBRARY_PATH_HACK 
      -DITK_LIBRARY_PATH=${ITK_LIBRARY_DIRS}/Release)
  else ()
    string (REGEX REPLACE "/[^/]*$" "" ITK_LIBRARY_DIRS_41
      "${ITK_CONFIG_TARGETS_FILE}")
    set (PLM_ITK_LIBRARY_PATH_HACK 
      -DITK_LIBRARY_PATH=${ITK_LIBRARY_DIRS_41}/bin/Release)
  endif ()
  set (PLM_PLASTIMATCH_PATH_HACK 
     -DPLM_PLASTIMATCH_PATH=${PLM_PLASTIMATCH_PATH})
  set (PLM_FFTW_PATH_HACK 
     -DPLM_FFTW_PATH=${PLM_FFTW_PATH})
else ()
  set (PLM_PLASTIMATCH_PATH ${CMAKE_CURRENT_BINARY_DIR})
  set (PLM_PLASTIMATCH_TESTING_PATH ${CMAKE_CURRENT_BINARY_DIR}/Testing)
  set (PLM_FFTW_PATH "${FFTW_DIR}")
  set (PLM_ITK_LIBRARY_PATH_HACK "")
  set (PLM_PLASTIMATCH_PATH_HACK "")
  set (PLM_FFTW_PATH_HACK "")
endif ()
endif ()

macro (PLM_ADD_TEST PLM_TEST_NAME PLM_TEST_COMMAND PARMS)
  # Optional extra parameters are passed through ${ARGN}
  # Allowed EXTRA_PARMS are:
  # "-DWORKING_DIR=XXX;-DEXPECTED_ERRNO=XXX"
  set (EXTRA_PARMS ${ARGV3})

  # CMake doesn't allow "=" to be passed in a -D parameter.  So we substitute 
  # with replacement string, which will get substituted back within the 
  # cmake script itself
  string (REPLACE "=" "&equal&" TMP_PARMS "${PARMS}")
  add_test (${PLM_TEST_NAME} ${CMAKE_COMMAND} 
    -DPLM_TEST_NAME=${PLM_TEST_NAME}
    ${PLM_ITK_LIBRARY_PATH_HACK}
    ${PLM_PLASTIMATCH_PATH_HACK}
    ${PLM_FFTW_PATH_HACK}
    -DPLM_TEST_COMMAND=${PLM_TEST_COMMAND}
    -DPLM_TESTING_SOURCE_DIR=${PLM_TESTING_SOURCE_DIR}
    -DPLM_BUILD_TESTING_DIR=${PLM_BUILD_TESTING_DIR}
    "-DPARMS=${TMP_PARMS}"
    ${EXTRA_PARMS}
    -P ${CMAKE_SOURCE_DIR}/cmake/RUN_CTEST.cmake
    )
endmacro ()

# Figure out which tests to ignore
# Note: we need two copies, because unix users might run "make test" 
# in either directory.  MSVC uses the one in ${CMAKE_BINARY_DIR}.
configure_file (
  ${PLM_TESTING_SOURCE_DIR}/CTestCustom.cmake.in
  ${CMAKE_BINARY_DIR}/CTestCustom.cmake
  @ONLY)
configure_file (
  ${PLM_TESTING_SOURCE_DIR}/CTestCustom.cmake.in
  ${CMAKE_BINARY_DIR}/Testing/CTestCustom.cmake
  @ONLY)

if (PLM_BUILD_TESTING)
  enable_testing ()
  add_subdirectory (Testing)

  # Copy the lconv script
  configure_file (
    "${CMAKE_SOURCE_DIR}/extra/devtools/run_lcov.sh" 
    "${CMAKE_BINARY_DIR}/run_lcov.sh"
    COPYONLY)
endif ()

##-----------------------------------------------------------------------------
##  Packaging
##-----------------------------------------------------------------------------
##  Simple version: "make package_source" to create tarball
##
##  Advanced version: to generate...
##   * zip    : cpack -G ZIP <source tree>
##   * tar.bz2: cpack -G TBZ2 <source tree>
##
##    ...from within your build directory
##-----------------------------------------------------------------------------
# Choose generator
if (PLM_PACKAGE_WIX)
  set (CPACK_GENERATOR "WIX")
elseif (PLM_PACKAGE_NSIS)
  set (CPACK_GENERATOR "NSIS")
else ()
  set (CPACK_GENERATOR "ZIP")
endif ()
set (CPACK_SOURCE_GENERATOR "TBZ2")

# For WiX install, and possibly for debian -dev targets,
# we should properly set up the install components.
# Until then, let's just kill it
set (CPACK_MONOLITHIC_INSTALL 1)

# General stuff
set (CPACK_INSTALL_PREFIX "")
set (CPACK_PACKAGE_CONTACT "plastimatch@googlegroups.com")
set (CPACK_PACKAGE_NAME "plastimatch")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Plastimatch - A Medical Imaging Application")
set (CPACK_PACKAGE_VENDOR "Plastimatch Dev Team")
set (CPACK_PACKAGE_VERSION_MAJOR "${PLM_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${PLM_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${PLM_VERSION_PATCH}")
set (CPACK_PACKAGE_VERSION_TWEAK ${PLASTIMATCH_REPOSITORY_VERSION})
set (CPACK_SOURCE_IGNORE_FILES
  "/\\\\.git"
  "/\\\\.svn"
  "~$"
)

# NSIS stuff
#set(CPACK_NSIS_INSTALLED_ICON_NAME "${APP_LOW_NAME}.ico")
set (CPACK_NSIS_HELP_LINK "http://plastimatch.org")
set (CPACK_NSIS_URL_INFO_ABOUT "http://plastimatch.org")
set (CPACK_NSIS_CONTACT "plastimatch@googlegroups.com")
set (PLM_NSIS_VERSION_STRING "${PLM_VERSION_MAJOR}.${PLM_VERSION_MINOR}.${PLM_VERSION_PATCH} (${PLASTIMATCH_REPOSITORY_VERSION})")

# WIX stuff
set (CPACK_WIX_HELP_LINK "http://plastimatch.org")
set (CPACK_WIX_UPGRADE_GUID "AA7C7964-14D7-4890-9CD1-EA1D80E4DC8C")
set (CPACK_WIX_LICENSE_RTF  "${PLM_WINDOWS_INSTALL_DIR}/License.rtf")
set (CPACK_WIX_UI_BANNER  "${PLM_WINDOWS_INSTALL_DIR}/plm_logo.jpg")
set (CPACK_WIX_UI_DIALOG  "${PLM_WINDOWS_INSTALL_DIR}/plm_logo_GUI.jpg")

# ZIP stuff
if (CPACK_GENERATOR STREQUAL "ZIP")
  set (CPACK_SET_DESTDIR ON)
endif ()

include (CPack)
