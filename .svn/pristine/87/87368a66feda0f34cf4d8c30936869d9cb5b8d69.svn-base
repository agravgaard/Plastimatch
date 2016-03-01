##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_segment)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmsegment_config.h.in
    ${PLM_BUILD_ROOT}/plmsegment_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLMSEGMENT_LIBRARY_DEPENDENCIES 
  plmregister
  plmutil
  plmbase
  plmsys
  ${ITK_LIBRARIES}
  )

set (PLMSEGMENT_LIBRARY_SRC
  autolabel.cxx
  autolabel_feature.cxx
  autolabel_parms.cxx
  autolabel_ransac_est.cxx
  autolabel_task.cxx
  autolabel_thumbnailer.cxx
  autolabel_trainer.cxx
  dlib_trainer.cxx
  mabs.cxx
  mabs_atlas_selection.cxx
  mabs_parms.cxx
  mabs_seg_weights.cxx mabs_seg_weights.h 
  mabs_stats.cxx
  mabs_staple.cxx
  mabs_vote.cxx
  ml_convert.cxx
  segment_body.cxx
 )

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: OpenMP
##-----------------------------------------------------------------------------
if (OPENMP_FOUND)
  set (PLMSEGMENT_LIBRARY_LDFLAGS "${OPENMP_LDFLAGS}")
  set_source_files_properties (mabs_vote.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmsegment
  "${PLMSEGMENT_LIBRARY_SRC}" 
  "${PLMSEGMENT_LIBRARY_DEPENDENCIES}"
  "${PLMSEGMENT_LIBRARY_LDFLAGS}"
  "")
