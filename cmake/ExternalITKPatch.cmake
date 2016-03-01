##-----------------------------------------------------------------------------
##  Patch stock ITK 3.20.0 to make it work on gcc 4.6
##-----------------------------------------------------------------------------
execute_process (
  COMMAND "${CMAKE_COMMAND}"
    -E copy "${PLM_SOURCE_DIR}/libs/itk-3.20.0/metaUtils.cxx"
    "${PLM_TARGET_DIR}/${proj_itk}/Utilities/MetaIO/metaUtils.cxx"
)
execute_process (
  COMMAND "${CMAKE_COMMAND}"
    -E copy "${PLM_SOURCE_DIR}/libs/itk-3.20.0/itkImageIORegion.h"
    "${PLM_TARGET_DIR}/${proj_itk}/Code/IO/itkImageIORegion.h"
)
execute_process (
  COMMAND "${CMAKE_COMMAND}"
    -E copy "${PLM_SOURCE_DIR}/libs/itk-3.20.0/itkImageIOBase.h"
    "${PLM_TARGET_DIR}/${proj_itk}/Code/IO/itkImageIOBase.h"
)
