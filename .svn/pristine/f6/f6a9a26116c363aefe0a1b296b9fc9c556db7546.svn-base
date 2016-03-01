##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_test_cuda)

set_directory_properties (PROPERTIES INCLUDE_DIRECTORIES "")
set_directory_properties (PROPERTIES COMPILE_DEFINITIONS "")

# Test executable -- cuda
if (CUDA_FOUND)
    cuda_compile (CUDA_TEST_WRAPPERS cuda_test.cu)
    plm_add_executable (cuda_test "${CUDA_TEST_WRAPPERS}" 
	"${CUDA_LIBRARIES}" "" 
	${BUILD_ALWAYS} ${INSTALL_NEVER})
    set_target_properties (cuda_test PROPERTIES LINKER_LANGUAGE CXX)

    # Test executable -- cuda_tex_test
    add_subdirectory (CUDA_tex_test)
endif ()
