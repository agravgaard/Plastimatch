
# GUI-components library
SET(OWN_LIB_NAME ORAIFGUIComponents)

IF(ORAIFUTILS_USE_QT)
  QT4_ADD_RESOURCES(RCC_SRCS
    oraXYPlotWidget.qrc
  )
  
  QT4_WRAP_CPP(MOC_SRCS 
    oraXYPlotWidget.h
    oraAdvancedQSlider.h
  )
  
  SET(${OWN_LIB_NAME}_HDRS 
    oraXYPlotWidget.h
    oraAdvancedQSlider.h
  )
  
  SET(${OWN_LIB_NAME}_SRCS
    ${RCC_SRCS}
    ${MOC_SRCS}    
    oraXYPlotWidget.cxx
    oraAdvancedQSlider.cxx
  )
ENDIF(ORAIFUTILS_USE_QT)

SET(${OWN_LIB_NAME}_HDRS 
  ${${OWN_LIB_NAME}_HDRS}
  vtk2DSceneImageInteractorStyle.h

)

SET(${OWN_LIB_NAME}_SRCS
  ${${OWN_LIB_NAME}_SRCS}
  vtk2DSceneImageInteractorStyle.cxx
)

ADD_LIBRARY(${OWN_LIB_NAME} ${ORAIFUTILS_LIBRARY_TYPE} ${${OWN_LIB_NAME}_SRCS} ${${OWN_LIB_NAME}_HDRS})
SET_TARGET_PROPERTIES(${OWN_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)

SET(${OWN_LIB_NAME}_LINK_LIBRARIES vtkCommon vtkImaging vtkRendering vtkFiltering)
IF(ORAIFUTILS_USE_QT)
  SET(${OWN_LIB_NAME}_LINK_LIBRARIES ${${OWN_LIB_NAME}_LINK_LIBRARIES} 
    ${QT_QTCORE_LIBRARY} ${QT_GUI_LIBRARY}
  )
ENDIF(ORAIFUTILS_USE_QT)
TARGET_LINK_LIBRARIES(${OWN_LIB_NAME} ${${OWN_LIB_NAME}_LINK_LIBRARIES})


# Add headers to export-set. NOTE: Quotes are important here otherwise only the first header is installed!
SET_TARGET_PROPERTIES(${OWN_LIB_NAME} PROPERTIES PUBLIC_HEADER "${${OWN_LIB_NAME}_HDRS}")

INSTALL(TARGETS ${OWN_LIB_NAME}
  # Add the library to the export-set
  EXPORT ORAIFUTILSLibraryDepends
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib/static
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/${ORAIFUTILS_GUICOMPONENTS_SUBDIR}"
  COMPONENT dev)


