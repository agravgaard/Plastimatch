##-----------------------------------------------------------------------------
##  Welcome to the Plastimatch CMakeLists.txt file
##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch)

##-----------------------------------------------------------------------------
##  Local options
##-----------------------------------------------------------------------------

# Plastimatch software configuration options
option (PLM_CONFIG_CLANG_COMPLETE
    "Generate .clang_complete for hipster Vim-ers" OFF)
option (PLM_CONFIG_DISABLE_VISCOUS
    "Disable experimental viscous fluid registration algorithm" ON)
option (PLM_CONFIG_ENABLE_PLASTIMATCH_QT
    "Enable experimental plastimatch_qt executable" OFF)
option (PLM_CONFIG_PREFER_PATCHED_ITK
    "Prefer to use the patched version of certain ITK files" ON)
option (PLM_CONFIG_VOL_LIST
    "Native support for volumes with irregular slice thicknesses" OFF)

# Plastimatch legacy options
option (PLM_CONFIG_LEGACY_BSPLINE_EXTEND
  "Use legacy code for extending b-spline domain" OFF)
option (PLM_CONFIG_LEGACY_BSPLINE_XFORM_IO
  "Use legacy code for reading and writing b-spline xform files" OFF)
option (PLM_CONFIG_LEGACY_MI_METRIC
  "For ITK metrics, the legacy implementation of the mi metric is Viola-Wells to Mattes" OFF)
option (PLM_CONFIG_LEGACY_PROJ_GEO
  "Use legacy method for specifying projection geometry" ON)

##-----------------------------------------------------------------------------
##  Give a warning for obsolete dicom libraries
##-----------------------------------------------------------------------------
# If the user only has gdcm 2.x, give a warning
if (PLM_DCM_USE_GDCM2)
    message (WARNING "Plastimatch is being built with GDCM 2.X.  DICOM-RT functions are disabled.")
elseif (PLM_DCM_USE_GDCM1)
    message (WARNING "Plastimatch is being built with GDCM 1.X.  Certain features of DICOM-RT are disabled.")
endif ()


##-----------------------------------------------------------------------------
##  Figure out if we should patch itk
##-----------------------------------------------------------------------------
set (PLM_CONFIG_USE_PATCHED_ITK 0)
if (${ITK_VERSION} VERSION_LESS "4.1" AND PLM_CONFIG_PREFER_PATCHED_ITK)
  set (PLM_CONFIG_USE_PATCHED_ITK 1)
endif ()


##-----------------------------------------------------------------------------
##  Include directories
##-----------------------------------------------------------------------------
include_directories (BEFORE ${CMAKE_BINARY_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/base)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/cli)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/clp)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/dose)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/cuda)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/opencl)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/qt)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/reconstruct)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/reconstruct/cuda)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/register)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/register/cuda)
# GCS 2013-09-11.  Disable scripting, because it doesn't yet support 
#   smart pointers for registration.
# include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/script)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/segment)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/sys)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/util)

include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})
include_directories (AFTER ${CMAKE_SOURCE_DIR}/libs/demons_itk_insight)
include_directories (AFTER ${CMAKE_SOURCE_DIR}/libs/demons_itk_insight/DiffeomorphicDemons)
include_directories (AFTER ${CMAKE_SOURCE_DIR}/libs/demons_itk_insight/FastSymmetricForces)
include_directories (AFTER ${CMAKE_SOURCE_DIR}/libs/demons_itk_insight/LOGDomainDemons)
include_directories (AFTER ${CMAKE_SOURCE_DIR}/libs/nSIFT)
include_directories (AFTER ${DLIB_INCLUDE_DIR})
include_directories (AFTER ${LIBLBFGS_INCLUDE_DIR})
include_directories (AFTER ${MSINTTYPES_INCLUDE_DIR})
include_directories (AFTER ${RANSAC_INCLUDE_DIRS})

if (${PLM_CONFIG_USE_PATCHED_ITK})
    include_directories (AFTER ${CMAKE_SOURCE_DIR}/libs/itk-3.20.0)
endif ()

