##---------------------------------------------------------------------------
## See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##---------------------------------------------------------------------------

include (CheckCXXSourceCompiles)

macro (CHECK_QT QT_TEST_COMPILE_SUCCEEDED)
    # It took forever to get the quoting correct on this.
    # Thanks to sakra @ http://stackoverflow.com/questions/25726853
    try_compile (COMPILE_RESULT_VAR
	${CMAKE_BINARY_DIR}/helpme
	${CMAKE_SOURCE_DIR}/cmake/test_qt.cxx
	CMAKE_FLAGS 
	"-DINCLUDE_DIRECTORIES:STRING=${QT_INCLUDES}"
	"-DLINK_LIBRARIES:STRING=${QT_QTCORE_LIBRARIES}"
	OUTPUT_VARIABLE OUT_VAR
	)
    set (${QT_TEST_COMPILE_SUCCEEDED} ${COMPILE_RESULT_VAR})
endmacro ()
