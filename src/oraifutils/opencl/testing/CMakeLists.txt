# openCL test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_OPENCL_SUBDIR}
)
ADD_EXECUTABLE(TestOpenCLWrapper
  
  TestOpenCLWrapper.cxx
  
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_OPENCL_SUBDIR}/oraOpenCLWrapper.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_OPENCL_SUBDIR}/oraOpenCLWrapper.cxx
  

)
TARGET_LINK_LIBRARIES(TestOpenCLWrapper
  ORAIFOpenCLWrapper
)
ADD_TEST(TestOpenCLWrapper ${EXECUTABLE_OUTPUT_PATH}/TestOpenCLWrapper -v)