##-----------------------------------------------------------------------------
##  See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
##-----------------------------------------------------------------------------
project (src_plastimatch_base)

configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/plmbase_config.h.in
    ${PLM_BUILD_ROOT}/plmbase_config.h
)

include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories (AFTER ${NKIDECOMPRESS_INCLUDE_DIR})

##-----------------------------------------------------------------------------
##  SOURCE FILES
##-----------------------------------------------------------------------------
set (PLMBASE_LIBRARY_SRC
  aperture.cxx aperture.h 
  astroid_dose.cxx astroid_dose.h
  bspline_interpolate.cxx bspline_interpolate.h
  bspline_warp.cxx bspline_warp.h
  bspline_xform.cxx bspline_xform.h
  bspline_xform_legacy.cxx bspline_xform_legacy.h
  clamp.h
  cxt_extract.cxx cxt_extract.h
  cxt_io.cxx cxt_io.h
  dicom_probe.cxx dicom_probe.h
  dicom_util.cxx dicom_util.h
  dicom.dic   # How to make sure this gets installed?
  direction_cosines.cxx direction_cosines.h
  direction_matrices.cxx direction_matrices.h
  exchkeys.cxx
  float_pair_list.cxx float_pair_list.h
  gaussian.cxx gaussian.h 
  hnd_io.cxx hnd_io.h
  hnd_io_p.h
  interpolate.cxx interpolate.h
  interpolate_macros.h
  itkAndConstantToImageFilter.h
  itkClampCastImageFilter.txx itkClampCastImageFilter.h
  itk_dicom_load.cxx itk_dicom_load.h
  itk_dicom_save.cxx itk_dicom_save.h
  itk_directions.cxx itk_directions.h
  itk_image.cxx itk_image.h
  itk_image_accumulate.cxx itk_image_accumulate.h
  itk_image_cast.cxx itk_image_cast.h
  itk_image_clone.cxx itk_image_clone.h
#  itk_image_conv.cxx (broken)
  itk_image_create.cxx itk_image_create.h
  itk_image_load.txx itk_image_load.h
  itk_image_load_char.cxx
  itk_image_load_double.cxx
  itk_image_load_float.cxx
  itk_image_load_int32.cxx
  itk_image_load_short.cxx
  itk_image_load_uchar.cxx
  itk_image_load_uint32.cxx
  itk_image_load_ushort.cxx
  itk_image_load_vec.cxx
  itk_image_origin.cxx itk_image_origin.h 
  itk_image_region.cxx itk_image_region.h 
  itk_image_save.cxx itk_image_save.h
  itk_image_scale.cxx itk_image_scale.h 
  itk_image_stats.cxx itk_image_stats.h
  itk_image_type.h
  itk_metadata.cxx itk_metadata.h
  itk_point.h
  itk_pointset.cxx itk_pointset.h
  itk_resample.cxx itk_resample.h
  itk_volume_header.cxx itk_volume_header.h
  itk_warp.cxx itk_warp.h
  make_string.h
  mc_dose.cxx mc_dose.h
  metadata.cxx metadata.h
  mha_io.cxx mha_io.h
  nki_io.cxx nki_io.h
  parameter_parser.cxx parameter_parser.h
  plm_file_format.cxx plm_file_format.h
  plm_image.cxx plm_image.h 
  plm_image_p.h
  plm_image_convert.cxx
  plm_image_header.cxx plm_image_header.h
  plm_image_set.cxx plm_image_set.h
  plm_image_type.cxx plm_image_type.h
  plm_itk.h
  plm_uid_prefix.h
  plm_warp.cxx plm_warp.h
  pointset.cxx pointset.h
  pointset_warp.cxx pointset_warp.h
  proj_image.cxx proj_image.h
  proj_image_dir.cxx proj_image_dir.h
  proj_matrix.cxx proj_matrix.h
  proj_volume.cxx proj_volume.h
  pwlut.cxx pwlut.h 
  rasterize_slice.cxx rasterize_slice.h
  rasterizer.cxx rasterizer.h
  raw_pointset.cxx raw_pointset.h
  ray_data.h
  ray_trace_exact.cxx
  ray_trace_callback.h
  ray_trace_probe.cxx
  ray_trace_probe.h
  ray_trace_uniform.cxx
  rpl_volume.cxx rpl_volume.h
  rtds_dcmtk.cxx
  rtds_gdcm.cxx
  rt_study.cxx rt_study.h 
  rt_study_p.h
  rt_study_metadata.cxx rt_study_metadata.h 
  rtog_io.cxx
  rtss.cxx rtss.h 
  rtss_contour.cxx rtss_contour.h 
  rtss_roi.cxx rtss_roi.h 
  rtplan.cxx rtplan.h 
  rtplan_beam.cxx rtplan_beam.h 
  rtplan_control_pt.cxx rtplan_control_pt.h   
  segmentation.cxx segmentation.h 
  slice_extract.cxx slice_extract.h
  slice_list.cxx slice_list.h
  ss_img_extract.cxx ss_img_extract.h
  ss_list_io.cxx ss_list_io.h
  threading.h
  thumbnail.cxx thumbnail.h
  vf.cxx vf.h
  vf_convolve.cxx vf_convolve.h
  vf_jacobian.cxx vf_jacobian.h
  vf_stats.cxx vf_stats.h
  volume.cxx volume.h
  volume_conv.cxx volume_conv.h
  volume_fill.cxx volume_fill.h
  volume_gaussian.cxx volume_gaussian.h
  volume_grad.cxx volume_grad.h
  volume_header.cxx volume_header.h
  volume_limit.cxx volume_limit.h
  volume_macros.h
  volume_resample.cxx volume_resample.h
  volume_stats.cxx volume_stats.h
  xform.cxx xform.h
  xform_convert.cxx xform_convert.h
  xform_legacy.cxx xform_legacy.h
  xform_point.cxx xform_point.h
  xio_ct.cxx xio_ct.h
  xio_ct_transform.cxx xio_ct_transform.h
  xio_demographic.cxx xio_demographic.h
  xio_dir.cxx xio_dir.h
  xio_dose.cxx xio_dose.h
  xio_patient.cxx xio_patient.h
  xio_plan.cxx xio_plan.h
  xio_structures.cxx xio_structures.h
  xio_studyset.cxx xio_studyset.h
  xpm.cxx xpm.h xpm_p.h
  )

