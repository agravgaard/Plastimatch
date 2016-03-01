project (liblbfgs)

set (LIBLBFGS_LIBRARY_SRC
  lib/lbfgs.c
  lib/arithmetic_ansi.h
  lib/arithmetic_sse_float.h
  lib/arithmetic_sse_double.h
  )
include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include)

plm_add_static_library (lbfgs "${LIBLBFGS_LIBRARY_SRC}" "" "" "")
