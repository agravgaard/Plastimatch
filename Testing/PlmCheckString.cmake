## This script finds a regex in a file and compares the first match 
##   with an input string

message (STATUS "INFILE is ${INFILE}")
message (STATUS "REGEX is ${REGEX}")
message (STATUS "MATCH_STRING is ${MATCH_STRING}")

file (STRINGS ${INFILE} test_output REGEX "${REGEX}")
message (STATUS "PARSED VALUE=|${test_output}|")

string (REGEX MATCH "${REGEX}" test_output ${test_output})
set (test_output ${CMAKE_MATCH_1})
message (STATUS "PARSED_VALUE=|${test_output}|")

string (LENGTH "${test_output}" match_length)
message (STATUS "LENGTH=${match_length}")
message (STATUS "|${test_output}|${MATCH_STRING}|")
if ("${match_length}" GREATER 0
    AND "${test_output}" STREQUAL "${MATCH_STRING}")
  message("Not an error")
else ()
  message (SEND_ERROR "An error")
endif ()
