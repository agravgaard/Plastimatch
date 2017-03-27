
SET(LIB_NAME REG23GUI)

INCLUDE_DIRECTORIES(
  ${REG23_SOURCE_DIR}      # For reg23info.h (oraREG23AboutDialog.h)
  ${CMAKE_CURRENT_BINARY_DIR} # For headers produced by UIC
  ${QT_QTWEBKIT_INCLUDE_DIR} # For QtWebKit/QWebView
)

QT4_ADD_RESOURCES(RCC_SRCS
  oraREG23ControlWindow.qrc
  oraREG23RenderViewDialog.qrc
  oraREG23TaskPresentationWidget.qrc
  oraREG23AboutDialog.qrc
)

QT4_WRAP_UI(UI_SRCS 
  oraREG23ControlWindow.ui
  oraREG23RenderViewDialog.ui
  oraREG23TaskPresentationWidget.ui
  oraREG23AboutDialog.ui
)  

QT4_WRAP_CPP(MOC_SRCS 
  oraREG23ControlWindow.h
  oraREG23TaskPresentationWidget.h
  oraREG23RenderViewDialog.h
  oraREG23AboutDialog.h
)

ADD_LIBRARY(${LIB_NAME} ${REG23_LIBRARY_TYPE}
  ${RCC_SRCS}
  ${UI_SRCS}
  ${MOC_SRCS}
  oraREG23ControlWindow.cxx
  oraREG23TaskPresentationWidget.cxx
  oraREG23RenderViewDialog.cxx 	
  oraREG23AboutDialog.cxx
  vtkPerspectiveOverlayProjectionInteractorStyle.h
  vtkPerspectiveOverlayProjectionInteractorStyle.cxx 	
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(${LIB_NAME} 
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  
  REG23Core
  
  ORAIFMVC
  ORAIFImageAccess
  ORAIFImageTools
  ORAIFGUIComponents
  ORAIFModel
  
  QVTK
  vtkCommon
  vtkRendering
  vtkImaging
  vtkFiltering
  vtkGraphics
)

