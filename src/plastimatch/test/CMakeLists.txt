##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_test)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

if (CUDA_FOUND)
  add_subdirectory (cuda)
  include_directories (BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/cuda")
endif ()

##-----------------------------------------------------------------------------
##  CMAKE FLAGS
##-----------------------------------------------------------------------------
option (PLM_CONFIG_BUILD_TEST_PROGRAMS
    "Build extra test program executables" OFF)

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
# Test executable -- plastimatch api
if (PLM_CONFIG_BUILD_TEST_PROGRAMS)
    plm_add_executable (api_test api_test.cxx
	"${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})

    # Test executable -- c++ properties
    plm_add_executable (cpp_eps_test cpp_eps_test.cxx
	"" "" 
	${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
    plm_add_executable (cpp_floating_point_test cpp_floating_point_test.cxx
	"" "" 
	${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
    plm_add_executable (cpp_limits_test cpp_limits_test.cxx
	"" "" 
	${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
    plm_add_executable (cpp_overflow_test cpp_overflow_test.cxx
	"" "" 
	${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
    plm_add_executable (cpp_template_test cpp_template_test.cxx
	"" "" 
	${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
    plm_add_executable (cpp_sizeof_test cpp_sizeof_test.cxx
	"${PLASTIMATCH_LIBS}" "${GPUIT_EXE_LDFLAGS}" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})

    # Test executable -- dcmtk
    if (ITK_FOUND AND DCMTK_FOUND)
	plm_add_executable (dcmtk_test dcmtk_test.cxx
	    "${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
    endif ()

    # Test executable -- dlib
    plm_add_executable (dlib_test dlib_test.cxx
	"${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})
    plm_add_executable (dlib_thread_test dlib_thread_test.cxx
      "${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
      ${BUILD_ALWAYS} ${INSTALL_NEVER})

    # Test executable -- gabor_test
    plm_add_executable (gabor_test gabor_test.cxx 
	"${PLASTIMATCH_LIBS}" "${GPUIT_EXE_LDFLAGS}" 
	${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})

    # Test executables -- itk
    if (ITK_FOUND)
	plm_add_executable (itk_test itk_test.cxx
	    "${PLASTIMATCH_LIBS}" "${GPUIT_EXE_LDFLAGS}" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
	plm_add_executable (itk_test_directions itk_test_directions.cxx
	    "${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
	plm_add_executable (itk_test_threads itk_thread_test.cxx
	    "${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
    endif ()

    # Test executable -- libyaml
    if (LIBYAML_FOUND)
	plm_add_executable (libyaml_test libyaml_test.cxx
	    "${LIBYAML_LIBRARIES}" "" 
	    ${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})
    endif ()

    # Test executable -- matlab
    if (MATLAB_FOUND)
	file (WRITE "${CMAKE_BINARY_DIR}/compile_mex_test.m"
	    "mex \"${CMAKE_SOURCE_DIR}/mex_test.c\";exit;\n")
	file (WRITE "${CMAKE_BINARY_DIR}/mex_test.cmake"
	    "EXECUTE_PROCESS (COMMAND ${MATLAB_EXE} -nosplash -nodesktop -nojvm
            -r compile_mex_test
            RESULT_VARIABLE RESULT
            OUTPUT_VARIABLE STDOUT
            ERROR_VARIABLE STDERR)\n")
	add_custom_command (
	    OUTPUT "${CMAKE_BINARY_DIR}/mex_test${MATLAB_MEXEXT}"
	    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/mex_test.cmake"
	    DEPENDS "${CMAKE_SOURCE_DIR}/mex_test.c")
	add_custom_target (mat_mex_test
	    DEPENDS "${CMAKE_BINARY_DIR}/mex_test${MATLAB_MEXEXT}")
	target_link_libraries (mex_test ${MATLAB_LIBRARIES})
    endif ()

    # Test executable -- octave
    if (OCTAVE_FOUND)
	add_custom_command (
	    OUTPUT "${CMAKE_BINARY_DIR}/mex_test.mex"
	    COMMAND ${OCTAVE_MKOCTFILE} --mex 
	    -o "${CMAKE_BINARY_DIR}/mex_test.mex" 
	    "${CMAKE_SOURCE_DIR}/mex_test.c"
	    DEPENDS "${CMAKE_SOURCE_DIR}/mex_test.c")
	add_custom_target (oct_mex_test
	    DEPENDS "${CMAKE_BINARY_DIR}/mex_test.mex")
    endif ()

    # Test executable -- nlopt
    if (NLOPT_FOUND)
	set (NLOPT_TEST_LIBS ${GPUIT_LIBRARIES} ${NLOPT_LIBRARIES})
	plm_add_executable (nlopt_test nlopt_test.c
	    "${NLOPT_TEST_LIBS}" "${OPENMP_LDFLAGS}" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
	set_target_properties (nlopt_test PROPERTIES LINKER_LANGUAGE CXX)
    endif ()

    # Test executable -- nm
    plm_add_executable (nelder_mead_test nelder_mead_test.cxx
	"${PLASTIMATCH_LIBS}" "${OPENMP_LDFLAGS}" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})

    # Test executable -- opencl
    if (OPENCL_FOUND)
	set (OPENCL_TEST_SRC
	    opencl_test.cxx opencl_test.h opencl_test.cl
	    "${CMAKE_BINARY_DIR}/opencl_test.cl"
	    )
	set (OPENCL_TEST_LIBS ${OPENCL_LIBRARIES} ${PLASTIMATCH_LIBS})
	plm_add_executable (opencl_test "${OPENCL_TEST_SRC}" 
	    "${OPENCL_TEST_LIBS}" "" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})

	# I don't yet know how to bundle the .cl file within the executable.
	# Therefore, copy the .cl into binary directory.
	add_custom_command (
	    OUTPUT "${CMAKE_BINARY_DIR}/opencl_test.cl" 
	    COMMAND ${CMAKE_COMMAND} "-E" "copy" 
	    "${CMAKE_CURRENT_SOURCE_DIR}/opencl_test.cl" 
	    "${CMAKE_BINARY_DIR}/opencl_test.cl" 
	    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/opencl_test.cl")
    endif ()

    # Test executable -- openmp
    if (OPENMP_FOUND)
	set_source_files_properties (openmp_test.cxx
	    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
	plm_add_executable (openmp_test openmp_test.cxx
	    "${PLASTIMATCH_LIBS}" "${OPENMP_LDFLAGS}" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
    endif ()

    # Test executable -- plm
    plm_add_executable (plm_restart_test plm_restart_test.cxx
	"${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})

    # Test executable -- qt
    if (QT4_FOUND)
	plm_add_executable (qt_test qt_test.cxx
	    "${QT_LIBRARIES}" "" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
    endif ()

    # Test executable -- ransac
    plm_add_executable (ransac_test ransac_test.cxx
	"${PLASTIMATCH_LIBS}" "${GPUIT_EXE_LDFLAGS}" 
	${BUILD_IF_NOT_SLICER_EXT} ${INSTALL_NEVER})

    # Test executable -- rapidjson
    if (RAPIDJSON_FOUND)
	plm_add_executable (rapidjson_test rapidjson_test.cxx
	    "${PLASTIMATCH_LIBS}" "${PLASTIMATCH_LDFLAGS}" 
	    ${BUILD_ALWAYS} ${INSTALL_NEVER})
    endif ()

    # Test executable -- smart pointers I
    ## find_package (TR1)
    plm_add_executable (smartp_test_i smartp_test_i.cxx
	"${PLASTIMATCH_LIBS}" "${GPUIT_EXE_LDFLAGS}" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})

    # Test executable -- smart pointers II
    plm_add_executable (smartp_test_ii smartp_test_ii.cxx
	"${PLASTIMATCH_LIBS}" "${GPUIT_EXE_LDFLAGS}" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})

endif ()
