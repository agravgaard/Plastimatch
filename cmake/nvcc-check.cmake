# James Shackleford
# Date: 2010.08.24
# File: nvcc-check.cmake
#
# 2012.11.21  <James Shackleford>
#   -- Complete rewrite to make checking different gcc
#        versions easier for the increasing number of nvcc versions
######################################################################

#: FUNCTION: get_nvcc_version ()
#:  RETURNS: CUDA_VERSION_MAJOR = <STRING: MAJOR_VERSION>
#:           CUDA_VERSION_MINOR = <STRING: MINOR_VERSION>
#  ----------------------------------------------------------------------------
function (get_nvcc_version)
    # We have to get NVCC version ourselves. FindCUDA.cmake version checks are
    # skipped for non-initial CMake configuration runs
    exec_program(${CUDA_NVCC_EXECUTABLE} ARGS "--version" OUTPUT_VARIABLE NVCC_OUT)
    string(REGEX REPLACE ".*release ([0-9]+)\\.([0-9]+).*" "\\1" NVCC_VERSION_MAJOR ${NVCC_OUT})
    string(REGEX REPLACE ".*release ([0-9]+)\\.([0-9]+).*" "\\2" NVCC_VERSION_MINOR ${NVCC_OUT})
    set (CUDA_VERSION_MAJOR ${NVCC_VERSION_MAJOR} PARENT_SCOPE)
    set (CUDA_VERSION_MINOR ${NVCC_VERSION_MINOR} PARENT_SCOPE)
    set (CUDA_VERSION_STRING "${NVCC_VERSION_MAJOR}.${NVCC_VERSION_MINOR}" PARENT_SCOPE)
endfunction ()

