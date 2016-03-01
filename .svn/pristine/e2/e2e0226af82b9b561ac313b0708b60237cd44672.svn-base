##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_reconstruct)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmreconstruct_config.h.in
    ${PLM_BUILD_ROOT}/plmreconstruct_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

if (CUDA_FOUND)
  add_subdirectory (cuda)
  include_directories (BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/cuda")
endif ()

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLMRECONSTRUCT_LIBRARY_SRC
  bowtie_correction.cxx
  drr.cxx
  drr_trilin.cxx
  fdk.cxx
  fdk_util.cxx
  )
if (OPENCL_FOUND)
  set (PLMRECONSTRUCT_LIBRARY_SRC
    ${PLMRECONSTRUCT_LIBRARY_SRC}
    drr_opencl.cxx
    fdk_opencl.cxx
   )
endif ()

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMRECONSTRUCT_LIBRARY_DEPENDENCIES 
  plmutil
  plmbase
  plmsys
  )
if (CUDA_FOUND)
  set (PLMRECONSTRUCT_LIBRARY_DEPENDENCIES 
    ${PLMRECONSTRUCT_LIBRARY_DEPENDENCIES}
    plmcuda
    )
  set (PLMRECONSTRUCT_LIBRARY_DEPENDENCIES 
    ${PLMRECONSTRUCT_LIBRARY_DEPENDENCIES}
    plmreconstructcuda
    )
endif ()
if (OPENCL_FOUND)
  set (PLMRECONSTRUCT_LIBRARY_DEPENDENCIES 
    ${PLMRECONSTRUCT_LIBRARY_DEPENDENCIES}
    plmopencl
    )
endif ()

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: OpenMP
##-----------------------------------------------------------------------------
if (OPENMP_FOUND)
  set (PLMRECONSTRUCT_LIBRARY_LDFLAGS "${OPENMP_LDFLAGS}")
  set_source_files_properties (drr.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
  set_source_files_properties (fdk.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmreconstruct
  "${PLMRECONSTRUCT_LIBRARY_SRC}" 
  "${PLMRECONSTRUCT_LIBRARY_DEPENDENCIES}"
  "${PLMRECONSTRUCT_LIBRARY_LDFLAGS}"
  "")


if (CUDA_FOUND)
#  if (PLM_USE_GPU_PLUGINS)
    # because plmreconstructcuda is dynamically loaded (not linked)
    # CMake needs to be told explicitly that plmreconstruct
    # depends on it, so we tell it explicitly here
    add_dependencies (plmreconstruct plmreconstructcuda)
#  endif ()
endif ()
