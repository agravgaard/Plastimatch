
# tuple interpolation test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_MATHS_SUBDIR}
)
ADD_EXECUTABLE(Test1DTupleInterpolation
  Test1DTupleInterpolation.cxx
)
TARGET_LINK_LIBRARIES(Test1DTupleInterpolation
  ORAIFNReg2D3DMathMethods
)
ADD_TEST(Test1DTupleInterpolation ${EXECUTABLE_OUTPUT_PATH}/Test1DTupleInterpolation -v)

# interpolate function 1D test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_MATHS_SUBDIR}
)
ADD_EXECUTABLE(TestInterpolateFunction1D
   
  TestInterpolateFunction1D.cxx
  
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_MATHS_SUBDIR}/oraInterpolateFunction1D.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_MATHS_SUBDIR}/oraInterpolateFunction1D.cxx
)
TARGET_LINK_LIBRARIES(TestInterpolateFunction1D
  ORAIFNReg2D3DMathMethods
)
ADD_TEST(TestInterpolateFunction1D ${EXECUTABLE_OUTPUT_PATH}/TestInterpolateFunction1D -v)

