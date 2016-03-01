
# DRREngine library
SET(OWN_LIB_NAME ORAIFNReg2D3DDRREngine)

INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_MATHS_SUBDIR} 
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_NREG2D3D_TRANSFORM_SUBDIR}
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_IMAGETOOLS_SUBDIR}
)

SET(${OWN_LIB_NAME}_HDRS
  oraGLSLDRRRayCasterDevMethods.hxx
  oraGLSLRayCastingCodeFragments.hxx    
  oraProjectionGeometry.h
  oraImageBasedProjectionGeometry.h    
  oraIntensityTransferFunction.h    
  oraCPUSiddonDRRFilter.h    
  oraGLSLRayCastingDRRFilter.h  
)

SET(${OWN_LIB_NAME}_TXX
  oraImageBasedProjectionGeometry.txx
  oraIntensityTransferFunction.txx
  oraCPUSiddonDRRFilter.txx
  oraGLSLRayCastingDRRFilter.txx 
)

SET(${OWN_LIB_NAME}_SRCS
  oraProjectionGeometry.cxx
  oraIntensityTransferFunction.cxx
)

IF(VTK_FOUND)
SET(${OWN_LIB_NAME}_HDRS
    ${${OWN_LIB_NAME}_HDRS}
    oraDRRFilter.h           
    oraGLSLDRRRayCastMapper.h
    oraRGBTableManager.h
    oraITKVTKDRRFilter.h    
    oraScalarFieldManager.h
    oraProjectionProperties.h        
    oraFlexMapCorrection.h    
    oraTranslationScaleFlexMapCorrection.h    
    oraLinacMountedImagingDeviceProjectionProperties.h    
    oraPortalImagingDeviceProjectionProperties.h    
    ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_IMAGETOOLS_SUBDIR}/oraVTKUnsharpMaskingImageFilter.h
    )
    
  SET(${OWN_LIB_NAME}_TXX
    ${${OWN_LIB_NAME}_TXX}
    oraDRRFilter.txx
    oraITKVTKDRRFilter.txx
    oraProjectionProperties.txx
    oraTranslationScaleFlexMapCorrection.txx
    oraLinacMountedImagingDeviceProjectionProperties.txx
    oraPortalImagingDeviceProjectionProperties.txx
    oraFlexMapCorrection.txx
  )
    
  SET(${OWN_LIB_NAME}_SRCS
    ${${OWN_LIB_NAME}_SRCS}
    oraGLSLDRRRayCastMapper.cxx  
    oraRGBTableManager.cxx
    oraScalarFieldManager.cxx
    ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_IMAGETOOLS_SUBDIR}/oraVTKUnsharpMaskingImageFilter.cxx
    )
ENDIF(VTK_FOUND)

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

# Support for invisible X windows on X environments. 
IF (VTK_FOUND AND UNIX AND NOT WIN32)
  SET(${OWN_LIB_NAME}_HDRS ${${OWN_LIB_NAME}_HDRS}
    oraInvisibleXOpenGLRenderWindow.h
  )
  
  SET(${OWN_LIB_NAME}_SRCS ${${OWN_LIB_NAME}_SRCS}
    oraInvisibleXOpenGLRenderWindow.cxx
  )
ENDIF(VTK_FOUND AND UNIX AND NOT WIN32)

ADD_LIBRARY(${OWN_LIB_NAME} ${ORAIFUTILS_LIBRARY_TYPE} ${${OWN_LIB_NAME}_SRCS} ${${OWN_LIB_NAME}_HDRS} ${${OWN_LIB_NAME}_TXX})
SET_TARGET_PROPERTIES(${OWN_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
SET(${OWN_LIB_NAME}_LIBRARIES ITKCommon ITKIO ITKAlgorithms ITKBasicFilters ITKNumerics)
IF(VTK_FOUND)
  SET(${OWN_LIB_NAME}_LIBRARIES ${${OWN_LIB_NAME}_LIBRARIES}
    vtkCommon vtkVolumeRendering vtkFiltering vtkRendering
    ORAIFImageTools)
ENDIF(VTK_FOUND)
TARGET_LINK_LIBRARIES(${OWN_LIB_NAME} ${${OWN_LIB_NAME}_LIBRARIES})

# Add headers to export-set. NOTE: Quotes are important here otherwise only the first header is installed!
SET_TARGET_PROPERTIES(${OWN_LIB_NAME} PROPERTIES PUBLIC_HEADER "${${OWN_LIB_NAME}_HDRS}")

INSTALL(TARGETS ${OWN_LIB_NAME}
  # Add the library to the export-set
  EXPORT ORAIFUTILSLibraryDepends
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib/static
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/${ORAIFUTILS_NREG2D3D_DRRENGINE_SUBDIR}"
  COMPONENT dev)
INSTALL(FILES ${${OWN_LIB_NAME}_TXX} DESTINATION "${INSTALL_INCLUDE_DIR}/${ORAIFUTILS_NREG2D3D_DRRENGINE_SUBDIR}")

# UNIT TEST
IF (ORAIFUTILS_BUILD_TESTING)
  ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/testing)
ENDIF (ORAIFUTILS_BUILD_TESTING)
