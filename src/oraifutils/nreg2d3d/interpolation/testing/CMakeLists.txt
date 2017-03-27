
# interpolator test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_INTERPOLATION_SUBDIR}
)
ADD_EXECUTABLE(TestOneToOneInterpolateImageFunction
  
  TestOneToOneInterpolateImageFunction.cxx
  
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_INTERPOLATION_SUBDIR}/oraOneToOneInterpolateImageFunction.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_INTERPOLATION_SUBDIR}/oraOneToOneInterpolateImageFunction.txx
)
TARGET_LINK_LIBRARIES(TestOneToOneInterpolateImageFunction
  ORAIFNReg2D3DInterpolation
)
ADD_TEST(TestOneToOneInterpolateImageFunction ${EXECUTABLE_OUTPUT_PATH}/TestOneToOneInterpolateImageFunction -v)
