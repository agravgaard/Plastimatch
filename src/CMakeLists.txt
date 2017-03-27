##-----------------------------------------------------------------------------
##  Welcome to the Plastimatch CMakeLists.txt file
##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src)

##-----------------------------------------------------------------------------
##  PLASTIMATCH LIBRARIES
##-----------------------------------------------------------------------------
set (PLASTIMATCH_LIBS
  plmclp
#  plmscript
  plmsegment
  plmregister
  plmreconstruct
  plmdose
  plmutil
  plmbase
  plmsys
  ${ITK_LIBRARIES}
  devillard
  nkidecompress
#  lua
  ${MATH_LIB}
  )

if (CUDA_FOUND AND NOT PLM_USE_GPU_PLUGINS)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} plmcuda)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} plmreconstructcuda)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} plmregistercuda)
endif ()

if (DCMTK_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} ${DCMTK_LIBRARIES})
endif ()

if (DLIB_LIBRARIES)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} ${DLIB_LIBRARIES})
endif ()

if (FFTW_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} ${FFTW_LIBRARIES})
endif ()

if (LIBDL_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} dl)
endif ()

if (NLOPT_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} ${NLOPT_LIBRARIES})
endif ()

if (OPENCL_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} plmopencl)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} ${OPENCL_LIBRARIES})
endif ()

if (OPENMP_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} ${OPENMP_LIBRARIES})
endif ()

if (SPECFUN_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} specfun)
endif ()

if (QT4_FOUND)
  set (PLASTIMATCH_LIBS ${PLASTIMATCH_LIBS} ${QT_LIBRARIES})
endif ()

##-----------------------------------------------------------------------------
##  DETERMINE LINKER FLAGS
##-----------------------------------------------------------------------------
set (PLASTIMATCH_LDFLAGS "${OPENMP_LDFLAGS}")
if (PLM_USE_GPU_PLUGINS AND WIN32 AND NOT CYGWIN AND NOT MINGW)
  set (PLASTIMATCH_LDFLAGS
 "${PLASTIMATCH_LDFLAGS} /DELAYLOAD:plmregistercuda.dll /DELAYLOAD:plmreconstructcuda.dll")
endif ()

if (PLM_CONFIG_NOMANIFEST AND WIN32)
  set (PLASTIMATCH_LDFLAGS "${PLASTIMATCH_LDFLAGS} /MANIFEST:NO")
endif ()

##-----------------------------------------------------------------------------
##  Add subdirectories
##-----------------------------------------------------------------------------
if (PLM_CONFIG_DEBIAN_BUILD)
  set (PLM_CONFIG_DISABLE_ISE ON)
  set (PLM_CONFIG_DISABLE_FATM ON)
  set (PLM_CONFIG_DISABLE_MONDOSHOT ON)
  set (PLM_CONFIG_DISABLE_REG23 ON)
endif ()

if (NOT PLM_CONFIG_DISABLE_ISE)
  add_subdirectory (ise)
endif ()

if (NOT PLM_CONFIG_DISABLE_FATM)
  add_subdirectory (fatm)
endif ()

# mondoshot requires plastimatch, WIN32, wx, dcmtk, sqlite3
if (WIN32 AND NOT CYGWIN AND wxWidgets_FOUND AND DCMTK_FOUND
    AND NOT PLM_CONFIG_DISABLE_MONDOSHOT)
  add_subdirectory (mondoshot)
endif ()

if (NOT PLM_CONFIG_DISABLE_PLASTIMATCH)
  if (NOT ITK_FOUND)
    message (STATUS "Plastimatch will not be built (ITK not found)")
  else ()
    add_subdirectory (plastimatch)
  endif ()
endif ()

set (PLM_BUILD_ORAIFUTILS 1)
set (PLM_BUILD_REG23 1)
if (PLM_CONFIG_DISABLE_REG23)
  set (PLM_BUILD_ORAIFUTILS 0)
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 
    AND ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 2.8)
  message (STATUS "Reg-2-3 will not be built (CMake version)")
  set (PLM_BUILD_ORAIFUTILS 0)
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 AND NOT ITK_FOUND)
  message (STATUS "Reg-2-3 will not be built (ITK not found)")
  set (PLM_BUILD_ORAIFUTILS 0)
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 
    AND ${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR} VERSION_LESS 3.20)
  message (STATUS "Reg-2-3 will not be built (wrong ITK version)")
  set (PLM_BUILD_ORAIFUTILS 0)
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 
    AND NOT ${ITK_VERSION_MAJOR}.${ITK_VERSION_MINOR} VERSION_LESS 4.0)
  message (STATUS "Reg-2-3 will not be built (wrong ITK version)")
  set (PLM_BUILD_ORAIFUTILS 0)
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 AND ITK_USE_REVIEW_STATISTICS)
  message (STATUS 
    "Reg-2-3 will not be built (ITK compiled with review statistics)")
  set (PLM_BUILD_ORAIFUTILS 0)
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 AND NOT VTK_FOUND)
  message (STATUS "Reg-2-3 will not be built (VTK not found)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 AND NOT VTK_USE_QT)
  message (STATUS "Reg-2-3 will not be built (VTK not built with QT)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23
    AND ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_GREATER 5.10)
  message (STATUS "Reg-2-3 will not be built (wrong VTK version)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23
    AND ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} VERSION_LESS 5.6)
  message (STATUS "Reg-2-3 will not be built (wrong VTK version)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 AND NOT QT_FOUND)
  message (STATUS "Reg-2-3 will not be built (QT not found)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 
    AND ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR} VERSION_LESS 4.6)
  message (STATUS "Reg-2-3 will not be built (wrong QT version)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 
    AND ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR} VERSION_GREATER 4.8)
  message (STATUS "Reg-2-3 will not be built (wrong QT version)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 AND NOT QT_QMAKE_EXECUTABLE)
  message (STATUS "Reg-2-3 will not be built (no qmake executable)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()
if (PLM_BUILD_REG23 AND NOT QT_QTWEBKIT_FOUND)
  message (STATUS "Reg-2-3 will not be built (QtWebKit not found)")
  set (PLM_BUILD_ORAIFUTILS 0) ## REMOVE WHEN DONE
  set (PLM_BUILD_REG23 0)
endif ()

if (PLM_BUILD_ORAIFUTILS)
  add_subdirectory (oraifutils)
endif ()
if (PLM_BUILD_REG23)
  add_subdirectory (reg-2-3)
endif ()

