##---------------------------------------------------------------------------
## See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##---------------------------------------------------------------------------

macro (CHECK_EPSILON OUT_VAR)
  try_run (RUN_RESULT_VAR COMPILE_RESULT_VAR
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/cmake/test_eps.cxx
    RUN_OUTPUT_VARIABLE ${OUT_VAR})
endmacro ()
