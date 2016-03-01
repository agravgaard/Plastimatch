##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_reconstruct_cuda)

set_directory_properties (PROPERTIES INCLUDE_DIRECTORIES "")
set_directory_properties (PROPERTIES COMPILE_DEFINITIONS "")

include_directories (BEFORE ${CMAKE_BINARY_DIR})
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/base)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/cuda)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/reconstruct)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/sys)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/util)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (AFTER ${DLIB_INCLUDE_DIR})
include_directories (AFTER ${MSINTTYPES_INCLUDE_DIR})

set (PLMRECONSTRUCT_CUDA_SRC
  drr_cuda.cu
  fdk_cuda.cu
  )

set (PLMRECONSTRUCT_CUDA_IFACE_SRC
  drr_cuda.cpp
  fdk_cuda.cpp
  )

set (PLMRECONSTRUCTCUDA_LIBRARY_SRC
  ${PLMRECONSTRUCT_CUDA_IFACE_SRC}
  ${PLMRECONSTRUCT_CUDA_SRC}
  )

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: CUDA
##-----------------------------------------------------------------------------
if (CUDA_FOUND)
  if (PLM_USE_GPU_PLUGINS)

    # build CUDA as separate plmreconstructcuda shared library
    plm_add_gpu_plugin_library (plmreconstructcuda 
      "${PLMRECONSTRUCTCUDA_LIBRARY_SRC}")
    target_link_libraries (plmreconstructcuda plmcuda ${CUDA_LIBRARIES})

    # set dependencies that plmreconstruct library needs 
    if (WIN32 AND NOT CYGWIN AND NOT MINGW)
      # windows needs to delayload it
      set (PLMRECONSTRUCT_LIBRARY_DEPENDENCIES
	${PLMRECONSTRUCT_LIBRARY_DEPENDENCIES}
	plmreconstructcuda)
      set (PLMRECONSTRUCT_LIBRARY_LDFLAGS
	"${PLMRECONSTRUCT_LIBRARY_LDFLAGS} /DELAYLOAD:plmreconstructcuda.dll")
    endif ()
    if (NOT WIN32 AND LIBDL_FOUND)
      # and unix needs dlopen()
      set (PLMRECONSTRUCT_LIBRARY_DEPENDENCIES
	${PLMRECONSTRUCT_LIBRARY_DEPENDENCIES}
	-ldl)
    endif ()
  else ()
    # building CUDA into plmreconstruct
    cuda_compile (CUDA_WRAPPERS ${PLMRECONSTRUCT_CUDA_SRC})
#    set (PLMRECONSTRUCT_LIBRARY_SRC
#      ${PLMRECONSTRUCT_LIBRARY_SRC}
#      ${PLMRECONSTRUCT_CUDA_IFACE_SRC}
#      ${CUDA_WRAPPERS}
#      )
#    set (PLMRECONSTRUCT_LIBRARY_DEPENDENCIES
#      ${PLMRECONSTRUCT_LIBRARY_DEPENDENCIES}
#      ${CUDA_LIBRARIES}
#      )
    set (PLMRECONSTRUCTCUDA_LIBRARY_SRC
      ${CUDA_WRAPPERS}
      ${PLMRECONSTRUCT_CUDA_IFACE_SRC}
      )
    plm_add_library (plmreconstructcuda
      "${PLMRECONSTRUCTCUDA_LIBRARY_SRC}"
      ""
      ""
      "")
  endif ()
endif ()