#: FUNCTION: find_gcc_version (major_version minor_version)
#:  RETURNS: NVC_GCC_STATUS = 0 [FAILURE]
#:                          = 1 [USE DEFAULT GCC]
#:                          = 2 [USE SELECT  GCC]
#:           NVC_SELECT_GCC = <STRING: PATH_TO_GCC>
#  ----------------------------------------------------------------------------
function (find_gcc_version major minor)
    set (NVC_GCC_STATUS "0" PARENT_SCOPE)

    exec_program (gcc ARGS "-dumpversion" OUTPUT_VARIABLE GCCVER)
    string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\1" GCCVER_MAJOR "${GCCVER}")
    string (REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\2" GCCVER_MINOR "${GCCVER}")

    if (GCCVER_MAJOR MATCHES "${major}" AND GCCVER_MINOR MATCHES "${minor}")
        set (NVC_GCC_STATUS "1" PARENT_SCOPE)
    else ()
        exec_program (which ARGS "gcc-${major}.${minor}" OUTPUT_VARIABLE GCC_XX RETURN_VALUE GCC_XX_EXIST)
        if (GCC_XX_EXIST EQUAL 0)
            set (NVC_GCC_STATUS "2" PARENT_SCOPE)
            set (NVC_SELECT_GCC ${GCC_XX} PARENT_SCOPE)
        endif ()
    endif ()
endfunction ()

#: FUNCTION: error_request_gcc_version (major_version minor_version)
#  ----------------------------------------------------------------------------
function (error_request_gcc_version major minor)
    message (FATAL_ERROR "nvcc-check: Please install gcc-${major}.${minor}, it is needed by nvcc \(CUDA\).\nNote that gcc-${major}.${minor} can be installed side-by-side with your current version of gcc.\nYou need not replace your current version of gcc; just make gcc-${major}.${minor} available as well so that nvcc can use it.\nDebian/Ubuntu users with root privilages may simply enter the following at a terminal prompt:\n sudo apt-get install gcc-${major}.${minor} g++-${major}.${minor}\n")
endfunction ()



#: Only perform the gcc version check if it is necessary
#  ----------------------------------------------------------------------------
if (CUDA_FOUND AND CMAKE_SYSTEM_NAME MATCHES "Linux" AND CMAKE_COMPILER_IS_GNUCC)

    get_nvcc_version ()
    message(STATUS "nvcc-check: NVCC Version is ${CUDA_VERSION_MAJOR}.${CUDA_VERSION_MINOR}")

    #: CUDA 2.X: UNSUPPORTED
    #  ----------------------------------------------------------------
    if (CUDA_VERSION_MAJOR MATCHES "2")
        message (FATAL_ERROR "nvcc-check: Plastimatch only supports CUDA 3.0+\n")
    endif ()

    #: CUDA 3.X+: gcc-4.3
    #  ----------------------------------------------------------------
    if (CUDA_VERSION_MAJOR MATCHES "3")
        find_gcc_version (4 3)

        # JAS 08.06.2012: default behavior in CUDA 4.1+
        list (APPEND CUDA_NVCC_FLAGS --host-compilation C++)
    endif ()

    #: CUDA 4.0, 4.1: gcc-4.4 or gcc-4.3
    #  ----------------------------------------------------------------
    if ((NVC_GCC_STATUS STREQUAL "0") AND 
	    (CUDA_VERSION_STRING VERSION_LESS "4.2"))
	find_gcc_version (4 4)
	if (NVC_GCC_STATUS MATCHES "0")
            find_gcc_version (4 3)
	endif ()
    endif ()

    #: CUDA 4.2+, CUDA 5.X+: gcc-4.6, gcc-4.4 or gcc-4.3
    #  ----------------------------------------------------------------
    if ((NVC_GCC_STATUS STREQUAL "0") AND 
	    (CUDA_VERSION_STRING VERSION_GREATER "4.1"))
	find_gcc_version (4 6)
	if (NVC_GCC_STATUS MATCHES "0")
            find_gcc_version (4 4)
	endif ()
	if (NVC_GCC_STATUS MATCHES "0")
            find_gcc_version (4 3)
	endif ()
    endif ()

    #: Set CUDA_NVCC_FLAGS and give the user a little feedback
    #  ----------------------------------------------------------------
    #: CASE 0: NO COMPATIBLE VERSION OF GCC WAS FOUND!
    if (NVC_GCC_STATUS MATCHES "0")
        if (CUDA_VERSION_MAJOR MATCHES "3")
            error_request_gcc_version (4 3)
        endif()
        if (CUDA_VERSION_MAJOR MATCHES "4")
            error_request_gcc_version (4 4)
        endif()
        if (CUDA_VERSION_MAJOR MATCHES "5")
            error_request_gcc_version (4 6)
        endif()
    endif ()

    #: CASE 1: SYSTEM DEFAULT GCC IS COMPATIBLE
    if (NVC_GCC_STATUS MATCHES "1")
        message (STATUS "nvcc-check: Using system default gcc for CUDA compilation.")
    endif ()

    #: CASE 2: COMPATIBLE VERSION OF GCC WAS FOUND (NOT SYSTEM DEFAULT)
    if (NVC_GCC_STATUS MATCHES "2")
        message (STATUS "nvcc-check: Using \"${NVC_SELECT_GCC}\" for CUDA compilation.")
        list (APPEND CUDA_NVCC_FLAGS --compiler-bindir ${NVC_SELECT_GCC})
    endif ()

endif ()


#: Exceptions for specific quirks in certain CUDA versions
#  ----------------------------------------------------------------
if (CUDA_FOUND AND CMAKE_COMPILER_IS_GNUCC)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "APPLE")
        # For CUDA 3.2: surface_functions.h does some non-compliant things, 
        #   so we tell g++ to ignore them when called via nvcc by passing the
        #   -fpermissive flag through the nvcc build trajectory. Unfortunately,
        #    nvcc will also blindly pass this flag to gcc, even though it is
        #    not valid; thus, resulting in TONS of warnings.  So, we 1st check
        #    the nvcc version number
        if (CUDA_VERSION_MAJOR MATCHES "3" AND CUDA_VERSION_MINOR MATCHES "2")
            list (APPEND CUDA_NVCC_FLAGS --compiler-options -fpermissive)
            message (STATUS "nvcc-check: CUDA 3.2 exception (-fpermissive enabled)")
        endif ()
    endif ()
endif ()



if (verbose)
    message(STATUS "nvcc-check: CUDA_NVCC_FLAGS=\"${CUDA_NVCC_FLAGS}\"")
endif ()