if (CUDA_FOUND)
  include_directories (AFTER ${CUDA_INCLUDE_DIRS})
endif ()
if (DCMTK_FOUND)
  include_directories (AFTER ${DCMTK_INCLUDE_DIR})
endif ()
if (FFTW_FOUND)
  include_directories (BEFORE ${FFTW_INCLUDE_DIR})
  link_directories (${FFTW_DIR})
endif ()
if (MATLAB_FOUND)
  include_directories (AFTER ${MATLAB_INCLUDE_DIRS})
endif ()
if (NLOPT_FOUND)
  include_directories (AFTER ${NLOPT_INCLUDE_DIR})
endif ()
if (OPENCL_FOUND)
  include_directories (BEFORE ${OPENCL_INCLUDE_DIRS})
endif ()
if (PANTHEIOS_FOUND)
  include_directories (AFTER ${STLSOFT_INCLUDE_DIR})
  include_directories (AFTER ${PANTHEIOS_INCLUDE_DIR})
endif ()
if (QT4_FOUND)
  if (QT_QTGUI_FOUND)
    include_directories (AFTER ${QT_QTGUI_INCLUDE_DIR})
  endif ()
  if (QT_QTSQL_FOUND)
    include_directories (AFTER ${QT_QTSQL_INCLUDE_DIR})
  endif ()
endif ()
if (RAPIDJSON_FOUND)
    include_directories (AFTER ${RAPIDJSON_INCLUDE_DIR})
endif ()
if (SQLITE_FOUND)
  include_directories (AFTER ${SQLITE_INCLUDE_DIR})
endif ()
if (LIBYAML_FOUND)
  include_directories (AFTER ${LIBYAML_INCLUDE_DIR})
endif ()

##-----------------------------------------------------------------------------
##  CONFIGURE INCLUDE FILES
##-----------------------------------------------------------------------------
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/sys/plm_config.h.in
  ${PLM_BUILD_ROOT}/plm_config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/sys/plm_version.h.in
  ${PLM_BUILD_ROOT}/plm_version.h)

##-----------------------------------------------------------------------------
## Option to generate .clang_complete for hip Vim users using clang_complete
##-----------------------------------------------------------------------------
if (PLM_CONFIG_CLANG_COMPLETE)
    get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
    set (file_clang_complete "${PLM_SRC_ROOT}/src/plastimatch/.clang_complete")
    foreach (arg ${inc_dirs})
        set (inc_args "${inc_args} -I${arg}\n")
    endforeach ()
    file (WRITE "${file_clang_complete}" "${inc_args}")
    message (STATUS "Generated ${file_clang_complete}")
endif ()

##-----------------------------------------------------------------------------
##  Do we have prerequisites for viscous code?
##-----------------------------------------------------------------------------
set (PLM_BUILD_VISCOUS FALSE)
if (NOT PLM_CONFIG_DISABLE_VISCOUS
    AND CUDA_FOUND AND CUDA_SDK_ROOT_DIR AND THRUST_FOUND)
  set (PLM_BUILD_VISCOUS TRUE)
endif ()


##  Specify which libraries of plastimatch should be built
if (PLM_CONFIG_LIBRARY_BUILD)
else ()
  set (PLMLIB_CONFIG_ENABLE_CLI true)
  set (PLMLIB_CONFIG_ENABLE_CLP true)
  set (PLMLIB_CONFIG_ENABLE_DOSE true)
  set (PLMLIB_CONFIG_ENABLE_QT true)
  set (PLMLIB_CONFIG_ENABLE_RECONSTRUCT true)
  set (PLMLIB_CONFIG_ENABLE_REGISTER true)
#  set (PLMLIB_CONFIG_ENABLE_SCRIPT true)
  set (PLMLIB_CONFIG_ENABLE_SEGMENT true)
  set (PLMLIB_CONFIG_ENABLE_STANDALONE true)
  set (PLMLIB_CONFIG_ENABLE_TEST true)
endif ()

