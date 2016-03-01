
# Metric library
SET(OWN_LIB_NAME ORAIFNReg2D3DMetrics)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_MATHS_SUBDIR}
)

SET(${OWN_LIB_NAME}_HDRS
  oraAverageRanksImageToImageFilter.h  
  oraStochasticRankCorrelationImageToImageMetric.h  
  oraDRRITFLikelihoodCostFunction.h  
  oraGradientDifferenceImageToImageMetric.h  
  oraGradientMagnitudeDifferenceImageToImageMetric.h
)

SET(${OWN_LIB_NAME}_TXX
  oraAverageRanksImageToImageFilter.txx
  oraStochasticRankCorrelationImageToImageMetric.txx
  oraDRRITFLikelihoodCostFunction.txx
  oraGradientDifferenceImageToImageMetric.txx
  oraGradientMagnitudeDifferenceImageToImageMetric.txx
)

SET(${OWN_LIB_NAME}_SRCS
)

# Convince MSVC that txx files are also C++ files!
IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(
    ${${OWN_LIB_NAME}_TXX}
    PROPERTIES 
    LANGUAGE "CXX" 
    HEADER_FILE_ONLY FALSE
    COMPILE_FLAGS ""
  )
ENDIF(MSVC)

ADD_LIBRARY(${OWN_LIB_NAME} ${ORAIFUTILS_LIBRARY_TYPE} ${${OWN_LIB_NAME}_SRCS} ${${OWN_LIB_NAME}_HDRS} ${${OWN_LIB_NAME}_TXX})
SET_TARGET_PROPERTIES(${OWN_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(${OWN_LIB_NAME} 
  ITKAlgorithms ITKBasicFilters ITKStatistics ORAIFNReg2D3DMathMethods
)

# Add headers to export-set. NOTE: Quotes are important here otherwise only the first header is installed!
SET_TARGET_PROPERTIES(${OWN_LIB_NAME} PROPERTIES PUBLIC_HEADER "${${OWN_LIB_NAME}_HDRS}")

INSTALL(TARGETS ${OWN_LIB_NAME}
  # Add the library to the export-set
  EXPORT ORAIFUTILSLibraryDepends
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib/static
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/${ORAIFUTILS_NREG2D3D_METRIC_SUBDIR}"
  COMPONENT dev)
INSTALL(FILES ${${OWN_LIB_NAME}_TXX} DESTINATION "${INSTALL_INCLUDE_DIR}/${ORAIFUTILS_NREG2D3D_METRIC_SUBDIR}")

# UNIT TEST
IF (ORAIFUTILS_BUILD_TESTING)
  ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/testing)
ENDIF (ORAIFUTILS_BUILD_TESTING)
