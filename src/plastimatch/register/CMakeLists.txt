##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_register)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmregister_config.h.in
    ${PLM_BUILD_ROOT}/plmregister_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})

if (CUDA_FOUND)
  add_subdirectory (cuda)
  include_directories (BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/cuda")
endif ()



##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLMREGISTER_LIBRARY_SRC
  bspline.cxx bspline.h
  bspline_loop.txx
  bspline_landmarks.cxx bspline_landmarks.h
  bspline_gm.cxx bspline_gm.h
  bspline_gm.txx
  bspline_mi.cxx bspline_mi.h
  bspline_mi.txx
  bspline_mse.cxx bspline_mse.h
  bspline_mse.txx
  bspline_optimize.cxx bspline_optimize.h
  bspline_optimize_lbfgsb.cxx bspline_optimize_lbfgsb.h
  bspline_optimize_liblbfgs.cxx bspline_optimize_liblbfgs.h
  bspline_optimize_nlopt.cxx bspline_optimize_nlopt.h
  bspline_optimize_steepest.cxx bspline_optimize_steepest.h
  bspline_parms.cxx bspline_parms.h
  bspline_score.cxx bspline_score.h 
  bspline_stage.cxx bspline_stage.h 
  bspline_state.cxx bspline_state.h 
  bspline_regularize.cxx bspline_regularize.h 
  bspline_regularize_analytic.cxx bspline_regularize_analytic.h
  bspline_regularize_numeric.cxx bspline_regularize_numeric.h
  bspline_regularize_semi_analytic.cxx
  demons.cxx demons.h
  demons_cpu.cxx
  demons_opencl_p.h
  demons_state.cxx demons_state.h
  gpuit_demons.cxx gpuit_demons.h
  histogram.cxx histogram.h
  itk_align_center.cxx itk_align_center.h 
  itk_demons.cxx itk_demons.h
  itk_demons_registration_filter.h
  itk_demons_util.cxx itk_demons_util.h
  itk_diff_demons.cxx itk_diff_demons.h
  itk_fsf_demons.cxx itk_fsf_demons.h
  itk_log_demons.cxx itk_log_demons.h
  itk_sym_log_demons.cxx itk_sym_log_demons.h
  itk_optimizer.cxx itk_optimizer.h
  itk_registration.cxx itk_registration.h
  itk_registration_observer.cxx itk_registration_private.h
  itk_sym_log_demons.h
  itk_tps.cxx itk_tps.h
  joint_histogram.cxx joint_histogram.h
  landmark_warp.cxx landmark_warp.h
  metric_parms.cxx metric_parms.h 
  metric_state.cxx metric_state.h
  process_parms.cxx process_parms.h
  rbf_cluster.cxx rbf_cluster.h
  rbf_gauss.cxx rbf_gauss.h
  rbf_wendland.cxx rbf_wendland.h
  registration.cxx registration.h
  registration_data.cxx registration_data.h
  registration_parms.cxx registration_parms.h
  registration_resample.cxx registration_resample.h 
  registration_similarity_data.h
  shared_parms.cxx shared_parms.h
  similarity_metric_type.cxx similarity_metric_type.h
  stage_parms.cxx stage_parms.h
  translation_grid_search.cxx translation_grid_search.h 
  translation_mi.cxx translation_mi.h 
  translation_mse.cxx translation_mse.h 
  )

if (OPENCL_FOUND)
  set (PLMREGISTER_LIBRARY_SRC
    ${PLMREGISTER_LIBRARY_SRC}
    demons_opencl.cxx
   )
endif ()

if (PLM_BUILD_VISCOUS)
  set (PLMREGISTER_LIBRARY_SRC
    ${PLMREGISTER_LIBRARY_SRC}
    viscous.cxx
    )
endif ()

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMREGISTER_LIBRARY_DEPENDENCIES)

set (PLMREGISTER_LIBRARY_DEPENDENCIES 
  ${PLMREGISTER_LIBRARY_DEPENDENCIES}
  ${LIBLBFGS_LIBRARIES}
  plmbase
  plmsys
  plmutil
  )

if (CUDA_FOUND)
  set (PLMREGISTER_LIBRARY_DEPENDENCIES 
    ${PLMREGISTER_LIBRARY_DEPENDENCIES}
    plmcuda
    )
  set (PLMREGISTER_LIBRARY_DEPENDENCIES 
    ${PLMREGISTER_LIBRARY_DEPENDENCIES}
    plmregistercuda
    )
endif ()

if (OPENCL_FOUND)
  set (PLMREGISTER_LIBRARY_DEPENDENCIES 
    ${PLMREGISTER_LIBRARY_DEPENDENCIES}
    plmopencl
    )
endif ()

if (NLOPT_FOUND)
  set (PLMREGISTER_LIBRARY_DEPENDENCIES 
    ${PLMREGISTER_LIBRARY_DEPENDENCIES}
    ${NLOPT_LIBRARIES}
    )
endif ()

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: OpenMP & SSE2
##-----------------------------------------------------------------------------
if (OPENMP_FOUND)
  set (PLMREGISTER_LIBRARY_LDFLAGS "${OPENMP_LDFLAGS}")
  set_source_files_properties (bspline.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
  set_source_files_properties (bspline_gm.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
  set_source_files_properties (bspline_mi.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
  set_source_files_properties (bspline_mse.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
  set_source_files_properties (bspline_regularize_analytic.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
endif ()

# bspline registration benefits from SSE2
if (SSE2_FOUND AND NOT BUILD_AGAINST_SLICER3)
  plm_set_sse2_flags (bspline.cxx bspline_gm.cxx bspline_mi.cxx bspline_mse.cxx)
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmregister
  "${PLMREGISTER_LIBRARY_SRC}" 
  "${PLMREGISTER_LIBRARY_DEPENDENCIES}"
  "${PLMREGISTER_LIBRARY_LDFLAGS}"
  "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
  "")

# because plmregistercuda is dynamically loaded (not linked)
# CMake needs to be told explicitly that plmregister
# depends on it, so we tell it explicitly here
if (CUDA_FOUND)
  add_dependencies (plmregister plmregistercuda)
endif ()