if (PLMLIB_CONFIG_ENABLE_REGISTER OR PLMLIB_CONFIG_ENABLE_RECONSTRUCT)
  set (PLMLIB_CONFIG_ENABLE_CUDA true)
  set (PLMLIB_CONFIG_ENABLE_OPENCL true)
endif ()

## Specify which include directories are needed when
## compiling code that links to the libraries
set (PLASTIMATCH_INCLUDE_DIRECTORIES "")
if (NOT PLM_PACKAGE_LEGACY_CMAKE_CONFIG)
  list (APPEND PLASTIMATCH_INCLUDE_DIRECTORIES
    $<INSTALL_INTERFACE:${PLM_INSTALL_INCLUDE_DIR}>
    )
  list (APPEND PLASTIMATCH_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/plastimatch/sys>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/plastimatch/base>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/plastimatch/util>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libs/devillard>
    $<BUILD_INTERFACE:${DLIB_INCLUDE_DIR}>
    )
  if (PLMLIB_CONFIG_ENABLE_REGISTER)
    list (APPEND PLASTIMATCH_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/plastimatch/register>
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libs/liblbfgs-1.9/include>
      )
  endif ()
  if (PLMLIB_CONFIG_ENABLE_SEGMENT)
    list (APPEND PLASTIMATCH_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/plastimatch/segment>
      )
  endif ()
  if (PLMLIB_CONFIG_ENABLE_DOSE)
    list (APPEND PLASTIMATCH_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/plastimatch/dose>
      )
  endif ()
endif ()

# Core
add_subdirectory(base)
add_subdirectory(sys)
add_subdirectory(util)

# Optional
if (PLMLIB_CONFIG_ENABLE_CLI)
  add_subdirectory(cli)
endif ()
if (PLMLIB_CONFIG_ENABLE_CLP)
  add_subdirectory(clp)
endif ()
if (CUDA_FOUND AND PLMLIB_CONFIG_ENABLE_CUDA)
  add_subdirectory(cuda)
endif ()
if (PLMLIB_CONFIG_ENABLE_DOSE)
  add_subdirectory(dose)
endif ()
if (OPENCL_FOUND AND PLMLIB_CONFIG_ENABLE_OPENCL)
  add_subdirectory(opencl)
endif ()
if (QT_FOUND AND PLMLIB_CONFIG_ENABLE_QT)
  add_subdirectory(qt)
endif ()
if (PLMLIB_CONFIG_ENABLE_RECONSTRUCT)
  add_subdirectory(reconstruct)
endif ()
if (PLMLIB_CONFIG_ENABLE_REGISTER)
  add_subdirectory(register)
endif ()
#if (PLMLIB_CONFIG_ENABLE_SCRIPT)
#  add_subdirectory(script)
#endif ()
if (PLMLIB_CONFIG_ENABLE_SEGMENT)
  add_subdirectory(segment)
endif ()
if (PLMLIB_CONFIG_ENABLE_STANDALONE)
  add_subdirectory(standalone)
endif ()
if (PLMLIB_CONFIG_ENABLE_TEST)
  add_subdirectory(test)
endif ()

# Create a list of targets to be exported.  These are used by applications
# which link to plastimatch libraries.
set (EXPORT_TARGET_LIST plmsys plmbase devillard nkidecompress plmutil)
if (PLMLIB_CONFIG_ENABLE_DOSE)
  set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} specfun plmdose)
endif ()
if (PLMLIB_CONFIG_ENABLE_REGISTER)
  set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} lbfgs plmregister)
endif ()
if (PLMLIB_CONFIG_ENABLE_RECONSTRUCT)
  set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} plmreconstruct)
endif ()
if (CUDA_FOUND)
  if (PLMLIB_CONFIG_ENABLE_REGISTER OR PLMLIB_CONFIG_ENABLE_RECONSTRUCT)
    set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} plmcuda)
  endif ()
  if (PLMLIB_CONFIG_ENABLE_REGISTER)
    set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} plmregistercuda)
  endif ()
  if (PLMLIB_CONFIG_ENABLE_RECONSTRUCT)
    set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} plmreconstructcuda)
  endif ()
