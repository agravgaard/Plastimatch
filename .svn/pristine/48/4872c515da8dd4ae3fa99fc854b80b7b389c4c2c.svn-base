##---------------------------------------------------------------------------
## See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##---------------------------------------------------------------------------

macro (CHECK_CHAR_SIGN OUT_VAR)
  try_run (RUN_RESULT_VAR COMPILE_RESULT_VAR
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/cmake/char_is_signed.cxx
    RUN_OUTPUT_VARIABLE ${OUT_VAR})
endmacro ()
