##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_sys)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmsys_config.h.in
    ${PLM_BUILD_ROOT}/plmsys_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##  GCS 2012-09-28: Please include non-private header files as well
##-----------------------------------------------------------------------------
set (PLMSYS_LIBRARY_SRC
  compiler_warnings.h
  delayload.c delayload.h
  dir_list.cxx dir_list.h
  dlib_threads.cxx dlib_threads.h 
  file_util.cxx file_util.h
  logfile.cxx logfile.h
  option_range.cxx option_range.h 
  path_util.cxx path_util.h
  plm_endian.cxx plm_endian.h
  plm_exception.h 
  plm_fwrite.cxx plm_fwrite.h
  plm_int.h
  plm_macros.h
  plm_math.h
  plm_sleep.cxx plm_sleep.h 
  plm_timer.cxx plm_timer.h
  plm_version.h 
  print_and_exit.cxx print_and_exit.h
  string_util.cxx string_util.h
  )

set (PLMSYS_LIBRARY_HEADERS
  "${CMAKE_BINARY_DIR}/plm_config.h"
  "${CMAKE_BINARY_DIR}/plmsys_config.h"
  )
foreach (ARG ${PLMSYS_LIBRARY_SRC})
  string (REGEX MATCH ".*\\.h$" TMP "${ARG}")
  if (TMP)
    list (APPEND PLMSYS_LIBRARY_HEADERS "${TMP}")
  endif ()
endforeach ()

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMSYS_LIBRARY_DEPENDENCIES 
    )
if (LIBDL_FOUND)
    set (PLMSYS_LIBRARY_DEPENDENCIES
	${PLMSYS_LIBRARY_DEPENDENCIES}
	dl
	)
endif ()
if (CYGWIN OR NOT WIN32)
    find_library (pthreadlib pthread)
    if (pthreadlib)
	set (PLMSYS_LIBRARY_DEPENDENCIES
	    ${PLMSYS_LIBRARY_DEPENDENCIES}
	    ${pthreadlib}
	    )
    endif ()
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmsys
  "${PLMSYS_LIBRARY_SRC}" 
  "${PLMSYS_LIBRARY_DEPENDENCIES}"
  ""
  "${PLMSYS_LIBRARY_HEADERS}")
