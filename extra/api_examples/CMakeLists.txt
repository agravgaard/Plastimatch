project (plastimatch_api_example)

cmake_minimum_required (VERSION 2.8.12)
if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif ()

find_package (Plastimatch)

# If you have multiple ITK installed on the computer, use the one 
# which plastimatch is linked with.
set (ITK_DIR "${PLASTIMATCH_ITK_DIR}")
find_package (ITK)

if (Plastimatch_FOUND)
  include (${ITK_USE_FILE})

  ## Legacy method requires this
  ## New method uses target dependencies
  include_directories (${PLASTIMATCH_INCLUDE_DIRS})
  
  add_executable (plastimatch_example_01 example_01.cxx)
  target_link_libraries (plastimatch_example_01 ${PLASTIMATCH_LIBRARIES})

  add_executable (plastimatch_example_02 example_02.cxx)
  target_link_libraries (plastimatch_example_02 ${PLASTIMATCH_LIBRARIES})

  add_executable (plastimatch_example_03 example_03.cxx)
  target_link_libraries (plastimatch_example_03 ${PLASTIMATCH_LIBRARIES})

else ()
  message (WARNING "Warning: plastimatch not found")
endif ()