if (PLM_DCM_USE_GDCM1)
  set (PLMBASE_LIBRARY_SRC
    ${PLMBASE_LIBRARY_SRC}
    gdcm1_dose.cxx gdcm1_dose.h
    gdcm1_file.cxx gdcm1_file.h
    gdcm1_rdd.cxx gdcm1_rdd.h
    gdcm1_rtss.cxx gdcm1_rtss.h
    gdcm1_series.cxx gdcm1_series.h
    gdcm1_series_helper_2.cxx gdcm1_series_helper_2.h
    gdcm1_util.cxx gdcm1_util.h
    )
endif ()

if (PLM_DCM_USE_GDCM2)
  set (PLMBASE_LIBRARY_SRC
    ${PLMBASE_LIBRARY_SRC}
    gdcm2_util.cxx gdcm2_util.h
    )
endif ()

if (PLM_DCM_USE_DCMTK)
  set (PLMBASE_LIBRARY_SRC
    ${PLMBASE_LIBRARY_SRC}
    dcmtk_config.h
    dcmtk_file.cxx dcmtk_file.h
    dcmtk_image.cxx 
    dcmtk_loader.cxx dcmtk_loader.h
    dcmtk_loader_p.h
    dcmtk_metadata.cxx dcmtk_metadata.h
	dcmtk_module.cxx dcmtk_module.h
    #dcmtk_module_general_series.cxx dcmtk_module_general_series.h
    #dcmtk_module_general_study.cxx dcmtk_module_general_study.h
    #dcmtk_module_patient.cxx dcmtk_module_patient.h
    dcmtk_rdd.cxx dcmtk_rdd.h
    dcmtk_rtdose.cxx dcmtk_rtdose.h
    dcmtk_rtss.cxx dcmtk_rtss.h
    dcmtk_rtplan.cxx dcmtk_rtplan.h
    dcmtk_rt_study.cxx dcmtk_rt_study.h
    dcmtk_rt_study_p.cxx dcmtk_rt_study_p.h
    dcmtk_series.cxx dcmtk_series.h
    dcmtk_series_map.h
    dcmtk_sro.cxx dcmtk_sro.h
    dcmtk_uid.cxx dcmtk_uid.h
    dcmtk_util.cxx dcmtk_util.h
    )
endif ()

set (PLMBASE_LIBRARY_HEADERS
  "${CMAKE_BINARY_DIR}/plmbase_config.h"
  )
foreach (ARG ${PLMBASE_LIBRARY_SRC})
  string (REGEX MATCH ".*\\.h$" TMP "${ARG}")
  if (TMP)
    list (APPEND PLMBASE_LIBRARY_HEADERS "${TMP}")
  endif ()
endforeach ()

##-----------------------------------------------------------------------------
##  LIBRARY DEPENDENCIES
##-----------------------------------------------------------------------------
set (PLMBASE_LIBRARY_DEPENDENCIES
  ${ITK_LIBRARIES}
  nkidecompress
  plmsys
  )
if (DCMTK_FOUND)
  set (PLMBASE_LIBRARY_DEPENDENCIES
    ${PLMBASE_LIBRARY_DEPENDENCIES}
    ${DCMTK_LIBRARIES})
endif ()

##-----------------------------------------------------------------------------
##  SPECIAL BUILD RULES: OpenMP & SSE2
##-----------------------------------------------------------------------------
if (OPENMP_FOUND)
  set (PLMBASE_LIBRARY_LDFLAGS "${OPENMP_LDFLAGS}")
  set_source_files_properties (bspline_warp.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
  set_source_files_properties (volume_conv.cxx
    PROPERTIES COMPILE_FLAGS ${OPENMP_FLAGS})
endif ()

if (SSE2_FOUND)
  plm_set_sse2_flags (bspline_warp.cxx)
endif ()

##-----------------------------------------------------------------------------
##  BUILD TARGETS
##-----------------------------------------------------------------------------
plm_add_library (
  plmbase
  "${PLMBASE_LIBRARY_SRC}" 
  "${PLMBASE_LIBRARY_DEPENDENCIES}"
  "${PLMBASE_LIBRARY_LDFLAGS}"
  "${PLASTIMATCH_INCLUDE_DIRECTORIES}"
  "${PLMBASE_LIBRARY_HEADERS}"
  )
