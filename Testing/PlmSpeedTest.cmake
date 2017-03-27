## This script runs the speed tests

execute_process (
  COMMAND "${PLM_PLASTIMATCH_PATH}/plastimatch"
#  OUTPUT_FILE ${PLM_BUILD_TESTING_DIR}/hello.txt
  OUTPUT_VARIABLE STDOUT
  ERROR_VARIABLE STDOUT
  )
message (STATUS "${PLM_PLASTIMATCH_PATH}/plastimatch")
message (STATUS "${STDOUT}")

file (WRITE "${PLM_BUILD_TESTING_DIR}/hello.txt"
  "${PLM_PLASTIMATCH_PATH}/plastimatch\n")
file (APPEND "${PLM_BUILD_TESTING_DIR}/hello.txt"
  "STDOUT = ${STDOUT}\n")
file (APPEND "${PLM_BUILD_TESTING_DIR}/hello.txt"
  "STDERR = ${STDERR}\n")
