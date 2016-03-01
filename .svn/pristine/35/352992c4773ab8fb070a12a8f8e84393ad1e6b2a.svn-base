
# ImageAccess library: RTI/ITK/VTK IO capabilities
SET(OWN_LIB_NAME ORAIFImageAccess)

INCLUDE_DIRECTORIES(
  ${ORAIFUTILS_SOURCE_DIR}/${ORAIFUTILS_TOOLS_SUBDIR}
)

SET(${OWN_LIB_NAME}_HDRS
  oraITKVTKImage.h  
  oraITKVTKImageMetaInformation.h
  oraITKVTKMetaInformationXMLFile.h
  itkRTIImageIO.h
  itkRTIImageIOFactory.h
  oraCompactRTISliceMetaInfo.h
  oraFrameOfReference.h
  oraImageList.h
  oraComplementaryMetaFileCache.h
  vtkMetaImageWriter2.h
  oraXRawImageIO.h  
)

SET(${OWN_LIB_NAME}_TXX
  oraITKVTKImage.txx
  oraXRawImageIO.txx  
)

SET(${OWN_LIB_NAME}_SRCS
  oraITKVTKImage.cxx
  oraITKVTKImageMetaInformation.cxx
  oraITKVTKMetaInformationXMLFile.cxx
  itkRTIImageIO.cxx
  itkRTIImageIOFactory.cxx
  oraCompactRTISliceMetaInfo.cxx
  oraFrameOfReference.cxx
  oraImageList.cxx
  oraComplementaryMetaFileCache.cxx
  vtkMetaImageWriter2.cxx
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

IF(ORAIFUTILS_USE_QT)
  SET(${OWN_LIB_NAME}_HDRS ${${OWN_LIB_NAME}_HDRS}
    oraRTIToMHDConverter.h    # Uses Qt-based ORAIFTools
  )
  
  SET(${OWN_LIB_NAME}_TXX ${${OWN_LIB_NAME}_TXX}
    oraRTIToMHDConverter.txx
  )
  
  SET(${OWN_LIB_NAME}_SRCS ${${OWN_LIB_NAME}_SRCS}
  )
  
  # Convince MSVC that txx files are also C++ files!
  IF(MSVC)
    SET_SOURCE_FILES_PROPERTIES(
      oraRTIToMHDConverter.txx 
      PROPERTIES 
      LANGUAGE "CXX" 
      HEADER_FILE_ONLY FALSE
      COMPILE_FLAGS ""
    )
  ENDIF(MSVC)
ENDIF(ORAIFUTILS_USE_QT)

ADD_LIBRARY(${OWN_LIB_NAME} ${ORAIFUTILS_LIBRARY_TYPE} ${${OWN_LIB_NAME}_SRCS} ${${OWN_LIB_NAME}_HDRS} ${${OWN_LIB_NAME}_TXX})
SET_TARGET_PROPERTIES(${OWN_LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# Add preprocessor defines from ITKVTKImage
# USE_ITK_FOR_WRITING: Determines whether to use ITK or VTK for writing
#    ORA-XML (extended metaimage) files.
# USE_XRAWIO_FOR_ALL: Determines whether to use XRawImageIO for both
#    reading and writing instead of the standard ITK/VTK implementations
#    (preferably yes!)
# FAST_XIO_COMPRESSION: Determines whether to use fast or default compression
#    for XRawImageIO (preferably yes!)
#
# NOTE: Qt dependencies in 'oraITKVTKMetaInformationXMLFile' have been replaced with 
# non-Qt equivalents. Qt-features could be enabled with -DUSE_QT.
# USE_QT: [oraITKVTKMetaInformationXMLFile] If defined the MD5 hashes are computed 
#    with Qt, else vtksys is used.
IF(ORAIFUTILS_USE_QT)
    SET(QT_USE_OPTION "-DUSE_QT")
ELSE(ORAIFUTILS_USE_QT)
    SET(QT_USE_OPTION "")
ENDIF(ORAIFUTILS_USE_QT)
ADD_DEFINITIONS(-DUSE_ITK_FOR_WRITING -DUSE_XRAWIO_FOR_ALL -DFAST_XIO_COMPRESSION ${QT_USE_OPTION})


SET(${OWN_LIB_NAME}_LINK_LIBRARIES 
  ORAIFTools
  ITKIO ITKCommon ITKBasicFilters
  vtkIO vtkCommon vtkFiltering vtkImaging vtkmetaio
)
IF(ORAIFUTILS_USE_QT)
  SET(${OWN_LIB_NAME}_LINK_LIBRARIES ${${OWN_LIB_NAME}_LINK_LIBRARIES} ${QT_QTCORE_LIBRARY})
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
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/${ORAIFUTILS_IMAGEACCESS_SUBDIR}"
  COMPONENT dev)
INSTALL(FILES ${${OWN_LIB_NAME}_TXX} DESTINATION "${INSTALL_INCLUDE_DIR}/${ORAIFUTILS_IMAGEACCESS_SUBDIR}")

# UNIT TEST
IF (ORAIFUTILS_BUILD_TESTING)
  ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/testing)
ENDIF (ORAIFUTILS_BUILD_TESTING)



