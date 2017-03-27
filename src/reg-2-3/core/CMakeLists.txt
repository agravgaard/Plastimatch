
SET(LIB_NAME REG23Core)

QT4_WRAP_CPP(MOC_SRCS 
  oraSimpleTransformUndoRedo.h
  oraAbstractTransformTask.h
  oraAutoRegistrationTransformTask.h
  oraSparseAutoRegistrationTransformTask.h
  oraInitialTransformTask.h
  oraManualRegistrationTransformTask.h
  oraReferenceTransformTask.h
)

ADD_LIBRARY(${LIB_NAME} ${REG23_LIBRARY_TYPE}
  ${MOC_SRCS}
  itkImageKernelOperator.h
  itkImageKernelOperator.txx
  oraREG23Model.h
  oraREG23Model.cxx
  oraREG23Model.txx
  oraREG23RegistrationInitializationTask.h
  oraREG23RegistrationInitializationTask.cxx
  oraREG23RegistrationExecutionTask.h
  oraREG23RegistrationExecutionTask.cxx
  oraREG23SparseRegistrationExecutionTask.h
  oraREG23SparseRegistrationExecutionTask.cxx
  oraSimpleTransformUndoRedo.h
  oraSimpleTransformUndoRedo.cxx
  oraAbstractTransformTask.h
  oraAbstractTransformTask.cxx
  oraAutoRegistrationTransformTask.h
  oraAutoRegistrationTransformTask.cxx
  oraSparseAutoRegistrationTransformTask.h
  oraSparseAutoRegistrationTransformTask.cxx    
  oraInitialTransformTask.h
  oraInitialTransformTask.cxx
  oraManualRegistrationTransformTask.h
  oraManualRegistrationTransformTask.cxx
  oraReferenceTransformTask.h
  oraReferenceTransformTask.cxx
) 

# Convince MSVC that txx files are also C++ files!
IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(
    itkImageKernelOperator.txx
    oraREG23Model.txx
    PROPERTIES 
    LANGUAGE "CXX" 
    HEADER_FILE_ONLY FALSE
    COMPILE_FLAGS ""
  )
ENDIF(MSVC)

SET(REG23Core_LIBRARIES 
  ${QT_QTCORE_LIBRARY}

  ORAIFTools
  ORAIFMVC
  ORAIFModel
  ORAIFImageAccess
  ORAIFImageTools
  ORAIFStructureAccess
  ORAIFRegistration
  ORAIFTransform
  ORAIFNReg2D3DDRREngine
  ORAIFNReg2D3DAlgorithm
  ORAIFNReg2D3DMetrics
  ORAIFNReg2D3DTransform
  
  ITKIO
  ITKCommon
  ITKBasicFilters
  ITKAlgorithms
  ITKNumerics
  ITKStatistics
  ITKSpatialObject
  
  vtkIO
  vtkCommon
  vtkRendering
  vtkGraphics
  vtkFiltering
  vtkImaging
)
## GCS: This is needed for vtk 5.6.1 default compile
if (VTK_USE_HYBRID)
  SET(REG23Core_LIBRARIES 
    ${REG23Core_LIBRARIES}
    vtkHybrid
    )
endif ()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(${LIB_NAME} ${REG23Core_LIBRARIES})
