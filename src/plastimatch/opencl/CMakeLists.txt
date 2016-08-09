##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_opencl)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

set (PLMOPENCL_LIBRARY_SRC
  opencl_util.cxx
  )

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
if (OPENCL_FOUND)
  set (PLMOPENCL_LIBRARY_DEPENDENCIES
    plmsys
    ${OPENCL_LIBRARIES}
    )
  plm_add_library (
    plmopencl
    "${PLMOPENCL_LIBRARY_SRC}" 
    "${PLMOPENCL_LIBRARY_DEPENDENCIES}"
    "${PLMOPENCL_LIBRARY_LDFLAGS}"
    "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
    "")
endif ()
