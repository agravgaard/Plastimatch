##-----------------------------------------------------------------------------
##  Download ITK from internet and compile
##-----------------------------------------------------------------------------
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 2.8)
  return ()
endif ()

message (STATUS "Hello from ExternalITK")

set (proj_itk ITKv3)

set (itk_url http://sourceforge.net/projects/itk/files/itk/3.20/InsightToolkit-3.20.0.tar.gz)

ExternalProject_Add (${proj_itk}
  DOWNLOAD_DIR ${proj_itk}-download
  URL ${itk_url}
  URL_MD5 5d6b8e6e641624c6a8c06c7f95f05c9e
  PATCH_COMMAND "${CMAKE_COMMAND}" 
    -DPLM_SOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DPLM_TARGET_DIR=${CMAKE_BINARY_DIR}/${proj_itk}
    -P "${CMAKE_SOURCE_DIR}/cmake/ExternalITKPatch.cmake"
  SOURCE_DIR ${proj_itk}
  BINARY_DIR ${proj_itk}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DBUILD_TESTING:BOOL=OFF
    -DITK_USE_REVIEW:BOOL=ON
    -DITK_USE_REVIEW_STATISTICS:BOOL=ON
    -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON
  INSTALL_COMMAND ""
  )

set (ITK_DIR ${CMAKE_BINARY_DIR}/${proj_itk}-build)
