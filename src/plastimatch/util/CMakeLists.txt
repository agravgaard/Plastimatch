##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_util)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmutil_config.h.in
    ${PLM_BUILD_ROOT}/plmutil_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (AFTER ${CMAKE_SOURCE_DIR}/libs/devillard)

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLMUTIL_LIBRARY_SRC
  bspline_correspond.cxx
  dice_statistics.cxx dice_statistics.h
  dicom_sro_save.cxx
  distance_map.cxx
  diff.cxx
  dvh.cxx dvh.h
  gamma_dose_comparison.cxx gamma_dose_comparison.h
  geometry_chooser.cxx geometry_chooser.h
  hausdorff_distance.cxx hausdorff_distance.h
  image_boundary.cxx image_boundary.h
  image_center.cxx image_center.h
  itk_adjust.cxx
  itk_distance_map.cxx
  itk_crop.cxx
  itk_gabor.cxx
  itk_mask.cxx
  itk_scale.cxx
  itk_threshold.cxx
  itk_union.cxx
  landmark_diff.cxx
  proj_image_filter.cxx
  plm_series.cxx
  plm_study.cxx
  rt_study_warp.cxx rt_study_warp.h
  sift.cxx
  simplify_points.cxx
  ss_img_stats.cxx
  synthetic_mha.cxx
  synthetic_vf.cxx
  threshbox.cxx
  vf_invert.cxx
  )
if (FFTW_FOUND)
  set (PLMUTIL_LIBRARY_SRC
    ${PLMUTIL_LIBRARY_SRC}
    gabor.cxx gabor.h
    ramp_filter.cxx ramp_filter.h
   )
endif ()

set (PLMUTIL_LIBRARY_HEADERS
  "${CMAKE_BINARY_DIR}/plmutil_config.h"
  )
foreach (ARG ${PLMUTIL_LIBRARY_SRC})
  string (REGEX MATCH ".*\\.h$" TMP "${ARG}")
  if (TMP)
    list (APPEND PLMUTIL_LIBRARY_HEADERS "${TMP}")
  endif ()
endforeach ()

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMUTIL_LIBRARY_DEPENDENCIES
  ${FFTW_LIBRARIES}
  devillard
  plmbase
  plmsys
  )

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: OpenMP & SSE2
##-----------------------------------------------------------------------------
if (OPENMP_FOUND)
  set (PLMUTIL_LIBRARY_LDFLAGS "${OPENMP_LDFLAGS}")
  set_source_files_properties (dice_statistics.cxx
    PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
  set_source_files_properties (image_center.cxx
    PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
  set_source_files_properties (vf_invert.cxx
    PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmutil
  "${PLMUTIL_LIBRARY_SRC}"
  "${PLMUTIL_LIBRARY_DEPENDENCIES}"
  "${PLMUTIL_LIBRARY_LDFLAGS}"
  "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
  "${PLMUTIL_LIBRARY_HEADERS}")
