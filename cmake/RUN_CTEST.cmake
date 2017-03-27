## Some good info on using CTest:
##   http://www.cmake.org/pipermail/cmake/2007-April/013797.html
## Apparently this is how to make the CMake script return status
##   http://www.cmake.org/Wiki/CMake/C_Plugins_for_Loadable_Commands

set (ENV{PATH} "$ENV{PATH};${ITK_LIBRARY_PATH};${PLM_PLASTIMATCH_PATH};${PLM_FFTW_PATH}")

message ("PLM_TEST_COMMAND is ${PLM_TEST_COMMAND}")
message ("PARMS is ${PARMS}")
message ("EXPECTED_ERRNO is ${EXPECTED_ERRNO}")

# CMake doesn't allow "=" to be passed in a -D parameter.  So we substitute 
# with replacement string when calling ADD_TEST(), but need to back-substitute
# here.
string (REPLACE "&equal&" "=" PARMS "${PARMS}")

if (WORKING_DIR)
  set (WORKING_DIR WORKING_DIRECTORY ${WORKING_DIR})
endif ()

execute_process (
  COMMAND ${PLM_TEST_COMMAND} ${PARMS}
  ${WORKING_DIR}
  RESULT_VARIABLE RESULT
  OUTPUT_VARIABLE STDOUT
  ERROR_VARIABLE STDERR
)

message ("RETVAL: ${RESULT}")
message ("STDOUT: ${STDOUT}")
message ("STDERR: ${STDERR}")

# For CMake version 2.8 and higher, we can set execute permission
# http://www.mail-archive.com/cmake@cmake.org/msg26920.html
set (CMD_FN "${PLM_BUILD_TESTING_DIR}/${PLM_TEST_NAME}.cmd")
if (NOT ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 2.8)
  set (CMD_FN "${PLM_BUILD_TESTING_DIR}/tmp/${PLM_TEST_NAME}.cmd")
endif ()
file (WRITE ${CMD_FN} ${PLM_TEST_COMMAND})
foreach (PARM ${PARMS})
  file (APPEND ${CMD_FN} " \"" ${PARM} "\"")
endforeach ()
file (APPEND ${CMD_FN} "\n")
if (NOT ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 2.8)
  file (COPY "${CMD_FN}"
    DESTINATION "${PLM_BUILD_TESTING_DIR}"
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
    GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif ()

set (STDOUT_FN "${PLM_BUILD_TESTING_DIR}/${PLM_TEST_NAME}.stdout.txt")
file (WRITE ${STDOUT_FN} ${STDOUT})

set (STDERR_FN "${PLM_BUILD_TESTING_DIR}/${PLM_TEST_NAME}.stderr.txt")
file (WRITE ${STDERR_FN} ${STDERR})


if (NOT DEFINED EXPECTED_ERRNO)
  set (EXPECTED_ERRNO 0)
endif ()
if (${RESULT} EQUAL ${EXPECTED_ERRNO})
  message ("Not an error")
else ()
  message (SEND_ERROR "An error")
endif ()
