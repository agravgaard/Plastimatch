## This script compares a value in a text file against lower and 
##   upper thresholds

message (STATUS "INFILE is ${INFILE}")
message (STATUS "REGEX is ${REGEX}")
message (STATUS "LOWER_THRESH is ${LOWER_THRESH}")
message (STATUS "UPPER_THRESH is ${UPPER_THRESH}")

file (STRINGS ${INFILE} TEST_OUTPUT REGEX "${REGEX}")
message (STATUS "PARSED VALUE=|${TEST_OUTPUT}|")

string (REGEX MATCH "${REGEX}" TEST_OUTPUT ${TEST_OUTPUT})
set (TEST_OUTPUT ${CMAKE_MATCH_1})
message (STATUS "PARSED_VALUE=|${TEST_OUTPUT}|")

string (LENGTH "${CMAKE_MATCH_1}" MATCH_LENGTH)
if (MATCH_LENGTH GREATER 0
    AND NOT CMAKE_MATCH_1 LESS ${LOWER_THRESH} 
    AND NOT CMAKE_MATCH_1 GREATER ${UPPER_THRESH})
  message ("Not an error")
else ()
  message (SEND_ERROR "An error")
endif ()
