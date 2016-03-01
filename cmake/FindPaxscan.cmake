# Varian Paxscan
# These variables are set:
#
#   PAXSCAN_FOUND
#   PAXSCAN_INCLUDE_DIR
#   PAXSCAN_LIBRARIES
#
# Only works on Windows

if (PAXSCAN_INCLUDE_DIR)
  # Already in cache, be silent
  set (Paxscan_FIND_QUIETLY TRUE)
endif ()

FIND_PATH (PAXSCAN_INCLUDE_DIR "HcpFuncDefs.h"
  "C:/Program Files/Varian/PaxscanL04/DeveloperFiles/Includes"
  )
FIND_LIBRARY (PAXSCAN_LIBRARIES VirtCp 
  "C:/Program Files/Varian/PaxscanL04/DeveloperFiles/VirtCpRel")

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (Paxscan DEFAULT_MSG 
  PAXSCAN_LIBRARIES PAXSCAN_INCLUDE_DIR)