endif ()
if (OPENCL_FOUND)
  if (PLMLIB_CONFIG_ENABLE_REGISTER OR PLMLIB_CONFIG_ENABLE_RECONSTRUCT)
    set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} plmopencl)
  endif ()
endif ()
if (PLMLIB_CONFIG_ENABLE_SEGMENT)
  set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} plmsegment)
endif ()
#if (PLMLIB_CONFIG_ENABLE_SCRIPT)
#  set (EXPORT_TARGET_LIST ${EXPORT_TARGET_LIST} lua plmscript)
#endif ()

# Create the configuration files used by client applications
if (PLM_PACKAGE_LEGACY_CMAKE_CONFIG)

  export (TARGETS 
    ${EXPORT_TARGET_LIST}
    FILE "${CMAKE_BINARY_DIR}/PlastimatchLibraryDepends.cmake")

  # Help cmake find the PlastimatchConfig.cmake in the build directory
  if (NOT ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 2.8)
    export (PACKAGE Plastimatch)
  endif ()

  # Create PlastimatchBuildTreeSettings.cmake for the use from the build tree
  file (RELATIVE_PATH CONF_REL_INCLUDE_DIR "${PLM_INSTALL_CMAKE_DIR}"
    "${PLM_INSTALL_INCLUDE_DIR}")
  configure_file (
    "${PROJECT_SOURCE_DIR}/cmake/PlastimatchConfig-Legacy.cmake.in" 
    "${CMAKE_BINARY_DIR}/PlastimatchConfig.cmake" 
    @ONLY)
  configure_file (
    "${PROJECT_SOURCE_DIR}/cmake/PlastimatchBuildTreeSettings.cmake.in"
    "${CMAKE_BINARY_DIR}/PlastimatchBuildTreeSettings.cmake" @ONLY)

  # Install the PlastimatchConfig.cmake file
  install (FILES 
    "${CMAKE_BINARY_DIR}/PlastimatchConfig.cmake" 
    DESTINATION "${PLM_INSTALL_CMAKE_DIR}"
    # COMPONENT dev
    )

  # Install the export set for use with the install-tree
  install (EXPORT PlastimatchLibraryDepends 
    DESTINATION "${PLM_INSTALL_CMAKE_DIR}"
    # COMPONENT dev
    )

else (PLM_PACKAGE_LEGACY_CMAKE_CONFIG)

  include (CMakePackageConfigHelpers)

  # Make the version file
  write_basic_package_version_file (
    "${CMAKE_CURRENT_BINARY_DIR}/PlastimatchConfigVersion.cmake"
    VERSION ${PLM_BASIC_VERSION_STRING}
    COMPATIBILITY AnyNewerVersion
    )

  # Make the targets file
  if (NOT ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 3.0)
    export (EXPORT PlastimatchLibraryDepends
      FILE "${CMAKE_BINARY_DIR}/PlastimatchLibraryDepends.cmake"
      )
  endif ()
  
  # Make the config file
  configure_file (
    "${PROJECT_SOURCE_DIR}/cmake/PlastimatchConfig.cmake.in" 
    "${CMAKE_BINARY_DIR}/PlastimatchConfig.cmake" 
    @ONLY)
  
  # Install the files
  install (EXPORT PlastimatchLibraryDepends
    DESTINATION "${PLM_INSTALL_CMAKE_DIR}"
    )
  install (FILES 
    "${CMAKE_BINARY_DIR}/PlastimatchConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PlastimatchConfigVersion.cmake"
    DESTINATION "${PLM_INSTALL_CMAKE_DIR}"
    COMPONENT Devel
    )

endif (PLM_PACKAGE_LEGACY_CMAKE_CONFIG)

##-----------------------------------------------------------------------------
##  DOXYGEN
##-----------------------------------------------------------------------------
if (DOXYGEN_FOUND)
  file (MAKE_DIRECTORY "${PLM_BUILD_ROOT}/doc")
  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in 
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
    @ONLY)
  add_custom_target (doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY "${PLM_BUILD_ROOT}/doc"
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif ()
