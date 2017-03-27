
INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}
)
ADD_EXECUTABLE(TestORAStructureReader
  
  TestORAStructureReader.cxx
  
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkORAStructureReader.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkORAStructureReader.cxx
)
TARGET_LINK_LIBRARIES(TestORAStructureReader
  ORAIFStructureAccess
  vtkCommon
  vtkRendering
)
ADD_TEST(TestORAStructureReader ${EXECUTABLE_OUTPUT_PATH}/TestORAStructureReader -v)
SET_TESTS_PROPERTIES(TestORAStructureReader PROPERTIES TIMEOUT 30)
# Copy test-specific data to binary directory
ADD_CUSTOM_COMMAND(TARGET TestORAStructureReader POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E echo "moving test data ..."
)
ADD_CUSTOM_COMMAND(TARGET TestORAStructureReader POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/data
)
ADD_CUSTOM_COMMAND(TARGET TestORAStructureReader POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/2DStructureInfo.inf ${CMAKE_CURRENT_BINARY_DIR}/data 
)
ADD_CUSTOM_COMMAND(TARGET TestORAStructureReader POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/data/3DStructureInfo.inf ${CMAKE_CURRENT_BINARY_DIR}/data 
)

INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}
)
ADD_EXECUTABLE(TestSurfaceToPerspectiveProjectionImageFilter
  
  TestSurfaceToPerspectiveProjectionImageFilter.cxx
  
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkORAStructureReader.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkORAStructureReader.cxx
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkSurfaceToPerspectiveProjectionImageFilter.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkSurfaceToPerspectiveProjectionImageFilter.cxx
)
TARGET_LINK_LIBRARIES(TestSurfaceToPerspectiveProjectionImageFilter
  ORAIFStructureAccess
  vtkCommon
  vtkRendering
)
ADD_TEST(TestSurfaceToPerspectiveProjectionImageFilter ${EXECUTABLE_OUTPUT_PATH}/TestSurfaceToPerspectiveProjectionImageFilter -v)
SET_TESTS_PROPERTIES(TestSurfaceToPerspectiveProjectionImageFilter PROPERTIES TIMEOUT 30)


INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_BASICTESTING_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}
)
ADD_EXECUTABLE(TestSurfaceToPerspectiveProjectionContourFilter
  
  TestSurfaceToPerspectiveProjectionContourFilter.cxx
  
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkORAStructureReader.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkORAStructureReader.cxx
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkSurfaceToPerspectiveProjectionContourFilter.h
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_STRUCTUREACCESS_SUBDIR}/vtkSurfaceToPerspectiveProjectionContourFilter.cxx
)
TARGET_LINK_LIBRARIES(TestSurfaceToPerspectiveProjectionContourFilter
  ORAIFStructureAccess
  vtkCommon
  vtkRendering
  vtkHybrid
)
ADD_TEST(TestSurfaceToPerspectiveProjectionContourFilter ${EXECUTABLE_OUTPUT_PATH}/TestSurfaceToPerspectiveProjectionContourFilter -v)
SET_TESTS_PROPERTIES(TestSurfaceToPerspectiveProjectionContourFilter PROPERTIES TIMEOUT 30)

