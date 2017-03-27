##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_register_cuda)

set_directory_properties (PROPERTIES INCLUDE_DIRECTORIES "")
set_directory_properties (PROPERTIES COMPILE_DEFINITIONS "")

include_directories (BEFORE ${CMAKE_BINARY_DIR})
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/base)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/cuda)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/register)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/sys)
include_directories (BEFORE ${CMAKE_SOURCE_DIR}/src/plastimatch/util)
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (AFTER ${DLIB_INCLUDE_DIR})
include_directories (AFTER ${MSINTTYPES_INCLUDE_DIR})

include_directories (AFTER ${CUDA_INCLUDE_DIRS})
if (THRUST_INCLUDE_DIR)
  include_directories (AFTER ${THRUST_INCLUDE_DIR})
endif ()

set (PLMREGISTERCUDA_SRC
  bspline_cuda.cu
  bspline_cuda_kernels.h
  demons_cuda.cu
#  demons_cuda_kernel.cu
  )

set (PLMREGISTERCUDA_IFACE_SRC
  bspline_cuda.cxx
  )

if (PLM_BUILD_VISCOUS)
  set (PLMREGISTERCUDA_SRC
    ${PLMREGISTERCUDA_SRC}
    viscous_compute.cu
    viscous_convolution.cu
    viscous_finalize.cu
    viscous_funcHistogram.cu
    viscous_funcImageDomain.cu
    viscous_initialize.cu
    viscous_main.cu
    )
endif ()

set (PLMREGISTERCUDA_LIBRARY_SRC
  ${PLMREGISTERCUDA_IFACE_SRC}
  ${PLMREGISTERCUDA_SRC}
  )

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMREGISTERCUDA_LIBRARY_DEPENDENCIES
  plmutil plmbase plmcuda ${PLM_CUDA_LIBRARIES})
if (PLM_BUILD_VISCOUS)
  set (PLMREGISTERCUDA_LIBRARY_DEPENDENCIES 
    ${PLMREGISTERCUDA_LIBRARY_DEPENDENCIES}
    ${CUDA_CUBLAS_LIBRARIES})
endif ()

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: CUDA
##-----------------------------------------------------------------------------
if (CUDA_FOUND)
  if (PLM_USE_GPU_PLUGINS)

    # build CUDA as separate plmregistercuda shared library
    plm_add_gpu_plugin_library (plmregistercuda 
      "${PLMREGISTERCUDA_LIBRARY_SRC}")

    # GCS 2013-03-21.  Why can't we link plmregistercuda against plmbase?
    # I'm going to try it and see.
    target_link_libraries (plmregistercuda 
      ${PLMREGISTERCUDA_LIBRARY_DEPENDENCIES})

    # set dependencies that plmregister library needs 
    if (WIN32 AND NOT CYGWIN AND NOT MINGW)
      # windows needs to delayload it
      set (PLMREGISTER_LIBRARY_DEPENDENCIES
	${PLMREGISTER_LIBRARY_DEPENDENCIES}
	plmregistercuda)
      set (PLMREGISTER_LIBRARY_LDFLAGS
	"${PLMREGISTER_LIBRARY_LDFLAGS} /DELAYLOAD:plmregistercuda.dll")
    endif ()
    if (NOT WIN32 AND LIBDL_FOUND)
      # and unix needs dlopen()
      set (PLMREGISTER_LIBRARY_DEPENDENCIES
	${PLMREGISTER_LIBRARY_DEPENDENCIES}
	-ldl)
    endif ()
  else ()
    # building CUDA into plmregister
    cuda_compile (CUDA_WRAPPERS ${PLMREGISTERCUDA_SRC})
#    set (PLMREGISTER_LIBRARY_SRC
#      ${PLMREGISTER_LIBRARY_SRC}
#      ${PLMREGISTERCUDA_IFACE_SRC}
#      ${CUDA_WRAPPERS}
#      )
#    set (PLMREGISTER_LIBRARY_DEPENDENCIES
#      ${PLMREGISTER_LIBRARY_DEPENDENCIES}
#      ${CUDA_LIBRARIES}
#      )
    set (PLMREGISTERCUDA_LIBRARY_SRC
      ${CUDA_WRAPPERS}
      ${PLMREGISTERCUDA_IFACE_SRC}
      )
    plm_add_library (plmregistercuda
      "${PLMREGISTERCUDA_LIBRARY_SRC}"
      "${PLMREGISTERCUDA_LIBRARY_DEPENDENCIES}"
      ""
      "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
      "")
  endif ()
endif ()
