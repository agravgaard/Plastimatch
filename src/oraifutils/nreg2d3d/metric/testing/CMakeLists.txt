
# average ranks filter test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_METRIC_SUBDIR}
)
ADD_EXECUTABLE(TestAverageRanksFilter
  
  TestAverageRanksFilter.cxx
)
TARGET_LINK_LIBRARIES(TestAverageRanksFilter
  ORAIFNReg2D3DMetrics
  ITKIO
)
ADD_TEST(TestAverageRanksFilter ${EXECUTABLE_OUTPUT_PATH}/TestAverageRanksFilter -v)


# stochastic rank correlation metric test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_METRIC_SUBDIR}
)
ADD_EXECUTABLE(TestStochasticRankCorrelationMetric
  
  TestStochasticRankCorrelationMetric.cxx
)
TARGET_LINK_LIBRARIES(TestStochasticRankCorrelationMetric
  ORAIFNReg2D3DMetrics
  ITKIO
)
ADD_TEST(TestStochasticRankCorrelationMetric ${EXECUTABLE_OUTPUT_PATH}/TestStochasticRankCorrelationMetric -v)


# gradient difference metric test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_METRIC_SUBDIR}
)
ADD_EXECUTABLE(TestGradientDifferenceMetric
  
  TestGradientDifferenceMetric.cxx
)
TARGET_LINK_LIBRARIES(TestGradientDifferenceMetric
  ORAIFNReg2D3DMetrics
  ITKIO
)
ADD_TEST(TestGradientDifferenceMetric ${EXECUTABLE_OUTPUT_PATH}/TestGradientDifferenceMetric -v)


# DRR ITF likelihood cost function test
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_METRIC_SUBDIR}
)
ADD_EXECUTABLE(TestDRRITFLikelihoodCostFunction
  
  TestDRRITFLikelihoodCostFunction.cxx
)
TARGET_LINK_LIBRARIES(TestDRRITFLikelihoodCostFunction
  ORAIFNReg2D3DMetrics
  ITKIO
)
ADD_TEST(TestDRRITFLikelihoodCostFunction ${EXECUTABLE_OUTPUT_PATH}/TestDRRITFLikelihoodCostFunction -v)

