## This script that spits out the contents of two files, and then fails.
## It is used to debug debian build failures.

message (STATUS ">>${STDOUT_FILE}")
file (READ ${STDOUT_FILE} VAL)
message (STATUS "${VAL}")

message (STATUS ">>${STDERR_FILE}")
file (READ ${STDERR_FILE} VAL)
message (STATUS "${VAL}")

message (SEND_ERROR "An error")
