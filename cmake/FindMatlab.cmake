SET (MATLAB_FOUND 0)

FIND_PROGRAM (MATLAB_EXE
  matlab
  )
IF (MATLAB_EXE)

  MESSAGE (STATUS "Probing matlab capabilities")

  FILE (WRITE "${CMAKE_BINARY_DIR}/probe_matlab.c"
"
#include \"mex.h\"
void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mxArray *v = mxCreateDoubleMatrix (1, 1, mxREAL);
  double *data = mxGetPr (v);
  *data = 1.23456789;
  plhs[0] = v;
}
"
  )
  FILE (WRITE "${CMAKE_BINARY_DIR}/probe_matlab_2.m"
"
disp(sprintf('mexext=%s',mexext));
disp(sprintf('matlabroot=%s',matlabroot));
%cpp_config = mex.getCompilerConfigurations('C++');
%disp(sprintf('cxxflags=%s',cpp_config.Details.CompilerFlags));
mex -v probe_matlab.c;
exit;
"
  )
  EXECUTE_PROCESS (COMMAND
    "${MATLAB_EXE}" -nosplash -nodisplay -r "probe_matlab_2"
    TIMEOUT 20
    RESULT_VARIABLE MATLAB_RESULT
    OUTPUT_VARIABLE MATLAB_STDOUT
    ERROR_VARIABLE MATLAB_STDERR
    )
  IF (MATLAB_STDOUT)
    STRING (REGEX MATCH "mexext *=[ ]*([^\n]*)" JUNK ${MATLAB_STDOUT})
    SET (MATLAB_MEXEXT "${CMAKE_MATCH_1}")
    STRING (REGEX MATCH "matlabroot *=[ \n]*([^\n]*)" JUNK ${MATLAB_STDOUT})
    SET (MATLAB_ROOT "${CMAKE_MATCH_1}")
    STRING (REGEX MATCH "cxxflags *=[ \n]*([^\n]*)" JUNK ${MATLAB_STDOUT})
    SET (MATLAB_CXXFLAGS "${CMAKE_MATCH_1}")
    STRING (REGEX MATCH "CXXFLAGS *= *([^\n]*)" JUNK ${MATLAB_STDOUT})
    SET (MATLAB_CXXFLAGS "${CMAKE_MATCH_1}")
    STRING (REGEX MATCH "CXXLIBS *= *([^\n]*)" JUNK ${MATLAB_STDOUT})
    SET (MATLAB_CXXLIBS "${CMAKE_MATCH_1}")
    STRING (REGEX MATCH "LD *= *([^\n]*)" JUNK ${MATLAB_STDOUT})
    SET (MATLAB_LD "${CMAKE_MATCH_1}")
    STRING (REGEX MATCH "LDFLAGS *= *([^\n]*)" JUNK ${MATLAB_STDOUT})
    SET (MATLAB_LDFLAGS "${CMAKE_MATCH_1}")
  ENDIF (MATLAB_STDOUT)

  #MESSAGE (STATUS "Matlab stdout = ${MATLAB_STDOUT}")
  MESSAGE (STATUS "Matlab root = ${MATLAB_ROOT}")
  MESSAGE (STATUS "MEX extension = ${MATLAB_MEXEXT}")
  MESSAGE (STATUS "MEX cxxflags = ${MATLAB_CXXFLAGS}")
  MESSAGE (STATUS "MEX cxxlibs = ${MATLAB_CXXLIBS}")
  MESSAGE (STATUS "MEX ld = ${MATLAB_LD}")
  MESSAGE (STATUS "MEX ldflags = ${MATLAB_LDFLAGS}")
  
  IF (MATLAB_MEXEXT)
    SET (MATLAB_FOUND 1)
    SET (MATLAB_INCLUDE_DIRS "${MATLAB_ROOT}/extern/include")
  ENDIF (MATLAB_MEXEXT)

ENDIF (MATLAB_EXE)

#######################################################################
## Macro for compiling mex files
#######################################################################
MACRO (MEX_TARGET
    TARGET_NAME TARGET_SRC TARGET_LIBS TARGET_LDFLAGS)

  # GCS: This mostly works, except that "-framework OpenCL" 
  # gives a link error when combined with 
  # "-Wl,-syslibroot,/Developer/SDKs/MacOSX10.5.sdk" 
  # It seems to work ok if I don't use ${MATLAB_LDFLAGS}
  SET (MEX_COMPILE_TGT 
    "${CMAKE_BINARY_DIR}/${TARGET_NAME}${MATLAB_LDEXTENSION}")
  SET (MEX_COMPILE_SRC "${CMAKE_SOURCE_DIR}/${TARGET_SRC}")

  ADD_LIBRARY (${TARGET_NAME} MODULE ${TARGET_SRC})
  TARGET_LINK_LIBRARIES (${TARGET_NAME} ${TARGET_LIBS} ${MATLAB_CXXLIBS})
  IF (NOT ${TARGET_LDFLAGS} STREQUAL "")
    SET_TARGET_PROPERTIES (${TARGET_NAME} 
      PROPERTIES LINK_FLAGS "${TARGET_LDFLAGS}")
  ENDIF (NOT ${TARGET_LDFLAGS} STREQUAL "")
  SET_TARGET_PROPERTIES (${TARGET_NAME} 
    PROPERTIES PREFIX "" SUFFIX ".${MATLAB_MEXEXT}")

ENDMACRO (MEX_TARGET)
