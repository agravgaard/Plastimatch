project (PLM_Testing)

## -------------------------------------------------------------------------
## TODO: create targets like "make check plm-reg-bsp-a"
## http://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
## -------------------------------------------------------------------------

## -------------------------------------------------------------------------
## Note on test dependencies.  Some versions of cmake (e.g. Windows cmake 
## version 2.8.2) have a bug where the dependency graph is created 
## incorrectly.  Specifically, if B depends on A, and C depends on 
## both A and B, the graph will be generated incorrectly.  
## -------------------------------------------------------------------------

## -------------------------------------------------------------------------
## Test and data download options
## -------------------------------------------------------------------------
option (PLM_REDUCED_TESTS 
  "Run fewer tests (used when debugging tests)" OFF)
option (PLM_DOWNLOAD_TEST_DATA
  "Additional testing data (download from internet)" OFF)

##-----------------------------------------------------------------------------
## Configure test input files
##-----------------------------------------------------------------------------
set (CONFIG_FILE_LIST
  "plm-bsp-mse-c.txt"
  "plm-bsp-mse-h.txt"
  "plm-bsp-mse-k.txt"
  "plm-bsp-mse-l.txt"
  "plm-bsp-mi-c.txt"
  "plm-bsp-mi-k.txt"
  "plm-bsp-gm-k.txt"
  "plm-bsp-sm-multi-a.txt"
  "plm-bsp-cuda.txt"
  "plm-bsp-rect.txt"
  "plm-bsp-resume.txt"
  "plm-bsp-logfile.txt"
  "plm-bsp-dcos-a.txt"
  "plm-bsp-dcos-b.txt"
  "plm-bsp-dcos-c.txt"
  "plm-bsp-dcos-d.txt"
  "plm-bsp-dcos-e.txt"
  "plm-bsp-dcos-f.txt"
  "plm-bsp-landmark-a.txt"
  "plm-bsp-landmark-b.txt"
  "plm-bsp-landmark-c.txt"
  "plm-bsp-landmark-d.txt"
  "plm-bsp-landmark-e.txt"
  "plm-bsp-double.txt"
  "plm-bsp-regularize-numeric.txt"
  "plm-reg-itk-translation.txt"
  "plm-reg-roi-a.txt"
  "plm-reg-roi-b.txt"
  "plm-reg-roi-c.txt"
  "plm-reg-roi-d.txt"
  "plm-reg-roi-e.txt"
  "plm-reg-stiffness-a.txt"
  "plm-reg-multi-a.txt"
  "plm-reg-multi-b.txt"
  "plm-reg-dv-itk-translation.txt"
  "plm-reg-itk-rigid-a.txt"
  "plm-reg-itk-rigid-b.txt"
  "plm-reg-itk-similarity.txt"
  "plm-reg-itk-bspline.txt"
  "plm-reg-process-a.txt"
  "plm-reg-trans-a.txt"
  "plm-reg-trans-b.txt"
  "plm-reg-trans-mi-a.txt"
  "plm-reg-autores-a.txt"
  "plm-reg-autores-b.txt"
  "proton-dose-1.txt"
  "proton-dose-2.txt"
  "proton-dose-3.txt"
  "proton-dose-4.txt"
  "proton-dose-5a.txt"
  "proton-dose-5g.txt"
  "proton-dose-6a.txt"
  "speedtest-a.txt"
  "wed-a.txt"
  "wed-b.txt"
)
foreach (CONFIG_FILE ${CONFIG_FILE_LIST})
    string (REGEX REPLACE "\\.txt$" "" PLM_TEST_NAME ${CONFIG_FILE})
    configure_file ("${PLM_TESTING_DATA_DIR}/${CONFIG_FILE}"
	"${PLM_BUILD_TESTING_DIR}/${CONFIG_FILE}"
	@ONLY)
endforeach ()

## -------------------------------------------------------------------------
## Testing macros
## -------------------------------------------------------------------------
macro (plmtest_check_interval 
    test_name stdout_file regex_string lower_thresh upper_thresh)
  string (REPLACE "\\" "\\\\" regex_escaped "${regex_string}")
  add_test (${test_name}
    ${CMAKE_COMMAND} 
    "-DINFILE=${stdout_file}"
    "-DREGEX=${regex_escaped}"
    "-DLOWER_THRESH=${lower_thresh}"
    "-DUPPER_THRESH=${upper_thresh}"
    -P "${PLM_TESTING_SOURCE_DIR}/PlmCheckInterval.cmake"
    )
endmacro ()

macro (plmtest_check_string
    test_name stdout_file regex_string match_string)
  string (REPLACE "\\" "\\\\" regex_escaped "${regex_string}")
  add_test (${test_name}
    ${CMAKE_COMMAND} 
    "-DINFILE=${stdout_file}"
    "-DREGEX=${regex_escaped}"
    "-DMATCH_STRING=${match_string}"
    -P "${PLM_TESTING_SOURCE_DIR}/PlmCheckString.cmake"
    )
endmacro ()

macro (plmtest_debug test_name stdout_file stderr_file)
  add_test (${test_name}
    ${CMAKE_COMMAND} 
    "-DSTDOUT_FILE=${stdout_file}"
    "-DSTDERR_FILE=${stderr_file}"
    -P "${PLM_TESTING_SOURCE_DIR}/PlmTestDebug.cmake"
    )
endmacro ()

## -------------------------------------------------------------------------
## Download data sets
## -------------------------------------------------------------------------
macro (plm_download_and_extract DOWNLOAD_URL DOWNLOAD_FILENAME DOWNLOAD_DIR)
  if (NOT IS_DIRECTORY "${PLM_TESTING_DOWNLOAD_DATA_DIR}")
    file (MAKE_DIRECTORY "${PLM_TESTING_DOWNLOAD_DATA_DIR}")
  endif ()
  if (NOT EXISTS ${DOWNLOAD_FILENAME})
    message (STATUS "Downloading test data")
    file (DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_FILENAME}")
  endif ()
  if (NOT EXISTS ${DOWNLOAD_DIR})
    message (STATUS "Untarring test data: ${DOWNLOAD_FILENAME}")
    execute_process (
      COMMAND 
      ${CMAKE_COMMAND} "-E" "tar" "xvzf" "${DOWNLOAD_FILENAME}"
      WORKING_DIRECTORY ${PLM_BUILD_TESTING_DIR}
      RESULT_VARIABLE untar_result
      OUTPUT_VARIABLE untar_output
      ERROR_VARIABLE untar_error)
    message (STATUS "Untar command returned: ${untar_result}\n${untar_output}\n${untar_error}")
  endif ()
endmacro ()

if (PLM_DOWNLOAD_TEST_DATA)
  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/1929/headphantom.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/headphantom.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/headphantom")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/1613/foot-dicomrt-aw-4.4.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/foot-dicomrt-aw-4.4.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/foot-dicomrt-aw-4.4")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/1632/chest-phantom-dicomrt-CERR4pt0beta2_25_Jan_2011.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/chest-phantom-dicomrt-CERR4pt0beta2_25_Jan_2011.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-CERR4pt0beta2_25_Jan_2011")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/2160/dicomrt-corvus-6.2.2.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/dicomrt-corvus-6.2.2.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/dicomrt-corvus-6.2.2")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/2162/dicomrt_xio-4.60-irregular-spacing.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/dicomrt_xio-4.60-irregular-spacing.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/xio-4.60-irregular-spacing")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/1187/rando-dicomrt-pinnacle3-8.2g.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/rando-dicomrt-pinnacle3-8.2g.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/rando-dicomrt-pinnacle3-8.2g")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/934/chest-phantom-dicomrt-xio-4.33.02.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/chest-phantom-dicomrt-xio-4.33.02.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/2227/dicomrt-33-structures.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/dicomrt-33-structures.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/dicomrt-33-structures")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/1127/varian-catphan.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/varian-catphan.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/varian-catphan")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/2159/rtog-corvus-6.2.2.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/rtog-corvus-6.2.2.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/rtog-corvus-6.2.2")

  plm_download_and_extract (
    "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/939/chest-phantom-xio-4.33.02.tar.gz"
    "${PLM_TESTING_DOWNLOAD_DATA_DIR}/chest-phantom-xio-4.33.02.tar.gz"
    "${PLM_BUILD_TESTING_DIR}/chest-phantom-xio-4.33.02")
endif ()

#### This seems to be a duplicate of the dicom-rt data, not the xio data
# if (PLM_DOWNLOAD_XIO_IRREGULAR)
#   plm_download_and_extract (
#     "http://forge.abcd.harvard.edu/gf/download/frsrelease/85/2163/xio-4.60-irregular-spacing.tar.gz"
#     "${PLM_TESTING_DOWNLOAD_DATA_DIR}/xio-4.60-irregular-spacing.tar.gz"
#     "${PLM_BUILD_TESTING_DIR}/20110509")
# endif ()


## -------------------------------------------------------------------------
## Copy plugin library
##   GCS: N.b. I don't think this is necessary.  There is no need to 
##   copy gpuit or plastimatch1 libraries.  Instead we set the extra path 
##   as an argument to plm_add_test.
## -------------------------------------------------------------------------
# JAS 2010.11.23
# Tests will fail if the test binaries cannot find the cuda plugin.
# So, we copy it into the testing directory.  This seems to work well.
if (PLM_USE_GPU_PLUGINS)
  # Because MSVC places stuff in Release/ or Debug/, etc
  # CMAKE_CFG_INTDIR is "Release/" or "Debug/" for Windows or "/" for Linux
  # In the Linux case, the extra "/" doesn't hurt anything.
  set (GPU_PLUGIN_BIN_DIR
    "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")

  if (CUDA_FOUND)
    # CMAKE_SHARED_LIBRARY_PREFIX is "" under Windows and "lib" under Linux
    # CMAKE_SHARE_LIBRARY_SUFFIX is ".dll" under Windows and ".so" under Linux
    set (PLMREGISTERCUDA_FILE
      "${CMAKE_SHARED_LIBRARY_PREFIX}plmregistercuda${CMAKE_SHARED_LIBRARY_SUFFIX}")

    # This tells CMake how to handle a target depending on
    # "Testing/[lib]plmcuda[.dll/.so]"
    add_custom_command (
      OUTPUT "${PLM_BUILD_TESTING_DIR}/${CUDA_PLUGIN_FILE}"
      COMMAND
      ${CMAKE_COMMAND} "-E" "copy"
      "${GPU_PLUGIN_BIN_DIR}/${CUDA_PLUGIN_FILE}"
      "${PLM_BUILD_TESTING_DIR}/${CUDA_PLUGIN_FILE}"
      DEPENDS plmregistercuda
      )

    # The Phony target
    # This is the Makefile equivalent of
    # Testing/plmcuda.dll : plmcuda.dll      (Under Windows)  *OR*
    # Testing/libplmcuda.so : libplmcuda.so  (Under Linux)
    #
    # The "rule" for handling this is the above custom command 
    add_custom_target (plmregistercuda_tcpy
      DEPENDS "${PLM_BUILD_TESTING_DIR}/${PLMREGISTER_FILE}"
      )

    # When using the plmregister cuda plugin, plmregister depends on it for GPU
    # acceleration.  So, we add it as a dependency here so plmregistercuda will
    # be copied to Testing whenever the user builds plmregister (or any target
    # that depends on plmregister).
    add_dependencies (plmregister plmcuda_tcpy)
  endif ()

#  if (OPENCL_FOUND)
#    set (OPENCL_PLUGIN_FILE
#      "${CMAKE_SHARED_LIBRARY_PREFIX}plmopencl${CMAKE_SHARED_LIBRARY_SUFFIX}")
#
#    add_custom_command (
#      OUTPUT "${PLM_BUILD_TESTING_DIR}/${OPENCL_PLUGIN_FILE}"
#      COMMAND
#      ${CMAKE_COMMAND} "-E" "copy"
#      "${GPU_PLUGIN_BIN_DIR}/${OPENCL_PLUGIN_FILE}"
#      "${PLM_BUILD_TESTING_DIR}/${OPENCL_PLUGIN_FILE}"
#      DEPENDS plmopencl
#      )
#
#    add_custom_target (plmopencl_tcpy
#      DEPENDS "${PLM_BUILD_TESTING_DIR}/${OPENCL_PLUGIN_FILE}"
#      )
#    add_dependencies (gpuit plmopencl_tcpy)
#  endif ()
endif ()

## -------------------------------------------------------------------------
## Custom test command
## -------------------------------------------------------------------------
add_custom_target (speedtest
  ${CMAKE_COMMAND} 
  -DPLM_PLASTIMATCH_PATH="${PLM_PLASTIMATCH_PATH}"
  -DPLM_BUILD_TESTING_DIR="${PLM_BUILD_TESTING_DIR}"
  -P "${PLM_TESTING_SOURCE_DIR}/PlmSpeedTest.cmake"
  )

## -------------------------------------------------------------------------
## Test cases
## -------------------------------------------------------------------------
## Create synthetic images
##   black-1           All black, slightly different geometry
##   donut-1           Centered donut
##   gabor-xx          Fibonacci Gabor
##   gauss-1           Centered gauss
##   gauss-2           Off-center gauss
##   gauss-3           Inverted gauss
##   gauss-4           Gauss with inverted pixel spacing
##   gauss-5           Even more off-center gauss
##   gauss-6           Off-center gauss, in off-center world coordinates
##   gauss-7           Off-center gauss with wider sigma
##   gauss-ushort-1    Centered gauss, ushort
##   gauss-ushort-2    Off-center gauss, ushort
##   lung-1            Synthetic lung, tumor position 0
##   lung-2            Synthetic lung, tumor position -5
##   ptv-1             rect PTV target for dose calculation
##   ptv-2             rect PTV target for dose calculation, smaller geometry
##   rect-1            Inverted rect, centered
##   rect-2            Standard rect, off-center
##   rect-3            Standard rect, centered
##   rect-4            Uchar rect, centered
##   rect-5            Standard rect, centered, direction cosines alt-1
##   rect-6            Standard rect, centered, direction cosines alt-2
##   rect-7            Standard rect, centered, direction cosines alt-3
##   rect-8            Uchar rect, off-center
##   rect-9            Uchar rect, centered, higher resolution
##   rect-12           Small rect, bg=0, fg=10
##   rect-13           Small rect, bg=0, fg=11
##   rect-14           Small, off-center rect, bg=0, fg=10
##   rect-15           Standard rect, slicer direction cosines
##   rect-16           Off-center rect, slicer direction cosines
##   rect-17           Water phantom, resolution 5x5x5, for dose calculation
##   rect-18	       Water phantom, very low resolution 20x20, dose calc.
##   rect-19	       Standard, but larger rectangle
##   rectarr-01        Two rectangles, both centered
##   rectarr-02        Two rectangles, lower rect offset
##   rectarr-m-01      Upper mask for rectarr-01 and rectarr-02
##   rectarr-m-02      Lower mask for rectarr-01 and rectarr-02
##   rectarr-03        Two rectangles, both centered (for testing gamma)
##   rectarr-04        Two rectangles, lower rect offset (for testing gamma)
##   roi-1             left = 0, right = 1
##   rgc-1             rect range compensator for dose calculation
##   sphere-1          Uchar sphere, bg=0, fg=255
##   sphere-2          Standard sphere, centered
##   sphere-3          Standard, bg=0, fg=1
##   sphere-4          Standard, radius = 150
## -------------------------------------------------------------------------
set (SYNTH_MHA_SIZE 38)

plm_add_test (
  "black-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/black-1.mha;--output-type;float;--pattern;rect;--origin;-25 -25 -25;--dim;40 40 40;--volume-size;50 50 50;--background;-1000;--foreground;-1000"
  )
plm_add_test (
  "donut-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/donut-1.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/donut-ss-1.mha;--output-type;float;--pattern;donut;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--donut-radius;10 10 10;--background;-1000;--foreground;0;--output-dicom;${PLM_BUILD_TESTING_DIR}/donut-1-dicom"
  )
plm_add_test (
  "gabor-01"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gabor-01.mha;--pattern;gabor;--origin;-5 -5 -5;--dim;11 11 11;--spacing;1 1 1;--gabor-k-fib;0 1;--gauss-center;0 0 0;--gauss-std;3 3 3;--background;0;--foreground;1"
  )
plm_add_test (
  "gauss-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output-type;float;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;-1000;--foreground;0;--gauss-center;0 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-2.mha;--output-type;float;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;-1000;--foreground;0;--gauss-center;10 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-3"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-3.mha;--output-type;float;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;0;--foreground;-1000;--gauss-center;10 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-4"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-4.mha;--output-type;float;--pattern;gauss;--origin;+25 +25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--spacing;-1.31579 -1.31579 2;--background;-1000;--foreground;0;--gauss-center;0 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-5"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-5.mha;--output-type;float;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;-1000;--foreground;0;--gauss-center;20 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-6"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-6.mha;--output-type;float;--pattern;gauss;--origin;-15 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;-1000;--foreground;0;--gauss-center;10 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-7"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-7.mha;--output-type;float;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;-1000;--foreground;0;--gauss-center;10 0 0;--gauss-std;20 20 20"
  )
plm_add_test (
  "gauss-double-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-double-1.mha;--output-type;double;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;0;--foreground;1000;--gauss-center;0 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-double-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-double-2.mha;--output-type;double;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;0;--foreground;1000;--gauss-center;10 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-ushort-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-ushort-1.mha;--output-type;ushort;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;0;--foreground;1000;--gauss-center;0 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "gauss-ushort-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/gauss-ushort-2.mha;--output-type;ushort;--pattern;gauss;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;0;--foreground;1000;--gauss-center;10 0 0;--gauss-std;10 10 10"
  )
plm_add_test (
  "lung-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--dicom-with-uids;false;--output;${PLM_BUILD_TESTING_DIR}/lung-1.mha;--output-prefix;${PLM_BUILD_TESTING_DIR}/lung-1-prefix;--output-dicom;${PLM_BUILD_TESTING_DIR}/lung-1-dicom;--pattern;lung;--lung-tumor-pos;0"
  )
plm_add_test (
  "lung-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--dicom-with-uids;false;--output;${PLM_BUILD_TESTING_DIR}/lung-2.mha;--output-dicom;${PLM_BUILD_TESTING_DIR}/lung-2-dicom;--pattern;lung;--lung-tumor-pos;-5"
  )
plm_add_test (
  "ptv-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/ptv-1.mha;--pattern;rect;--dim;60 60 60;--background;0;--foreground;6;--rect-size;20 20 20;--origin;-150 -150 -150;--spacing;5 5 5"
  )
plm_add_test (
  "ptv-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/ptv-2.mha;--pattern;rect;--dim;40 40 40;--background;0;--foreground;6;--rect-size;20 20 20;--origin;-100 -100 -100;--spacing;5 5 5"
  )
plm_add_test (
  "rect-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--dicom-with-uids;false;--output;${PLM_BUILD_TESTING_DIR}/rect-1.mha;--output-type;float;--output-dose-img;${PLM_BUILD_TESTING_DIR}/rect-1-dose-img.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/rect-1-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/rect-1-ss-list.txt;--output-dicom;${PLM_BUILD_TESTING_DIR}/rect-1-dicom;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;-1000"
  )
plm_add_test (
  "rect-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-2.mha;--output-type;float;--output-dicom;${PLM_BUILD_TESTING_DIR}/rect-2-dicom;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-30 70 -50 50 -50 50"
  )

plm_add_test (
  "rect-2s"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--spacing;13.158 13.158 13.158;--rect-size; 200 200 200;--output;${PLM_BUILD_TESTING_DIR}/rect-2s.mha"
  )

plm_add_test (
  "rect-3"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-3.mha;--output-type;float;--output-dicom;${PLM_BUILD_TESTING_DIR}/rect-3-dicom;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-50 50 -50 50 -50 50"
  )
plm_add_test (
  "rect-4"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-4.mha;--output-type;uchar;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;1;--rect-size;-50 50 -50 50 -50 50"
  )
plm_add_test (
  "rect-5"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-5.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-50 50 -50 50 -50 50;--origin;-300 -150 -250;--direction-cosines;rotated-1"
  )
plm_add_test (
  "rect-6"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-6.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-50 50 -50 50 -50 50;--origin;0 -300 -200;--direction-cosines;rotated-2"
  )
plm_add_test (
  "rect-7"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-7.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-50 50 -50 50 -50 50;--origin;100 300 -100;--direction-cosines;rotated-3"
  )
plm_add_test (
  "rect-8"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-8.mha;--output-type;uchar;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;1;--rect-size;-30 70 -50 50 -50 50"
  )
plm_add_test (
  "rect-9"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-9.mha;--output-type;uchar;--pattern;rect;--dim;47 47 47;--background;0;--foreground;1;--rect-size;-50 50 -50 50 -50 50"
  )
plm_add_test (
  "rect-10"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-10.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-200 50 -50 50 -50 50"
  )
plm_add_test (
  "rect-10-roi-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-10-roi-1.mha;--output-type;uchar;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;1;--rect-size;-60 60 -60 60 -60 60"
  )
plm_add_test (
  "rect-10-roi-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-10-roi-2.mha;--output-type;uchar;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;1;--rect-size;-220 60 -60 60 -60 60"
  )
plm_add_test (
  "rect-11"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-11.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;-1000;--rect-size;-40 60 -40 60 -40 60"
  )
plm_add_test (
  "rect-11-roi"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-11-roi.mha;--output-type;uchar;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;1;--rect-size;-50 80 -50 80 -50 80"
  )
plm_add_test (
  "rect-12"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-12.mha;--output-type;float;--pattern;rect;--dim;20 20 20;--origin;-9.5 -9.5 -9.5;--spacing;1 1 1;--background;0;--foreground;10;--rect-size;-4.5 4.5 -4.5 4.5 -4.5 4.5"
  )
plm_add_test (
  "rect-13"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-13.mha;--output-type;float;--pattern;rect;--dim;20 20 20;--origin;-9.5 -9.5 -9.5;--spacing;1 1 1;--background;0;--foreground;11;--rect-size;-4.5 4.5 -4.5 4.5 -4.5 4.5"
  )
plm_add_test (
  "rect-14"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-14.mha;--output-type;float;--pattern;rect;--dim;20 20 20;--origin;-9.5 -9.5 -9.5;--spacing;1 1 1;--background;0;--foreground;10;--rect-size;-2.5 6.5 -4.5 4.5 -4.5 4.5"
  )
plm_add_test (
  "rect-15"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-15.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-50 50 -50 50 -50 50;--origin;243.421 243.421 -243.421;--spacing;13.1579 13.1579 13.1579;--direction-cosines;-1 0 0 0 -1 0 0 0 1"
  )
plm_add_test (
  "rect-16"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-16.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-30 70 -50 50 -50 50;--origin;243.421 243.421 -243.421;--spacing;13.1579 13.1579 13.1579;--direction-cosines;-1 0 0 0 -1 0 0 0 1"
  )
plm_add_test (
  "rect-17"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-17.mha;--pattern;rect;--dim;60 60 60;--background;-1000;--foreground;0;--rect-size;-100 100 -100 100 -100 100;--origin;-150 -150 -150;--spacing;5 5 5"
  )
plm_add_test (
  "rect-18"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-18.mha;--pattern;rect;--dim;15 15 15;--background;-1000;--foreground;0;--rect-size;-100 100 -100 100 -100 100;--origin;-150 -150 -150;--spacing;20 20 20"
  )
plm_add_test (
  "rect-19"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rect-19.mha;--output-type;float;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--rect-size;-150 150 -150 150 -150 150"
  )
plm_add_test (
  "rectarr-01-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rectarr-01-a.mha;--output-type;float;--pattern;rect;--dim;30 50 30;--background;-1000;--foreground;0;--rect-size;-5 5 -15 -5 -5 5;--origin;-14.5 -24.5 -14.5;--spacing;1 1 1"
  )
plm_add_test (
  "rectarr-01"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--input;${PLM_BUILD_TESTING_DIR}/rectarr-01-a.mha;--output;${PLM_BUILD_TESTING_DIR}/rectarr-01.mha;--output-type;float;--pattern;rect;--background;-1000;--foreground;0;--rect-size;-5 5 5 15 -5 5"
  )
plm_add_test (
  "rectarr-02"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--input;${PLM_BUILD_TESTING_DIR}/rectarr-01-a.mha;--output;${PLM_BUILD_TESTING_DIR}/rectarr-02.mha;--output-type;float;--pattern;rect;--background;-1000;--foreground;0;--rect-size;-10 0 5 15 -5 5"
  )
set_property (TEST rectarr-01 APPEND PROPERTY DEPENDS rectarr-01-a)
set_property (TEST rectarr-02 APPEND PROPERTY DEPENDS rectarr-01-a)
plm_add_test (
  "rectarr-m-01"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--fixed;rectarr-01-a.mha;--output;${PLM_BUILD_TESTING_DIR}/rectarr-m-01.mha;--output-type;uchar;--pattern;rect;--background;0;--foreground;1;--rect-size;-13 13 -20 0 -13 13"
  )
plm_add_test (
  "rectarr-m-02"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--fixed;rectarr-01-a.mha;--output;${PLM_BUILD_TESTING_DIR}/rectarr-m-02.mha;--output-type;uchar;--pattern;rect;--background;0;--foreground;1;--rect-size;-13 13 0 20 -13 13"
  )
set_property (TEST rectarr-m-01 APPEND PROPERTY DEPENDS rectarr-01-a)
set_property (TEST rectarr-m-02 APPEND PROPERTY DEPENDS rectarr-01-a)
plm_add_test (
  "rectarr-03-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rectarr-03-a.mha;--output-type;float;--pattern;rect;--dim;30 50 30;--background;0;--foreground;70;--rect-size;-5 5 -15 -5 -5 5;--origin;-14.5 -24.5 -14.5;--spacing;1 1 1"
  )
plm_add_test (
  "rectarr-03"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--input;${PLM_BUILD_TESTING_DIR}/rectarr-03-a.mha;--output;${PLM_BUILD_TESTING_DIR}/rectarr-03.mha;--output-type;float;--pattern;rect;--background;0;--foreground;50;--rect-size;-5 5 5 15 -5 5"
  )
plm_add_test (
  "rectarr-04"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--input;${PLM_BUILD_TESTING_DIR}/rectarr-03-a.mha;--output;${PLM_BUILD_TESTING_DIR}/rectarr-04.mha;--output-type;float;--pattern;rect;--background;0;--foreground;50;--rect-size;-10 0 5 15 -5 5"
  )
set_property (TEST rectarr-03 APPEND PROPERTY DEPENDS rectarr-03-a)
set_property (TEST rectarr-04 APPEND PROPERTY DEPENDS rectarr-03-a)
plm_add_test (
  "rgc-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/rgc-1.mha;--output-type;float;--pattern;rect;--dim;10 10 1;--background;0;--foreground;30;--rect-size;0 10 0 5 0 0;--origin;0 0 0;--spacing;1 1 1"
  )
plm_add_test (
    "roi-1"
    ${PLM_PLASTIMATCH_PATH}/plastimatch
    "synth;--output;${PLM_BUILD_TESTING_DIR}/roi-1.mha;--output-type;uchar;--pattern;rect;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;1;--rect-size;0 300 -300 300 -300 300"
    )
plm_add_test (
  "sphere-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/sphere-1.mha;--output-type;uchar;--pattern;sphere;--origin;-25 -25 -25;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--volume-size;50 50 50;--background;0;--foreground;255;--sphere-center;0 0 0;--sphere-radius;10 10 10"
  )
plm_add_test (
  "sphere-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/sphere-2.mha;--output-type;float;--pattern;sphere;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--sphere-center;0 0 0;--sphere-radius;50 50 50"
  )
plm_add_test (
  "sphere-3"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/sphere-3.mha;--output-type;float;--pattern;sphere;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;0;--foreground;1;--sphere-center;0 0 0;--sphere-radius;50 50 50"
  )
plm_add_test (
    "sphere-4"
    ${PLM_PLASTIMATCH_PATH}/plastimatch
    "synth;--output;${PLM_BUILD_TESTING_DIR}/sphere-4.mha;--output-type;float;--pattern;sphere;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE};--background;-1000;--foreground;0;--sphere-center;0 0 0;--sphere-radius;150 150 150"
    )
plm_add_test (
  "xramp-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/xramp-1.mha;--output-type;float;--pattern;xramp;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )
plm_add_test (
  "yramp-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/yramp-1.mha;--output-type;float;--pattern;yramp;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )
plm_add_test (
  "zramp-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth;--output;${PLM_BUILD_TESTING_DIR}/zramp-1.mha;--output-type;float;--pattern;zramp;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )

## -------------------------------------------------------------------------
## Create synthetic vector fields
##   vf-zero         Zero vector field
##   vf-trans-1      Translation x direction, 1cm
##   vf-trans-2      Translation y direction, 1cm
##   vf-radial-1     Radial warp, 1cm
##   vf-gauss-1      Gaussian warp, 10cm std, 1cm x, 2cm y
## -------------------------------------------------------------------------
plm_add_test (
  "vf-zero"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth-vf;--output;${PLM_BUILD_TESTING_DIR}/vf-zero.mha;--xf-zero;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )
plm_add_test (
  "vf-trans-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth-vf;--output;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha;--xf-trans;10 0 0;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )
plm_add_test (
  "vf-trans-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth-vf;--output;${PLM_BUILD_TESTING_DIR}/vf-trans-2.mha;--xf-trans;0 10 0;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )
plm_add_test (
  "vf-radial-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth-vf;--output;${PLM_BUILD_TESTING_DIR}/vf-radial-1.mha;--xf-radial;--radial-center;0 0 0;--radial-mag;10;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )
plm_add_test (
  "vf-gaussian-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "synth-vf;--output;${PLM_BUILD_TESTING_DIR}/vf-gaussian-1.mha;--xf-gauss;--gauss-center;0 0 0;--gauss-mag;10 20 0;--gauss-std;100 100 100;--dim;${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE} ${SYNTH_MHA_SIZE}"
  )

## -------------------------------------------------------------------------
## Test synthetic vector fields
## -------------------------------------------------------------------------
plm_add_test (
  "vf-zero-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/vf-zero.mha"
  )
plmtest_check_interval ("vf-zero-check"
  "${PLM_BUILD_TESTING_DIR}/vf-zero-stats.stdout.txt"
  "Mean abs: *([-0-9.]*)"
  "0.000"
  "0.000"
  )
set_property (TEST vf-zero-stats APPEND PROPERTY DEPENDS vf-zero)
set_property (TEST vf-zero-check APPEND PROPERTY DEPENDS vf-zero-stats)

plm_add_test (
  "vf-trans-1-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha"
  )
plmtest_check_interval ("vf-trans-1-check"
  "${PLM_BUILD_TESTING_DIR}/vf-trans-1-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "10.000"
  "10.000"
  )
set_property (TEST vf-trans-1-stats APPEND PROPERTY DEPENDS vf-trans-1)
set_property (TEST vf-trans-1-check APPEND PROPERTY DEPENDS vf-trans-1-stats)

plm_add_test (
  "vf-trans-2-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/vf-trans-2.mha"
  )
plmtest_check_interval ("vf-trans-2-check"
  "${PLM_BUILD_TESTING_DIR}/vf-trans-2-stats.stdout.txt"
  "Mean: *[-0-9.]* *([-0-9.]*)"
  "10.000"
  "10.000"
  )
set_property (TEST vf-trans-2-stats APPEND PROPERTY DEPENDS vf-trans-2)
set_property (TEST vf-trans-2-check APPEND PROPERTY DEPENDS vf-trans-2-stats)

plm_add_test (
  "vf-radial-1-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/vf-radial-1.mha"
  )
plmtest_check_interval ("vf-radial-1-check"
  "${PLM_BUILD_TESTING_DIR}/vf-radial-1-stats.stdout.txt"
  "Mean abs: *([-0-9.]*)"
  "5.15"
  "5.16"
  )
set_property (TEST vf-radial-1-stats APPEND PROPERTY DEPENDS vf-radial-1)
set_property (TEST vf-radial-1-check APPEND PROPERTY DEPENDS vf-radial-1-stats)

## -------------------------------------------------------------------------
## bspline - bspline algorithm flavors
## -------------------------------------------------------------------------
plm_add_test (
  "bspline-c"
  ${PLM_PLASTIMATCH_PATH}/bspline
  "-a;steepest;-m;10;-f;c;-x;${PLM_BUILD_TESTING_DIR}/bspline_c_xf.txt;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;${PLM_BUILD_TESTING_DIR}/gauss-2.mha"
  )
plmtest_check_interval (bspline-c-check
  "${PLM_BUILD_TESTING_DIR}/bspline-c.stdout.txt"
  "^\\\\[.*, 10] MSE *([0-9.]*)"
  "1168.25"
  "1168.35"
  )
set_property (TEST bspline-c APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST bspline-c APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (bspline-c-check PROPERTIES DEPENDS bspline-c)

plm_add_test (
  "bspline-g"
  ${PLM_PLASTIMATCH_PATH}/bspline
  "-a;steepest;-m;10;-f;g;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;${PLM_BUILD_TESTING_DIR}/gauss-2.mha"
  )
plmtest_check_interval (bspline-g-check
  "${PLM_BUILD_TESTING_DIR}/bspline-g.stdout.txt"
  "^\\\\[.*, 10] MSE *([0-9.]*)"
  "1168.25"
  "1168.35"
  )
set_property (TEST bspline-g APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST bspline-g APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (bspline-g-check PROPERTIES DEPENDS bspline-g)

plm_add_test (
  "bspline-h"
  ${PLM_PLASTIMATCH_PATH}/bspline
  "-a;steepest;-m;10;-f;h;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;${PLM_BUILD_TESTING_DIR}/gauss-2.mha"
  )
plmtest_check_interval (bspline-h-check
  "${PLM_BUILD_TESTING_DIR}/bspline-h.stdout.txt"
  "^\\\\[.*, 10] MSE *([0-9.]*)"
  "1168.25"
  "1168.35"
  )
set_property (TEST bspline-h APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST bspline-h APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (bspline-h-check PROPERTIES DEPENDS bspline-h)

plm_add_test (
  "bspline-mi-c-1"
  ${PLM_PLASTIMATCH_PATH}/bspline
  "-a;steepest;-m;10;-f;c;-M;mi;-B;20 20;-O;${PLM_BUILD_TESTING_DIR}/bspline-mi-c-1.mha;-x;${PLM_BUILD_TESTING_DIR}/bspline-mi-c-1.txt;-V;${PLM_BUILD_TESTING_DIR}/bspline-mi-c-1-vf.mha;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;${PLM_BUILD_TESTING_DIR}/gauss-3.mha"
  )
plmtest_check_interval (bspline-mi-c-1-check
  "${PLM_BUILD_TESTING_DIR}/bspline-mi-c-1.stdout.txt"
  "^\\\\[.*, 10] *MI *([-0-9.]*)"
  "-0.795"
  "-0.791"
  )
set_property (TEST bspline-mi-c-1 APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST bspline-mi-c-1 APPEND PROPERTY DEPENDS gauss-3)
set_tests_properties (bspline-mi-c-1-check PROPERTIES DEPENDS bspline-mi-c-1)

plm_add_test (
  "bspline-mi-c-2"
  ${PLM_PLASTIMATCH_PATH}/bspline
  "-a;steepest;-m;10;-f;c;-M;mi;-O;${PLM_BUILD_TESTING_DIR}/bspline-mi-c-2.mha;${PLM_BUILD_TESTING_DIR}/rect-1.mha;${PLM_BUILD_TESTING_DIR}/rect-2.mha"
  )
## Without DOUBLE_HIST defined, output is 0.031
## With DOUBLE_HIST defined, output is 0.032
plmtest_check_interval (bspline-mi-c-2-check
  "${PLM_BUILD_TESTING_DIR}/bspline-mi-c-2.stdout.txt"
  "^\\\\[.*, 10] *MI *([-0-9.]*)"
  "-0.035"
  "-0.031"
  )
set_tests_properties (bspline-mi-c-2 PROPERTIES DEPENDS "rect-1;rect-2")
set_tests_properties (bspline-mi-c-2-check PROPERTIES DEPENDS bspline-mi-c-2)

## -------------------------------------------------------------------------
## demons
##   demons-a          gauss-1.mha, gauss-2.mha
##   demons-cuda-a     gauss-1.mha, gauss-2.mha
## *** 2011-10-30
##    Debian ARM gives value of 2.02.  Normally the returned value 
##    is about 2.15, which would be tested for a range of 2.1 to 2.2
##    This has been changed to 1.95 to let ARM test pass.
## -------------------------------------------------------------------------
plm_add_test (
  "demons-a"
  ${PLM_PLASTIMATCH_PATH}/demons
  "-m;50;-s;3;-f;5 5 5;-O;${PLM_BUILD_TESTING_DIR}/demons-a.mha;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;${PLM_BUILD_TESTING_DIR}/gauss-2.mha"
  )
plmtest_check_interval (demons-a-check
  "${PLM_BUILD_TESTING_DIR}/demons-a.stdout.txt"
  "^Mean: *([0-9.]*)"
  "3.4"
  "3.5"
#  "1.95"
#  "2.2"
  )
set_property (TEST demons-a APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST demons-a APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (demons-a-check PROPERTIES DEPENDS demons-a)

plm_add_test (
  "demons-cuda-a"
  ${PLM_PLASTIMATCH_PATH}/demons
  "-A;cuda;-m;50;-s;3;-f;5 5 5;-O;${PLM_BUILD_TESTING_DIR}/demons-cuda-a.mha;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;${PLM_BUILD_TESTING_DIR}/gauss-2.mha"
  )
plmtest_check_interval (demons-cuda-a-check
  "${PLM_BUILD_TESTING_DIR}/demons-cuda-a.stdout.txt"
  "^Mean: *([0-9.]*)"
  "2.1"
  "2.2"
  )
set_property (TEST demons-cuda-a APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST demons-cuda-a APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (demons-cuda-a-check PROPERTIES DEPENDS demons-cuda-a)

## -------------------------------------------------------------------------
## drr
##   drr-a       gauss-1.mha (normal image), exact
##   drr-b       gauss-1.mha (normal image), uniform
##   drr-c       gauss-4.mha (negative spacing), exact
##   drr-d       gauss-4.mha (negative spacing), uniform
##   drr-cuda    gauss-1.mha, uniform
##   drr-opencl  gauss-1.mha, uniform
## -------------------------------------------------------------------------
plm_add_test (
  "drr-a"
  ${PLM_PLASTIMATCH_PATH}/drr
  "-a;20;-O;${PLM_BUILD_TESTING_DIR}/drr-a/out_;${PLM_BUILD_TESTING_DIR}/gauss-1.mha"
  )
plm_add_test (
  "drr-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/drr-a/out_0010.pfm"
  )
plmtest_check_interval ("drr-a-check"
  "${PLM_BUILD_TESTING_DIR}/drr-a-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "0.045"
  "0.055"
  )
set_tests_properties (drr-a PROPERTIES DEPENDS gauss-1)
set_tests_properties (drr-a-stats PROPERTIES DEPENDS drr-a)
set_tests_properties (drr-a-check PROPERTIES DEPENDS drr-a-stats)

plm_add_test (
  "drr-b"
  ${PLM_PLASTIMATCH_PATH}/drr
  "-a;20;-O;${PLM_BUILD_TESTING_DIR}/drr-b/out_;-i;uniform;${PLM_BUILD_TESTING_DIR}/gauss-1.mha"
  )
plm_add_test (
  "drr-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;drr-b/out_0010.pfm"
  )
plmtest_check_interval ("drr-b-check"
  "${PLM_BUILD_TESTING_DIR}/drr-b-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "0.045"
  "0.055"
  )
set_tests_properties (drr-b PROPERTIES DEPENDS gauss-1)
set_tests_properties (drr-b-stats PROPERTIES DEPENDS drr-b)
set_tests_properties (drr-b-check PROPERTIES DEPENDS drr-b-stats)

plm_add_test (
  "drr-c"
  ${PLM_PLASTIMATCH_PATH}/drr
  "-a;20;-O;${PLM_BUILD_TESTING_DIR}/drr-c/out_;${PLM_BUILD_TESTING_DIR}/gauss-4.mha"
  )
plm_add_test (
  "drr-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;drr-c/out_0010.pfm"
  )
plmtest_check_interval ("drr-c-check"
  "${PLM_BUILD_TESTING_DIR}/drr-c-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "0.045"
  "0.055"
  )
set_tests_properties (drr-c PROPERTIES DEPENDS gauss-4)
set_tests_properties (drr-c-stats PROPERTIES DEPENDS drr-c)
set_tests_properties (drr-c-check PROPERTIES DEPENDS drr-c-stats)

plm_add_test (
  "drr-d"
  ${PLM_PLASTIMATCH_PATH}/drr
  "-a;20;-O;${PLM_BUILD_TESTING_DIR}/drr-d/out_;-i;uniform;${PLM_BUILD_TESTING_DIR}/gauss-4.mha"
  )
plm_add_test (
  "drr-d-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;drr-d/out_0010.pfm"
  )
plmtest_check_interval ("drr-d-check"
  "${PLM_BUILD_TESTING_DIR}/drr-d-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "0.045"
  "0.055"
  )
set_tests_properties (drr-d PROPERTIES DEPENDS gauss-1)
set_tests_properties (drr-d-stats PROPERTIES DEPENDS drr-d)
set_tests_properties (drr-d-check PROPERTIES DEPENDS drr-d-stats)

plm_add_test (
  "drr-cuda"
  ${PLM_PLASTIMATCH_PATH}/drr
  "-A;cuda;-a;20;-O;${PLM_BUILD_TESTING_DIR}/drr-cuda/out_;${PLM_BUILD_TESTING_DIR}/gauss-1.mha"
  )
plm_add_test (
  "drr-cuda-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/drr-cuda/out_0010.pfm"
  )
plmtest_check_interval ("drr-cuda-check"
  "${PLM_BUILD_TESTING_DIR}/drr-cuda-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "0.045"
  "0.055"
  )
set_tests_properties (drr-cuda PROPERTIES DEPENDS gauss-1)
set_tests_properties (drr-cuda-stats PROPERTIES DEPENDS drr-cuda)
set_tests_properties (drr-cuda-check PROPERTIES DEPENDS drr-cuda-stats)

plm_add_test (
  "drr-opencl"
  ${PLM_PLASTIMATCH_PATH}/drr
  "-A;opencl;-a;20;-O;${PLM_BUILD_TESTING_DIR}/drr-opencl/out_;${PLM_BUILD_TESTING_DIR}/gauss-1.mha"
  )
plm_add_test (
  "drr-opencl-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/drr-opencl/out_0010.pfm"
  )
plmtest_check_interval ("drr-opencl-check"
  "${PLM_BUILD_TESTING_DIR}/drr-opencl-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "0.045"
  "0.055"
  )
set_tests_properties (drr-opencl PROPERTIES DEPENDS gauss-1)
set_tests_properties (drr-opencl-stats PROPERTIES DEPENDS drr-opencl)
set_tests_properties (drr-opencl-check PROPERTIES DEPENDS drr-opencl-stats)

## -------------------------------------------------------------------------
## fdk - all tests using fdk filtered backprojection
##   fdk-cpu-a     CPU reconstruction of synthetic data, flavor a
##   fdk-cpu-b     CPU reconstruction of Varian data, no filtering
##   fdk-cpu-c     CPU reconstruction of Varian data, fftw filtering
##   fdk-cpu-d     CPU reconstruction of synthetic data, flavor 0
##   fdk-cuda-a    GPU reconstruction of synthetic data using CUDA
##    [fdk-cuda-b  GPU reconstruction of Varian data using CUDA
##   fdk-opencl-a  GPU reconstruction of synthetic data using OpenCL
## -------------------------------------------------------------------------
plm_add_test (
  "fdk-cpu-a"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-I;${PLM_BUILD_TESTING_DIR}/drr-a;-f;none;-a;0 19;-O;${PLM_BUILD_TESTING_DIR}/fdk-cpu-a.mha"
  )
plm_add_test (
  "fdk-cpu-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-cpu-a.mha"
  )
plmtest_check_interval ("fdk-cpu-a-check"
  "${PLM_BUILD_TESTING_DIR}/fdk-cpu-a-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "-987.5"
  "-986.5"
  )
set_tests_properties (fdk-cpu-a PROPERTIES DEPENDS drr-a)
set_tests_properties (fdk-cpu-a-stats PROPERTIES DEPENDS fdk-cpu-a)
set_tests_properties (fdk-cpu-a-check PROPERTIES DEPENDS fdk-cpu-a-stats)

plm_add_test (
  "fdk-cpu-b"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-I;${PLM_BUILD_TESTING_DIR}/varian-catphan;-f;none;-a;0 20 670;-O;${PLM_BUILD_TESTING_DIR}/fdk-cpu-b.mha"
  )
plm_add_test (
  "fdk-cpu-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-cpu-b.mha"
  )
plmtest_check_interval ("fdk-cpu-b-check"
  "${PLM_BUILD_TESTING_DIR}/fdk-cpu-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-708.1"
  "-707.5"
  )
set_tests_properties (fdk-cpu-b-stats PROPERTIES DEPENDS fdk-cpu-b)
set_tests_properties (fdk-cpu-b-check PROPERTIES DEPENDS fdk-cpu-b-stats)

plm_add_test (
  "fdk-cpu-c"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-I;${PLM_BUILD_TESTING_DIR}/varian-catphan;-f;ramp;-a;0 50 670;-O;${PLM_BUILD_TESTING_DIR}/fdk-cpu-c.mha"
  )
plm_add_test (
  "fdk-cpu-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-cpu-c.mha"
  )
plmtest_check_interval ("fdk-cpu-c-check"
  "${PLM_BUILD_TESTING_DIR}/fdk-cpu-c-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-982.3"
  "-981.5"
  )
set_tests_properties (fdk-cpu-c-stats PROPERTIES DEPENDS fdk-cpu-c)
set_tests_properties (fdk-cpu-c-check PROPERTIES DEPENDS fdk-cpu-c-stats)

plm_add_test (
  "fdk-cpu-d"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-I;${PLM_BUILD_TESTING_DIR}/drr-a;-X;0;-f;none;-a;0 19;-O;${PLM_BUILD_TESTING_DIR}/fdk-cpu-d.mha"
  )
plm_add_test (
  "fdk-cpu-d-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-cpu-d.mha"
  )
plmtest_check_interval ("fdk-cpu-d-check"
  "${PLM_BUILD_TESTING_DIR}/fdk-cpu-d-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "-987.5"
  "-986.5"
  )
set_tests_properties (fdk-cpu-d PROPERTIES DEPENDS drr-a)
set_tests_properties (fdk-cpu-d-stats PROPERTIES DEPENDS fdk-cpu-d)
set_tests_properties (fdk-cpu-d-check PROPERTIES DEPENDS fdk-cpu-d-stats)

plm_add_test (
  "fdk-cuda-a"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-I;${PLM_BUILD_TESTING_DIR}/drr-a;-f;none;-A;cuda;-a;0 19;-O;${PLM_BUILD_TESTING_DIR}/fdk-cuda-a.mha"
  )
plm_add_test (
  "fdk-cuda-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-cuda-a.mha"
  )
plmtest_check_interval ("fdk-cuda-a-check"
  "${PLM_BUILD_TESTING_DIR}/fdk-cuda-a-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "-987.5"
  "-986.5"
  )
set_tests_properties (fdk-cuda-a PROPERTIES DEPENDS drr-a)
set_tests_properties (fdk-cuda-a-stats PROPERTIES DEPENDS fdk-cuda-a)
set_tests_properties (fdk-cuda-a-check PROPERTIES DEPENDS fdk-cuda-a-stats)

plm_add_test (
  "fdk-opencl-a"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-I;${PLM_BUILD_TESTING_DIR}/drr-a;-f;none;-A;opencl;-a;0 19;-O;${PLM_BUILD_TESTING_DIR}/fdk-opencl-a.mha"
  )
plm_add_test (
  "fdk-opencl-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-opencl-a.mha"
  )
plmtest_check_interval ("fdk-opencl-a-check"
  "${PLM_BUILD_TESTING_DIR}/fdk-opencl-a-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "-987.5"
  "-986.5"
  )
set_tests_properties (fdk-opencl-a PROPERTIES DEPENDS drr-a)
set_tests_properties (fdk-opencl-a-stats PROPERTIES DEPENDS fdk-opencl-a)
set_tests_properties (fdk-opencl-a-check PROPERTIES DEPENDS fdk-opencl-a-stats)

## -------------------------------------------------------------------------
## fdk_tutorial - run the fdk tutorial
## -------------------------------------------------------------------------
plm_add_test (
  "fdk-tutorial-a"
  ${PLM_PLASTIMATCH_PATH}/drr
  "-t;pfm;-a;60;-N;3;-g;1000 1500;-r;100 100;-z;300 300;-I;${PLM_BUILD_TESTING_DIR}/headphantom/headphantom.mha;-O;${PLM_BUILD_TESTING_DIR}/fdk-tutorial-a/"
  )
plm_add_test (
  "fdk-tutorial-b"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-f;none;-r;100 100 100;-I;${PLM_BUILD_TESTING_DIR}/fdk-tutorial-a;-O;${PLM_BUILD_TESTING_DIR}/fdk-tutorial-b.mha"
  )
plm_add_test (
  "fdk-tutorial-b-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-tutorial-b.mha"
  )
plmtest_check_interval ("fdk-tutorial-b-check-1"
  "${PLM_BUILD_TESTING_DIR}/fdk-tutorial-b-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-955.8"
  "-954.8"
  )
plm_add_test (
  "fdk-tutorial-c"
  ${PLM_PLASTIMATCH_PATH}/fdk
  "-f;ramp;-r;100 100 100;-I;${PLM_BUILD_TESTING_DIR}/fdk-tutorial-a;-O;${PLM_BUILD_TESTING_DIR}/fdk-tutorial-c.mha"
  )
plm_add_test (
  "fdk-tutorial-c-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/fdk-tutorial-c.mha"
  )
plmtest_check_interval ("fdk-tutorial-c-check-1"
  "${PLM_BUILD_TESTING_DIR}/fdk-tutorial-c-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-999.0"
  "-998.0"
  )
set_tests_properties (fdk-tutorial-b PROPERTIES DEPENDS fdk-tutorial-a)
set_tests_properties (fdk-tutorial-b-stats-1 PROPERTIES DEPENDS fdk-tutorial-b)
set_tests_properties (fdk-tutorial-b-check-1 PROPERTIES 
  DEPENDS fdk-tutorial-b-stats-1)
set_tests_properties (fdk-tutorial-c PROPERTIES DEPENDS fdk-tutorial-a)
set_tests_properties (fdk-tutorial-c-stats-1 PROPERTIES DEPENDS fdk-tutorial-c)
set_tests_properties (fdk-tutorial-c-check-1 PROPERTIES 
  DEPENDS fdk-tutorial-c-stats-1)

## -------------------------------------------------------------------------
## landmark_warp a: itk tps, slicer fcsv
## landmark_warp b: wendland rbf, slicer fcsv
## landmark_warp c: gauss rbf, slicer fcsv
## landmark_warp d: itk tps, slicer fcsv, -L option
## -------------------------------------------------------------------------
plm_add_test (
  "landmark-warp-a" 
  ${PLM_PLASTIMATCH_PATH}/landmark_warp
  "-a;tps;-f;${PLM_TESTING_DATA_DIR}/fiducials-rect-2.fcsv;-m;${PLM_TESTING_DATA_DIR}/fiducials-rect-3.fcsv;-I;${PLM_BUILD_TESTING_DIR}/rect-3.mha;-O;${PLM_BUILD_TESTING_DIR}/landmark-warp-a-img.mha;-V;${PLM_BUILD_TESTING_DIR}/landmark-warp-a-vf.mha"
  )
plm_add_test (
  "landmark-warp-a-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/landmark-warp-a-img.mha"
  )
plmtest_check_interval ("landmark-warp-a-check-1"
  "${PLM_BUILD_TESTING_DIR}/landmark-warp-a-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-992.0"
  "-991.5"
  )
plm_add_test (
  "landmark-warp-a-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/landmark-warp-a-vf.mha"
  )
plmtest_check_interval ("landmark-warp-a-check-2"
  "${PLM_BUILD_TESTING_DIR}/landmark-warp-a-stats-2.stdout.txt"
  "Mean: *([-0-9.]*)"
  "-23.2"
  "-22.7"
  )
set_tests_properties (landmark-warp-a PROPERTIES DEPENDS rect-3)
set_tests_properties (landmark-warp-a-stats-1 PROPERTIES 
  DEPENDS landmark-warp-a)
set_tests_properties (landmark-warp-a-check-1 PROPERTIES 
  DEPENDS landmark-warp-a-stats-1)
set_tests_properties (landmark-warp-a-stats-2 PROPERTIES 
  DEPENDS landmark-warp-a)
set_tests_properties (landmark-warp-a-check-2 PROPERTIES 
  DEPENDS landmark-warp-a-stats-2)

plm_add_test (
  "landmark-warp-b" 
  ${PLM_PLASTIMATCH_PATH}/landmark_warp
  "-a;wendland;-f;${PLM_TESTING_DATA_DIR}/fiducials-rect-2.fcsv;-m;${PLM_TESTING_DATA_DIR}/fiducials-rect-3.fcsv;-I;${PLM_BUILD_TESTING_DIR}/rect-3.mha;-O;${PLM_BUILD_TESTING_DIR}/landmark-warp-b-img.mha;-V;${PLM_BUILD_TESTING_DIR}/landmark-warp-b-vf.mha;-r;90;-d;0;-Y;0.0"
  )
plm_add_test (
  "landmark-warp-b-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/landmark-warp-b-img.mha"
  )
plmtest_check_interval ("landmark-warp-b-check-1"
  "${PLM_BUILD_TESTING_DIR}/landmark-warp-b-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-992.2"
  "-991.8"
  )
plm_add_test (
  "landmark-warp-b-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/landmark-warp-b-vf.mha"
  )
plmtest_check_interval ("landmark-warp-b-check-2"
  "${PLM_BUILD_TESTING_DIR}/landmark-warp-b-stats-2.stdout.txt"
  "Mean: *([-0-9.]*)"
  "-1.55"
  "-1.4"
  )
set_tests_properties (landmark-warp-b PROPERTIES DEPENDS rect-3)
set_tests_properties (landmark-warp-b-stats-1 PROPERTIES 
  DEPENDS landmark-warp-b)
set_tests_properties (landmark-warp-b-check-1 PROPERTIES 
  DEPENDS landmark-warp-b-stats-1)
set_tests_properties (landmark-warp-b-stats-2 PROPERTIES 
  DEPENDS landmark-warp-b)
set_tests_properties (landmark-warp-b-check-2 PROPERTIES 
  DEPENDS landmark-warp-b-stats-2)

plm_add_test (
  "landmark-warp-c" 
  ${PLM_PLASTIMATCH_PATH}/landmark_warp
  "-a;gauss;-f;${PLM_TESTING_DATA_DIR}/fiducials-rect-2.fcsv;-m;${PLM_TESTING_DATA_DIR}/fiducials-rect-3.fcsv;-I;${PLM_BUILD_TESTING_DIR}/rect-3.mha;-O;${PLM_BUILD_TESTING_DIR}/landmark-warp-c-img.mha;-V;${PLM_BUILD_TESTING_DIR}/landmark-warp-c-vf.mha;-r;74;-d;0;-Y;1.0"
  )
plm_add_test (
  "landmark-warp-c-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/landmark-warp-c-img.mha"
  )
plmtest_check_interval ("landmark-warp-c-check-1"
  "${PLM_BUILD_TESTING_DIR}/landmark-warp-c-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-992.0"
  "-991.5"
  )
plm_add_test (
  "landmark-warp-c-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/landmark-warp-c-vf.mha"
  )
plmtest_check_interval ("landmark-warp-c-check-2"
  "${PLM_BUILD_TESTING_DIR}/landmark-warp-c-stats-2.stdout.txt"
  "Mean: *([-0-9.]*)"
  "-1.6"
  "-1.5"
  )
set_tests_properties (landmark-warp-c PROPERTIES DEPENDS rect-3)
set_tests_properties (landmark-warp-c-stats-1 PROPERTIES 
  DEPENDS landmark-warp-c)
set_tests_properties (landmark-warp-c-check-1 PROPERTIES 
  DEPENDS landmark-warp-c-stats-1)
set_tests_properties (landmark-warp-c-stats-2 PROPERTIES 
  DEPENDS landmark-warp-c)
set_tests_properties (landmark-warp-c-check-2 PROPERTIES 
  DEPENDS landmark-warp-c-stats-2)

plm_add_test (
  "landmark-warp-d" 
  ${PLM_PLASTIMATCH_PATH}/landmark_warp
  "-a;tps;-f;${PLM_TESTING_DATA_DIR}/fiducials-rect-2.fcsv;-m;${PLM_TESTING_DATA_DIR}/fiducials-rect-3.fcsv;-I;${PLM_BUILD_TESTING_DIR}/rect-3.mha;-O;${PLM_BUILD_TESTING_DIR}/landmark-warp-d-img.mha;-V;${PLM_BUILD_TESTING_DIR}/landmark-warp-d-vf.mha;-L;${PLM_BUILD_TESTING_DIR}/landmark-warp-d.fcsv"
  )
set_tests_properties (landmark-warp-d PROPERTIES DEPENDS rect-3)

## -------------------------------------------------------------------------
## plastimatch add, plastimatch average
##  plm-add-a      Add two images
##  plm-add-b      Average two images
##  plm-add-vf-a   Add two vector fields
##  plm-add-vf-b   Average two vector fields
## -------------------------------------------------------------------------
plm_add_test (
  "plm-add-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "add;--output;${PLM_BUILD_TESTING_DIR}/plm-add-a.nrrd;${PLM_BUILD_TESTING_DIR}/rect-1.mha;${PLM_BUILD_TESTING_DIR}/rect-3.mha"
  )
plm_add_test (
  "plm-add-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-add-a.nrrd"
  )
plmtest_check_interval ("plm-add-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-add-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-1000.1"
  "-999.9"
  )
set_tests_properties (plm-add-a PROPERTIES DEPENDS "rect-1;rect-3")
set_tests_properties (plm-add-a-stats PROPERTIES DEPENDS plm-add-a)
set_tests_properties (plm-add-a-check PROPERTIES DEPENDS plm-add-a-stats)

plm_add_test (
  "plm-add-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "average;--output;${PLM_BUILD_TESTING_DIR}/plm-add-b.nrrd;${PLM_BUILD_TESTING_DIR}/rect-1.mha;${PLM_BUILD_TESTING_DIR}/rect-3.mha"
  )
plm_add_test (
  "plm-add-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-add-b.nrrd"
  )
plmtest_check_interval ("plm-add-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-add-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-500.1"
  "-499.9"
  )
set_tests_properties (plm-add-b PROPERTIES DEPENDS "rect-1;rect-3")
set_tests_properties (plm-add-b-stats PROPERTIES DEPENDS plm-add-b)
set_tests_properties (plm-add-b-check PROPERTIES DEPENDS plm-add-b-stats)


plm_add_test (
  "plm-add-vf-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "add;--output;${PLM_BUILD_TESTING_DIR}/plm-add-vf-a.nrrd;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha"
  )
plm_add_test (
  "plm-add-vf-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-add-vf-a.nrrd"
  )
plmtest_check_interval ("plm-add-vf-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-add-vf-a-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "20.000"
  "20.000"
  )
set_tests_properties (plm-add-vf-a PROPERTIES DEPENDS vf-trans-1)
set_tests_properties (plm-add-vf-a-stats PROPERTIES DEPENDS plm-add-vf-a)
set_tests_properties (plm-add-vf-a-check PROPERTIES DEPENDS plm-add-vf-a-stats)

plm_add_test (
  "plm-add-vf-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "average;--output;${PLM_BUILD_TESTING_DIR}/plm-add-vf-b.nrrd;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha;${PLM_BUILD_TESTING_DIR}/vf-trans-2.mha"
  )
plm_add_test (
  "plm-add-vf-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-add-vf-b.nrrd"
  )
plmtest_check_interval ("plm-add-vf-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-add-vf-b-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "5.000"
  "5.000"
  )
set_tests_properties (plm-add-vf-b PROPERTIES DEPENDS "vf-trans-1;vf-trans-2")
set_tests_properties (plm-add-vf-b-stats PROPERTIES DEPENDS plm-add-vf-b)
set_tests_properties (plm-add-vf-b-check PROPERTIES DEPENDS plm-add-vf-b-stats)

## -------------------------------------------------------------------------
## plastimatch boundary
##  plm-boundary-a:   sphere
## -------------------------------------------------------------------------
plm_add_test (
  "plm-boundary-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "boundary;--output;${PLM_BUILD_TESTING_DIR}/plm-boundary-a.nrrd;${PLM_BUILD_TESTING_DIR}/sphere-1.mha"
  )
plm_add_test (
  "plm-boundary-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-boundary-a.nrrd"
  )
plmtest_check_interval ("plm-boundary-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-boundary-a-stats.stdout.txt"
  "NONZERO *([-0-9.]*)"
  "546"
  "546"
  )
set_tests_properties (plm-boundary-a PROPERTIES DEPENDS sphere-1)
set_tests_properties (plm-boundary-a-stats PROPERTIES DEPENDS plm-boundary-a)
set_tests_properties (plm-boundary-a-check 
  PROPERTIES DEPENDS plm-boundary-a-stats)

## -------------------------------------------------------------------------
## plastimatch convert dicom (and dicom rt)
##   plm convert dicom a: (rect) input rtss, no fixed image, no ct reference
##   plm convert dicom b: set metadata patient position to FFS
##   plm convert dicom c: make prefix images from lung-1
##   plm convert dicom d: error testing on non-existant directory
##   plm convert dicom e: error testing on directory without dicom files
##   plm convert dicom f: dicom rtss -> dicom rtss, with referenced ct
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/rect-1-dicom/rtss.dcm;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-a-ss.mha;--output-ss-list;plm-convert-dicom-a-ss.txt;--output-cxt;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-a.cxt;--output-prefix;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-a-prefix"
  )
plm_add_test (
  "plm-convert-dicom-a-stats" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-a-ss.mha"
  )
plmtest_check_interval (
  "plm-convert-dicom-a-check" 
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-a-stats.stdout.txt"
  " 0 *NVOX *([-0-9.]*)"
  "1648003"
  "1648003"
  )
set_tests_properties (plm-convert-dicom-a PROPERTIES DEPENDS rect-1)
set_tests_properties (plm-convert-dicom-a-stats PROPERTIES 
  DEPENDS plm-convert-dicom-a)
set_tests_properties (plm-convert-dicom-a-check PROPERTIES 
  DEPENDS plm-convert-dicom-a-stats)

plm_add_test (
  "plm-convert-dicom-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--dicom-with-uids;false;--input;${PLM_BUILD_TESTING_DIR}/rect-1-dicom;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-b-dicom;--metadata;0018,5100=FFS"
  )
plm_add_test (
  "plm-convert-dicom-b-stats" 
  ${PLM_PLASTIMATCH_PATH}/dicom_info
  "--input;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-b-dicom/image0000.dcm"
  )
plmtest_check_string ("plm-convert-dicom-b-check" 
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-b-stats.stdout.txt"
  "Patient Position . (.*[^ ]) *$"
  "FFS"
  )
set_tests_properties (plm-convert-dicom-b PROPERTIES DEPENDS rect-1)
set_tests_properties (plm-convert-dicom-b-stats PROPERTIES 
  DEPENDS plm-convert-dicom-b)
set_tests_properties (plm-convert-dicom-b-check PROPERTIES 
  DEPENDS plm-convert-dicom-b-stats)

plm_add_test (
  "plm-convert-dicom-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/lung-1-dicom/;--output-prefix;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-c-prefix"
  )
plm_add_test (
  "plm-convert-dicom-c-stats" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-c-prefix/Tumor.mha"
  )
plmtest_check_interval (
  "plm-convert-dicom-c-check" 
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-c-stats.stdout.txt"
  "NONZERO *([-0-9.]*)"
  "280"
  "280"
  )
set_tests_properties (plm-convert-dicom-c PROPERTIES DEPENDS lung-1)
set_tests_properties (plm-convert-dicom-c-stats PROPERTIES 
  DEPENDS plm-convert-dicom-c)
set_tests_properties (plm-convert-dicom-c-check PROPERTIES 
  DEPENDS plm-convert-dicom-c-stats)

plm_add_test (
  "plm-convert-dicom-d" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/non-existant-directory/;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-d.nrrd"
  "-DEXPECTED_ERRNO=1"
  )

plm_add_test (
  "plm-convert-dicom-e" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/tmp/;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-e.nrrd"
  "-DEXPECTED_ERRNO=1"
  )

plm_add_test (
  "plm-convert-dicom-f" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/rect-1-dicom/rtss.dcm;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-f-dicom;--referenced-ct;${PLM_BUILD_TESTING_DIR}/rect-2-dicom"
  )
## GCS FIX: Normally, I would want to probe the dicom data to see if they 
## match.  But dicom_info only works with images, not rtss.
set_tests_properties (plm-convert-dicom-f PROPERTIES DEPENDS "rect-1;rect2")

## -------------------------------------------------------------------------
## These tests are designed to check the handling of overlapping structures 
## in DICOM-RT format.  The tests assume that the original synthetic donut 
## DICOM-RT files are generated without keyholing.
##   plastimatch-convert-dicom-donut-a   From DICOM to ss-img (or semantics)
##   plastimatch-convert-dicom-donut-b   From DICOM to ss-img (xor semantics)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-donut-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/donut-1-dicom;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-donut-a-ss.nrrd"
  )
plm_add_test (
  "plm-convert-dicom-donut-a-stats" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-donut-a-ss.nrrd"
  )
plmtest_check_interval (
  "plm-convert-dicom-donut-a-check" 
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-donut-a-stats.stdout.txt"
  "S *0 *NVOX *([-0-9.]*)"
  "2623"
  "2623"
  )
plm_add_test (
  "plm-convert-dicom-donut-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/donut-1-dicom;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-donut-b-ss.nrrd;--xor-contours"
  )
plm_add_test (
  "plm-convert-dicom-donut-b-stats" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-donut-b-ss.nrrd"
  )
plmtest_check_interval (
  "plm-convert-dicom-donut-b-check" 
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-donut-b-stats.stdout.txt"
  "S *0 *NVOX *([-0-9.]*)"
  "1924"
  "1924"
  )
set_tests_properties (plm-convert-dicom-donut-a PROPERTIES DEPENDS donut-1)
set_tests_properties (plm-convert-dicom-donut-a-stats PROPERTIES 
  DEPENDS plm-convert-dicom-donut-a)
set_tests_properties (plm-convert-dicom-donut-a-check PROPERTIES 
  DEPENDS plm-convert-dicom-donut-a-stats)
set_tests_properties (plm-convert-dicom-donut-b PROPERTIES DEPENDS donut-1)
set_tests_properties (plm-convert-dicom-donut-b-stats PROPERTIES 
  DEPENDS plm-convert-dicom-donut-b)
set_tests_properties (plm-convert-dicom-donut-b-check PROPERTIES 
  DEPENDS plm-convert-dicom-donut-b-stats)

## -------------------------------------------------------------------------
## plastimatch convert rtog (corvus)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-rtog-corvus-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/rtog-corvus-6.2.2;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-rtog-corvus-a.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-rtog-corvus-a-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-rtog-corvus-a-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-rtog-corvus-a-dose.mha"
  )

## -------------------------------------------------------------------------
## plastimatch convert dicom-rt (aw)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-aw-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--dicom-with-uids;false;--input;${PLM_BUILD_TESTING_DIR}/foot-dicomrt-aw-4.4/20101202HFS;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-a.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-a-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-a-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-a-dose.mha;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-a-dicom"
  )
plm_add_test (
  "plm-convert-dicom-aw-a-stats" 
  ${PLM_PLASTIMATCH_PATH}/dicom_info
  "--input;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-a-dicom/image0000.dcm"
  )
plmtest_check_string ("plm-convert-dicom-aw-a-check" 
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-a-stats.stdout.txt"
  "^Patient's Name . (.*[^ ]) *$"
  "PHANTOM^HFS"
  )
plm_add_test (
  "plm-convert-dicom-aw-b" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/foot-dicomrt-aw-4.4/20101202FFS;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-b.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-b-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-b-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-aw-b-dose.mha"
  )
set_tests_properties (plm-convert-dicom-aw-a-stats PROPERTIES 
  DEPENDS plm-convert-dicom-aw-a)
set_tests_properties (plm-convert-dicom-aw-a-check PROPERTIES 
  DEPENDS plm-convert-dicom-aw-a-stats)

## -------------------------------------------------------------------------
## plastimatch convert dicom-rt (cerr)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-cerr-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-CERR4pt0beta2_25_Jan_2011;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-cerr-a.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-cerr-a-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-cerr-a-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-cerr-a-dose.mha"
  )

## -------------------------------------------------------------------------
## plastimatch convert dicom-rt (corvus)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-corvus-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/dicomrt-corvus-6.2.2;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-corvus-a.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-corvus-a-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-corvus-a-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-corvus-a-dose.mha"
  )

## -------------------------------------------------------------------------
## plastimatch convert dicom-rt (pinnacle)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-pinnacle-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/rando-dicomrt-pinnacle3-8.2g;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-pinnacle-a.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-pinnacle-a-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-pinnacle-a-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-pinnacle-a-dose.mha"
  )

## -------------------------------------------------------------------------
## plastimatch-convert-dicom-xio (xio)
##   plm-convert-dicom-xio-a: ct only
##   plm-convert-dicom-xio-b: rtss -> ss_img, with referenced-ct
##   plm-convert-dicom-xio-c: rtss -> ss_img, with fixed image reference
##   plm-convert-dicom-xio-d: dose only, native sampling rate
##   plm-convert-dicom-xio-e: input xio dicom, output plm dicom
##   plm-convert-dicom-xio-f: input plm dicom, output plm dicom
##   plm-convert-dicom-xio-g: all inputs, image outputs
##   plm-warp-dicom-xio-a:    all inputs, bspline transform, all outputs
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-xio-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-a.mha"
  )
plm_add_test (
  "plm-convert-dicom-xio-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-a.mha"
  )
plmtest_check_interval ("plm-convert-dicom-xio-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-667.5"
  "-666.6"
  )
set_tests_properties (plm-convert-dicom-xio-a-stats PROPERTIES 
  DEPENDS plm-convert-dicom-xio-a)
set_tests_properties (plm-convert-dicom-xio-a-check PROPERTIES 
  DEPENDS plm-convert-dicom-xio-a-stats)

plm_add_test (
  "plm-convert-dicom-xio-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02/SS.rtp1.12.20080627A.5.dcm;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-b-ss.mha;--referenced-ct;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02"
  )
plm_add_test (
  "plm-convert-dicom-xio-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-b-ss.mha"
  )
plmtest_check_interval ("plm-convert-dicom-xio-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-b-stats.stdout.txt"
  "S *1 *NVOX *([-0-9.]*)"
  "62767"
  "62767"
  )
set_tests_properties (plm-convert-dicom-xio-b-stats PROPERTIES 
  DEPENDS plm-convert-dicom-xio-b)
set_tests_properties (plm-convert-dicom-xio-b-check PROPERTIES 
  DEPENDS plm-convert-dicom-xio-b-stats)

plm_add_test (
  "plm-convert-dicom-xio-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02/SS.rtp1.12.20080627A.5.dcm;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-c-ss.mha;--fixed;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-a.mha;--output-cxt;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-c.cxt"
  )
plm_add_test (
  "plm-convert-dicom-xio-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-c-ss.mha"
  )
# To be resolved.  Why do dicom-xio-b and dicom-xio-c give different answers?
plmtest_check_interval ("plm-convert-dicom-xio-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-c-stats.stdout.txt"
  "S *1 *NVOX *([-0-9.]*)"
  "62769"
  "62769"
  )
set_tests_properties (plm-convert-dicom-xio-c PROPERTIES 
  DEPENDS plm-convert-dicom-xio-a)
set_tests_properties (plm-convert-dicom-xio-c-stats PROPERTIES 
  DEPENDS plm-convert-dicom-xio-c)
set_tests_properties (plm-convert-dicom-xio-c-check PROPERTIES 
  DEPENDS plm-convert-dicom-xio-c-stats)

plm_add_test (
  "plm-convert-dicom-xio-d"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02/DOSE.20080627A.TRAINING4FLD.dcm;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-d-dose-img.mha"
  )
plm_add_test (
  "plm-convert-dicom-xio-d-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-d-dose-img.mha"
  )
plmtest_check_interval ("plm-convert-dicom-xio-d-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-d-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "42.1"
  "42.2"
  )
set_tests_properties (plm-convert-dicom-xio-d-stats PROPERTIES 
  DEPENDS plm-convert-dicom-xio-d)
set_tests_properties (plm-convert-dicom-xio-d-check PROPERTIES 
  DEPENDS plm-convert-dicom-xio-d-stats)

plm_add_test (
  "plm-convert-dicom-xio-e" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-e-dicom;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-e-dose-img.mha"
  )

plm_add_test (
  "plm-convert-dicom-xio-f" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-e-dicom;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-f-dicom;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-f-dose-img.mha"
  )
plm_add_test (
  "plm-convert-dicom-xio-f-stats" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "compare;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-e-dose-img.mha;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-f-dose-img.mha"
  )
plmtest_check_interval ("plm-convert-dicom-xio-f-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-f-stats.stdout.txt"
  "MAE *([-0-9.]*)"
  "0.000"
  "0.005"
  )
set_tests_properties (plm-convert-dicom-xio-f PROPERTIES 
  DEPENDS plm-convert-dicom-xio-e)
set_tests_properties (plm-convert-dicom-xio-f-stats PROPERTIES 
  DEPENDS plm-convert-dicom-xio-f)
set_tests_properties (plm-convert-dicom-xio-f-check PROPERTIES 
  DEPENDS plm-convert-dicom-xio-f-stats)

plm_add_test (
  "plm-convert-dicom-xio-g"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-g.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-g-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-g-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-xio-g-dose.mha"
  )

plm_add_test (
  "plm-warp-dicom-xio-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_TESTING_DATA_DIR}/xf-bspline-chest-phantom.txt;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-dicomrt-xio-4.33.02;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-dicom;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-ss.nrrd;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-ss.txt;--output-labelmap;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-labelmap.nrrd;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-img.nrrd;--output-prefix;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a;--output-cxt;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a.cxt;--default-value;0"
  )
plm_add_test (
  "plm-warp-dicom-xio-a-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-img.nrrd"
  )
plmtest_check_interval ("plm-warp-dicom-xio-a-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-661.9"
  "-661.4"
  )
plm_add_test (
  "plm-warp-dicom-xio-a-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-ss.nrrd"
  )
plmtest_check_interval ("plm-warp-dicom-xio-a-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-dicom-xio-a-stats-2.stdout.txt"
  "S *1 *NVOX *([-0-9.]*)"
  "1971"
  "1971"
  )
set_tests_properties (plm-warp-dicom-xio-a-stats-1 
  PROPERTIES DEPENDS plm-warp-dicom-xio-a)
set_tests_properties (plm-warp-dicom-xio-a-check-1 
  PROPERTIES DEPENDS plm-warp-dicom-xio-a-stats-1)
set_tests_properties (plm-warp-dicom-xio-a-stats-2 
  PROPERTIES DEPENDS plm-warp-dicom-xio-a)
set_tests_properties (plm-warp-dicom-xio-a-check-2 
  PROPERTIES DEPENDS plm-warp-dicom-xio-a-stats-2)


## -------------------------------------------------------------------------
## plastimatch-convert-dicom-irregular (xio, irregular slice spacing)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-irregular-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/xio-4.60-irregular-spacing;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-irregular-a.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-irregular-a-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-irregular-a-ss.txt;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-irregular-a-dose.mha"
  )

## -------------------------------------------------------------------------
## plm-convert-dicom-33 (mim, 33 structures)
##  plm-convert-dicom-33-a (native method, depends on configuration)
##  plm-convert-dicom-33-b (force unsigned long)
##  plm-convert-dicom-33-c (force uchar_vec)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dicom-33-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/dicomrt-33-structures;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a.mha;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a-ss.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a-ss.txt;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a-dicom"
  )
plm_add_test (
  "plm-convert-dicom-33-a-stats-1" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch 
  "header;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a-ss.mha"
  )
plm_add_test (
  "plm-convert-dicom-33-a-stats-2" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch 
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a-ss.mha"
  )
plmtest_check_string ("plm-convert-dicom-33-a-check-1" 
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a-stats-1.stdout.txt"
  "^Planes . (.*[^ ]) *$"
  "5"
  )
plmtest_check_interval ("plm-convert-dicom-33-a-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-33-a-stats-2.stdout.txt"
  "S *28 *NVOX *([-0-9.]*)"
  "130"
  "130"
  )
set_tests_properties (plm-convert-dicom-33-a-stats-1 PROPERTIES 
  DEPENDS plm-convert-dicom-33-a)
set_tests_properties (plm-convert-dicom-33-a-stats-2 PROPERTIES 
  DEPENDS plm-convert-dicom-33-a)
set_tests_properties (plm-convert-dicom-33-a-check-1 PROPERTIES 
  DEPENDS plm-convert-dicom-33-a-stats-1)
set_tests_properties (plm-convert-dicom-33-a-check-2 PROPERTIES 
  DEPENDS plm-convert-dicom-33-a-stats-2)

## -------------------------------------------------------------------------
## plastimatch-convert-cxt
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-cxt"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/plm-convert-dicom-a.cxt;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-convert-cxt.mha;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-convert-cxt.txt"
  )
plm_add_test (
  "plm-convert-cxt-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-cxt.mha"
  )
# There is a bug in the dicom converter, whereby the image in the 
# same directory is not parsed to find rasterization size.  This 
# test should be modified when that bug is fixed.
plmtest_check_interval ("plm-convert-cxt-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-cxt-stats.stdout.txt"
  "S *0 *NVOX *([-0-9.]*)"
  "1648003"
  "1648003"
  )
set_tests_properties (plm-convert-cxt PROPERTIES DEPENDS plm-convert-dicom-a)
set_tests_properties (plm-convert-cxt-stats PROPERTIES DEPENDS plm-convert-cxt)
set_tests_properties (plm-convert-cxt-check PROPERTIES 
  DEPENDS plm-convert-cxt-stats)

## -------------------------------------------------------------------------
## plm-convert-xio
##  plm-convert-xio-a
##  plm-convert-xio-b
##    (2014-10-13) These used to convert to AVG between -667.5 to -666.6, 
##    but now the AVG is -665.638611.  Would be good to test with phantom
##    to make sure the conversion is good.
##  plm-warp-xio-a
##    (2014-10-13) Ditto. AVG was between -661.5 to -661.4, but now 
##    is -660.018921
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-xio-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-xio-4.33.02;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-a.mha;--output-xio;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-a-xio-output;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-a-dicom;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-a-dose.mha"
  )
plm_add_test (
  "plm-convert-xio-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-a.mha"
  )
plmtest_check_interval ("plm-convert-xio-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-xio-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-667.5"
  "-665.2"
  )
set_tests_properties (plm-convert-xio-a-stats PROPERTIES 
  DEPENDS plm-convert-xio-a)
set_tests_properties (plm-convert-xio-a-check PROPERTIES 
  DEPENDS plm-convert-xio-a-stats)

plm_add_test (
  "plm-convert-xio-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-xio-4.33.02/20080627A/plan/TRAINING4FLD;--output-img;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-b.mha;--output-xio;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-b-xio-output;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-b-dicom"
  )
plm_add_test (
  "plm-convert-xio-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-xio-b.mha"
  )
plmtest_check_interval ("plm-convert-xio-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-xio-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-667.5"
  "-665.2"
  )
set_tests_properties (plm-convert-xio-b-stats PROPERTIES 
  DEPENDS plm-convert-xio-b)
set_tests_properties (plm-convert-xio-b-check PROPERTIES 
  DEPENDS plm-convert-xio-b-stats)

plm_add_test (
  "plm-warp-xio-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_TESTING_DATA_DIR}/xf-bspline-chest-phantom.txt;--input;${PLM_BUILD_TESTING_DIR}/chest-phantom-xio-4.33.02;--output-xio;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-xio;--output-ss-img;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-ss.nrrd;--output-ss-list;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-ss.txt;--output-labelmap;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-labelmap.nrrd;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-img.nrrd;--output-prefix;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a;--output-cxt;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a.cxt;--output-colormap;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a.ctbl;--default-value;0"
  )
plm_add_test (
  "plm-warp-xio-a-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-img.nrrd"
  )
plmtest_check_interval ("plm-warp-xio-a-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-661.5"
  "-659.9"
  )
plm_add_test (
  "plm-warp-xio-a-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a/Patient.mha"
  )
plmtest_check_interval ("plm-warp-xio-a-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-stats-2.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.385"
  "0.395"
  )
plm_add_test (
  "plm-warp-xio-a-stats-3"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-labelmap.nrrd"
  )
plmtest_check_interval ("plm-warp-xio-a-check-3"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-stats-3.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.67"
  "0.68"
  )
plm_add_test (
  "plm-warp-xio-a-stats-4"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-ss.nrrd"
  )
plmtest_check_interval ("plm-warp-xio-a-check-4"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-stats-4.stdout.txt"
  "S *1 *NVOX *([-0-9.]*)"
  "1970"
  "1971"
  )
add_test ("plm-warp-xio-a-check-5" ${CMAKE_COMMAND} -E compare_files
  "${PLM_TESTING_DATA_DIR}/plm-warp-xio-a-ss.txt"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a-ss.txt")
add_test ("plm-warp-xio-a-check-6" ${CMAKE_COMMAND} -E compare_files
  "${PLM_TESTING_DATA_DIR}/plm-warp-xio-a.ctbl"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-xio-a.ctbl")
set_tests_properties (plm-warp-xio-a-stats-1 PROPERTIES DEPENDS plm-warp-xio-a)
set_tests_properties (plm-warp-xio-a-check-1 
  PROPERTIES DEPENDS plm-warp-xio-a-stats-1)
set_tests_properties (plm-warp-xio-a-stats-2 PROPERTIES DEPENDS plm-warp-xio-a)
set_tests_properties (plm-warp-xio-a-check-2 
  PROPERTIES DEPENDS plm-warp-xio-a-stats-2)
set_tests_properties (plm-warp-xio-a-stats-3 PROPERTIES DEPENDS plm-warp-xio-a)
set_tests_properties (plm-warp-xio-a-check-3 
  PROPERTIES DEPENDS plm-warp-xio-a-stats-3)
set_tests_properties (plm-warp-xio-a-stats-4 PROPERTIES DEPENDS plm-warp-xio-a)
set_tests_properties (plm-warp-xio-a-check-4 
  PROPERTIES DEPENDS plm-warp-xio-a-stats-4)
set_tests_properties (plm-warp-xio-a-check-5 PROPERTIES DEPENDS plm-warp-xio-a)
set_tests_properties (plm-warp-xio-a-check-6 PROPERTIES DEPENDS plm-warp-xio-a)

## -------------------------------------------------------------------------
## plastimatch-convert-prefix-fcsv
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-prefix-fcsv"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input-ss-img;${PLM_BUILD_TESTING_DIR}/donut-ss-1.mha;--output-prefix-fcsv;${PLM_BUILD_TESTING_DIR}/plm-convert-prefix-fcsv"
  )
set_tests_properties (plm-convert-prefix-fcsv PROPERTIES DEPENDS donut-1)

## -------------------------------------------------------------------------
## plastimatch-convert-dose-scale
## -------------------------------------------------------------------------
plm_add_test (
  "plm-convert-dose-scale"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "convert;--input-dose-img;${PLM_BUILD_TESTING_DIR}/sphere-3.mha;--output-dose-img;${PLM_BUILD_TESTING_DIR}/plm-convert-dose-scale.mha;--dose-scale;77.7"
  )
plm_add_test (
  "plm-convert-dose-scale-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-convert-dose-scale.mha"
  )
plmtest_check_interval ("plm-convert-dose-scale-check"
  "${PLM_BUILD_TESTING_DIR}/plm-convert-dose-scale-stats.stdout.txt"
  "MAX *([-0-9.]*)"
  "77.699"
  "77.7"
  )
set_tests_properties (plm-convert-dose-scale PROPERTIES 
  DEPENDS sphere-3)
set_tests_properties (plm-convert-dose-scale-stats PROPERTIES 
  DEPENDS plm-convert-dose-scale)
set_tests_properties (plm-convert-dose-scale-check PROPERTIES 
  DEPENDS plm-convert-dose-scale-stats)

## -------------------------------------------------------------------------
## plm-dice-a  dice, centered and off-centered rect
## plm-dice-b  dice, centered rects of different resolution
## plm-dice-c  HD, centered and off-centered rect
## plm-dice-d  95 HD, centered and off-centered rect
## -------------------------------------------------------------------------
plm_add_test (
  "plm-dice-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "dice;${PLM_BUILD_TESTING_DIR}/rect-4.mha;${PLM_BUILD_TESTING_DIR}/rect-8.mha"
  )
plmtest_check_interval ("plm-dice-a-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-dice-a.stdout.txt"
  "^DICE: *([-0-9.]*)"
  "0.799"
  "0.801"
  )
set_tests_properties (plm-dice-a PROPERTIES DEPENDS "rect-4;rect-8")
set_tests_properties (plm-dice-a-check-1 PROPERTIES DEPENDS "plm-dice-a")

plm_add_test (
  "plm-dice-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "dice;${PLM_BUILD_TESTING_DIR}/rect-4.mha;${PLM_BUILD_TESTING_DIR}/rect-9.mha"
  )
plmtest_check_interval ("plm-dice-b-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-dice-b.stdout.txt"
  "^DICE: *([-0-9.]*)"
  "0.999"
  "1.000"
  )
set_tests_properties (plm-dice-b PROPERTIES DEPENDS "rect-4;rect-9")
set_tests_properties (plm-dice-b-check-1 PROPERTIES DEPENDS "plm-dice-b")

plm_add_test (
  "plm-dice-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "dice;--hausdorff;${PLM_BUILD_TESTING_DIR}/rect-4.mha;${PLM_BUILD_TESTING_DIR}/rect-8.mha"
  )
plmtest_check_interval ("plm-dice-c-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-dice-c.stdout.txt"
  "^Hausdorff distance = *([-0-9.]*)"
  "26.31"
  "26.32"
  )
plmtest_check_interval ("plm-dice-c-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-dice-c.stdout.txt"
  "^Avg average Hausdorff distance = *([-0-9.]*)"
  "3.40"
  "3.41"
  )
set_tests_properties (plm-dice-c PROPERTIES DEPENDS "rect-4;rect-8")
set_tests_properties (plm-dice-c-check-1 PROPERTIES DEPENDS "plm-dice-c")
set_tests_properties (plm-dice-c-check-2 PROPERTIES DEPENDS "plm-dice-c")


## -------------------------------------------------------------------------
## plm-dmap-a  distance map, itk maurer
## plm-dmap-b  distance map, native danielsson
## -------------------------------------------------------------------------
plm_add_test (
  "plm-dmap-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "dmap;--algorithm;maurer;--input;${PLM_BUILD_TESTING_DIR}/rect-4.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-dmap-a.mha"
  )
set_tests_properties (plm-dmap-a PROPERTIES DEPENDS "rect-4")

plm_add_test (
  "plm-dmap-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "dmap;--algorithm;danielsson;--input;${PLM_BUILD_TESTING_DIR}/rect-4.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-dmap-b.mha"
  )
set_tests_properties (plm-dmap-b PROPERTIES DEPENDS "rect-4")

## -------------------------------------------------------------------------
## plm-dvh-a  ss-img, without ss-list
## plm-dvh-b  ss-img, with ss-list
## -------------------------------------------------------------------------
plm_add_test (
  "plm-dvh-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "dvh;--input-ss-img;${PLM_BUILD_TESTING_DIR}/rect-1-ss.mha;--input-dose;${PLM_BUILD_TESTING_DIR}/rect-1-dose-img.mha;--output-csv;${PLM_BUILD_TESTING_DIR}/plm-dvh-a.csv"
  )
plmtest_check_interval ("plm-dvh-a-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-dvh-a.csv"
  "^15,([-0-9.]*),"
  "0.999"
  "1.0"
  )
plmtest_check_interval ("plm-dvh-a-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-dvh-a.csv"
  "^15.5,([-0-9.]*),"
  "0.0"
  "0.001"
  )
set_tests_properties (plm-dvh-a PROPERTIES DEPENDS "rect-1")
set_tests_properties (plm-dvh-a-check-1 PROPERTIES DEPENDS "plm-dvh-a")
set_tests_properties (plm-dvh-a-check-2 PROPERTIES DEPENDS "plm-dvh-a")

plm_add_test (
  "plm-dvh-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "dvh;--input-ss-img;${PLM_BUILD_TESTING_DIR}/rect-1-ss.mha;--input-ss-list;${PLM_BUILD_TESTING_DIR}/rect-1-ss-list.txt;--input-dose;${PLM_BUILD_TESTING_DIR}/rect-1-dose-img.mha;--output-csv;${PLM_BUILD_TESTING_DIR}/plm-dvh-b.csv"
  )
plmtest_check_interval ("plm-dvh-b-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-dvh-b.csv"
  "^15,([-0-9.]*)"
  "0.999"
  "1.0"
  )
plmtest_check_interval ("plm-dvh-b-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-dvh-b.csv"
  "^15.5,([-0-9.]*)"
  "0.0"
  "0.001"
  )
set_tests_properties (plm-dvh-b PROPERTIES DEPENDS "rect-1")
set_tests_properties (plm-dvh-b-check-1 PROPERTIES DEPENDS "plm-dvh-b")
set_tests_properties (plm-dvh-b-check-2 PROPERTIES DEPENDS "plm-dvh-b")

## -------------------------------------------------------------------------
## plastimatch fill/mask
## -------------------------------------------------------------------------
plm_add_test (
  "plm-fill-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "fill;--input;${PLM_BUILD_TESTING_DIR}/rect-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-fill-a.mha;--mask;${PLM_BUILD_TESTING_DIR}/rect-4.mha"
  )
plm_add_test (
  "plm-fill-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-fill-a.mha"
  )
plmtest_check_interval ("plm-fill-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-fill-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-0.1"
  "+0.1"
  )
set_tests_properties (plm-fill-a PROPERTIES DEPENDS "rect-1;rect-4")
set_tests_properties (plm-fill-a-stats PROPERTIES DEPENDS plm-fill-a)
set_tests_properties (plm-fill-a-check PROPERTIES DEPENDS plm-fill-a-stats)

plm_add_test (
  "plm-mask-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "mask;--input;${PLM_BUILD_TESTING_DIR}/rect-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-mask-a.mha;--mask;${PLM_BUILD_TESTING_DIR}/rect-4.mha;--mask-value;-1000"
  )
plm_add_test (
  "plm-mask-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-mask-a.mha"
  )
plmtest_check_interval ("plm-mask-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-mask-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-1000.1"
  "-999.9"
  )
set_tests_properties (plm-mask-a PROPERTIES DEPENDS "rect-1;rect-4")
set_tests_properties (plm-mask-a-stats PROPERTIES DEPENDS plm-mask-a)
set_tests_properties (plm-mask-a-check PROPERTIES DEPENDS plm-mask-a-stats)

## -------------------------------------------------------------------------
## plm-gamma-a  centered and off-center rectangle, centered as reference
## plm-gamma-b  centered rectangles with different dosees
## plm-gamma-c  analysis using dose threshold on both images
## plm-gamma-d  analysis using dose threshold on ref only
## -------------------------------------------------------------------------
plm_add_test (
  "plm-gamma-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "gamma;--compute-full-region;--output;${PLM_BUILD_TESTING_DIR}/plm-gamma-a.mha;${PLM_BUILD_TESTING_DIR}/rect-12.mha;${PLM_BUILD_TESTING_DIR}/rect-14.mha"
  )
plm_add_test (
  "plm-gamma-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-gamma-a.mha"
  )
plmtest_check_interval ("plm-gamma-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-gamma-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.023"
  "0.024"
  )
set_tests_properties (plm-gamma-a PROPERTIES DEPENDS "rect-12;rect-14")
set_tests_properties (plm-gamma-a-stats PROPERTIES DEPENDS plm-gamma-a)
set_tests_properties (plm-gamma-a-check PROPERTIES DEPENDS plm-gamma-a-stats)

plm_add_test (
  "plm-gamma-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "gamma;--output;${PLM_BUILD_TESTING_DIR}/plm-gamma-b.mha;${PLM_BUILD_TESTING_DIR}/rect-12.mha;${PLM_BUILD_TESTING_DIR}/rect-13.mha"
  )
plm_add_test (
  "plm-gamma-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-gamma-b.mha"
  )
plmtest_check_interval ("plm-gamma-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-gamma-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.245"
  "0.255"
  )
set_tests_properties (plm-gamma-b PROPERTIES DEPENDS "rect-12;rect-13")
set_tests_properties (plm-gamma-b-stats PROPERTIES DEPENDS plm-gamma-b)
set_tests_properties (plm-gamma-b-check PROPERTIES DEPENDS plm-gamma-b-stats)

plm_add_test (
  "plm-gamma-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "gamma;--output;${PLM_BUILD_TESTING_DIR}/plm-gamma-c.mha;${PLM_BUILD_TESTING_DIR}/rectarr-03.mha;${PLM_BUILD_TESTING_DIR}/rectarr-04.mha"
  )
plmtest_check_interval ("plm-gamma-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-gamma-c.stdout.txt"
  "Pass rate = *([-0-9.]*)"
  "90.6"
  "90.8"
  )
set_tests_properties (plm-gamma-c PROPERTIES DEPENDS "rectarr-03;rectarr-04")
set_tests_properties (plm-gamma-c-check PROPERTIES DEPENDS plm-gamma-c)

plm_add_test (
  "plm-gamma-d"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "gamma;--output;${PLM_BUILD_TESTING_DIR}/plm-gamma-d.mha;--ref-only-threshold;${PLM_BUILD_TESTING_DIR}/rectarr-03.mha;${PLM_BUILD_TESTING_DIR}/rectarr-04.mha"
  )
plmtest_check_interval ("plm-gamma-d-check"
  "${PLM_BUILD_TESTING_DIR}/plm-gamma-d.stdout.txt"
  "Pass rate = *([-0-9.]*)"
  "89.9"
  "90.1"
  )
set_tests_properties (plm-gamma-d PROPERTIES DEPENDS "rectarr-03;rectarr-04")
set_tests_properties (plm-gamma-d-check PROPERTIES DEPENDS plm-gamma-d)


## -------------------------------------------------------------------------
## plastimatch usage
## -------------------------------------------------------------------------
plm_add_test (
  "plm-usage" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  ""
  )

## -------------------------------------------------------------------------
## plastimatch register (group 1) - basic functionality
##   plm-reg-align-center
##   plm-reg-itk-translation
##   plm-reg-itk-rigid-a
##   plm-reg-itk-rigid-b
##   plm-reg-itk-bspline
##   plm-reg-itk-demons
## -------------------------------------------------------------------------
# For Nocedal optimizer
set (PLM_BSPLINE_TEST_LOWER_THRESH "-746.0")
set (PLM_BSPLINE_TEST_UPPER_THRESH "-744.5")
# For Liblbfgs optimizer
#set (PLM_BSPLINE_TEST_LOWER_THRESH "-706.0")
#set (PLM_BSPLINE_TEST_UPPER_THRESH "-704.5")

plm_add_test (
  "plm-reg-align-center"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-reg-align-center.txt"
  )
plm_add_test (
  "plm-reg-align-center-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "compare;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;${PLM_BUILD_TESTING_DIR}/plm-reg-align-center-img.mha"
  )
plmtest_check_interval ("plm-reg-align-center-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-align-center-stats.stdout.txt"
  "MAE *([-0-9.]*)"
  "-0.005"
  "0.005"
  )
set_property (TEST plm-reg-align-center APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-reg-align-center APPEND PROPERTY DEPENDS gauss-6)
set_tests_properties (plm-reg-align-center-stats PROPERTIES 
  DEPENDS plm-reg-align-center)
set_tests_properties (plm-reg-align-center-check PROPERTIES 
  DEPENDS plm-reg-align-center-stats)

plm_add_test (
  "plm-reg-itk-translation"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-translation.txt"
  )
plm_add_test (
  "plm-reg-itk-translation-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-translation-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-translation-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-translation-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-677.6"
  "-677.5"
  )
set_property (TEST plm-reg-itk-translation APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-reg-itk-translation APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-reg-itk-translation-stats PROPERTIES 
  DEPENDS plm-reg-itk-translation)
set_tests_properties (plm-reg-itk-translation-check PROPERTIES 
  DEPENDS plm-reg-itk-translation-stats)

plm_add_test (
  "plm-reg-itk-rigid-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-a.txt"
  )
plm_add_test (
  "plm-reg-itk-rigid-a-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-a-intermediate-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-rigid-a-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-a-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-939.3"
  "-939.1"
  )
plm_add_test (
  "plm-reg-itk-rigid-a-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-a-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-rigid-a-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-a-stats-2.stdout.txt"
  "AVE *([-0-9.]*)"
  "-939.3"
  "-939.1"
  )
set_tests_properties (plm-reg-itk-rigid-a PROPERTIES 
  DEPENDS "rect-2;rect-3")
set_tests_properties (plm-reg-itk-rigid-a-stats-1 PROPERTIES 
  DEPENDS plm-reg-itk-rigid-a)
set_tests_properties (plm-reg-itk-rigid-a-check-1 PROPERTIES 
  DEPENDS plm-reg-itk-rigid-a-stats-1)
set_tests_properties (plm-reg-itk-rigid-a-stats-2 PROPERTIES 
  DEPENDS plm-reg-itk-rigid-a)
set_tests_properties (plm-reg-itk-rigid-a-check-2 PROPERTIES 
  DEPENDS plm-reg-itk-rigid-a-stats-2)

plm_add_test (
  "plm-reg-itk-similarity"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-similarity.txt"
  )
plm_add_test (
  "plm-reg-itk-similarity-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-similarity-intermediate-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-similarity-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-similarity-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-981.3"
  "-981.1"
  )
plm_add_test (
  "plm-reg-itk-similarity-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-similarity-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-similarity-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-similarity-stats-2.stdout.txt"
  "AVE *([-0-9.]*)"
  "-940.8"
  "-940.5"
  )

set_tests_properties (plm-reg-itk-similarity PROPERTIES
  DEPENDS "rect-2s;rect-2")
set_tests_properties (plm-reg-itk-similarity-stats-1 PROPERTIES
  DEPENDS plm-reg-itk-similarity)
set_tests_properties (plm-reg-itk-similarity-check-1 PROPERTIES
  DEPENDS plm-reg-itk-similaritystats-1)
set_tests_properties (plm-reg-itk-similarity-stats-2 PROPERTIES
  DEPENDS plm-reg-itk-similarity)
set_tests_properties (plm-reg-itk-similarity-check-2 PROPERTIES
  DEPENDS plm-reg-itk-similarity-stats-2)

plm_add_test (
  "plm-reg-itk-rigid-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-b.txt"
  )
plm_add_test (
  "plm-reg-itk-rigid-b-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-b-intermediate-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-rigid-b-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-rigid-b-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-939.3"
  "-939.1"
  )
set_tests_properties (plm-reg-itk-rigid-b PROPERTIES 
  DEPENDS "rect-1;rect-2")
set_tests_properties (plm-reg-itk-rigid-b-stats-1 PROPERTIES 
  DEPENDS plm-reg-itk-rigid-b)
set_tests_properties (plm-reg-itk-rigid-b-check-1 PROPERTIES 
  DEPENDS plm-reg-itk-rigid-b-stats-1)

plm_add_test (
  "plm-reg-itk-bspline"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-bspline.txt"
  )
plm_add_test (
  "plm-reg-itk-bspline-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-bspline-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-bspline-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-bspline-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-764.0"
  "-763.6"
  )
set_property (TEST plm-reg-itk-bspline APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-reg-itk-bspline APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-reg-itk-bspline-stats-1 PROPERTIES 
  DEPENDS plm-reg-itk-bspline)
set_tests_properties (plm-reg-itk-bspline-check-1 PROPERTIES 
  DEPENDS plm-reg-itk-bspline-stats-1)

plm_add_test (
  "plm-reg-itk-demons"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-reg-itk-demons.txt"
  )
plmtest_check_interval ("plm-reg-itk-demons-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-demons.stdout.txt"
  "VF_AVG +[^ ] +([-0-9.]*)"
  "31.85"
  "34.95"
  )
plm_add_test (
  "plm-reg-itk-demons-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-demons-img.mha"
  )
plmtest_check_interval ("plm-reg-itk-demons-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-demons-stats-2.stdout.txt"
  "AVE *([-0-9.]*)"
  "-755.3"
  "-754.6"
  )
set_property (TEST plm-reg-itk-demons PROPERTY DEPENDS "gauss-1;gauss-2")
set_tests_properties (plm-reg-itk-demons-check-1 
  PROPERTIES DEPENDS plm-reg-itk-demons)
set_tests_properties (plm-reg-itk-demons-stats-2 
  PROPERTIES DEPENDS plm-reg-itk-demons)
set_tests_properties (plm-reg-itk-demons-check-2 
  PROPERTIES DEPENDS plm-reg-itk-demons-stats-2)

plm_add_test (
  "plm-reg-itk-demons-diff"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-reg-itk-demons-diff.txt"
  )
plmtest_check_interval ("plm-reg-itk-demons-diff-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-demons-diff.stdout.txt"
  "VF_AVG +[^ ] +([-0-9.]*)"
  "31.2"
  "31.3"
  )
set_property (TEST plm-reg-itk-demons-diff 
  PROPERTY DEPENDS "gauss-1;gauss-2")
set_tests_properties (plm-reg-itk-demons-diff-check-1 
  PROPERTIES DEPENDS plm-reg-itk-demons-diff)

plm_add_test (
  "plm-reg-itk-demons-logd"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-reg-itk-demons-logd.txt"
  )
plmtest_check_interval ("plm-reg-itk-demons-logd-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-demons-logd.stdout.txt"
  "VF_AVG +[^ ] +([-0-9.]*)"
  "32.65"
  "32.75"
  )
set_property (TEST plm-reg-itk-demons-logd 
  PROPERTY DEPENDS "gauss-1;gauss-2")
set_tests_properties (plm-reg-itk-demons-logd-check-1 
  PROPERTIES DEPENDS plm-reg-itk-demons-logd)

plm_add_test (
  "plm-reg-itk-demons-sym-logd"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-reg-itk-demons-sym-logd.txt"
  )
plmtest_check_interval ("plm-reg-itk-demons-sym-logd-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-itk-demons-sym-logd.stdout.txt"
  "VF_AVG +[^ ] +([-0-9.]*)"
  "31.85"
  "35.95"
  )
set_property (TEST plm-reg-itk-demons-sym-logd 
  PROPERTY DEPENDS "gauss-1;gauss-2")
set_tests_properties (plm-reg-itk-demons-sym-logd-check-1 
  PROPERTIES DEPENDS plm-reg-itk-demons-sym-logd)

## -------------------------------------------------------------------------
## plastimatch register (group 2)
##   plm-bsp-mse-c
##   plm-bsp-mse-h
##   plm-bsp-mse-k
##   plm-bsp-mse-l
##   plm-bsp-openmp
##   plm-bsp-cuda
##   plm-bsp-resume
##   plm-bsp-logfile
##   plm-bsp-ushort
##   plm-bsp-double
##   plm-bsp-itk-output
##   plm-bsp-char-output
## -------------------------------------------------------------------------
plm_add_test (
  "plm-bsp-mse-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-c.txt"
  )
plm_add_test (
  "plm-bsp-mse-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-c-img.mha"
  )
plmtest_check_interval ("plm-bsp-mse-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-c-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "${PLM_BSPLINE_TEST_LOWER_THRESH}"
  "${PLM_BSPLINE_TEST_UPPER_THRESH}"
  )
set_property (TEST plm-bsp-mse-c APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-mse-c APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-mse-c-stats PROPERTIES 
  DEPENDS plm-bsp-mse-c)
set_tests_properties (plm-bsp-mse-c-check PROPERTIES 
  DEPENDS plm-bsp-mse-c-stats)

plm_add_test (
  "plm-bsp-mse-h" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-h.txt"
  )
plm_add_test (
  "plm-bsp-mse-h-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-h-img.mha"
  )
plmtest_check_interval ("plm-bsp-mse-h-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-h-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "${PLM_BSPLINE_TEST_LOWER_THRESH}"
  "${PLM_BSPLINE_TEST_UPPER_THRESH}"
  )
set_property (TEST plm-bsp-mse-h APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-mse-h APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-mse-h-stats PROPERTIES 
  DEPENDS plm-bsp-mse-h)
set_tests_properties (plm-bsp-mse-h-check PROPERTIES 
  DEPENDS plm-bsp-mse-h-stats)

plm_add_test (
  "plm-bsp-mse-k" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-k.txt"
  )
plm_add_test (
  "plm-bsp-mse-k-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-k-img.mha"
  )
plmtest_check_interval ("plm-bsp-mse-k-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-k-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "${PLM_BSPLINE_TEST_LOWER_THRESH}"
  "${PLM_BSPLINE_TEST_UPPER_THRESH}"
  )
set_property (TEST plm-bsp-mse-k APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-mse-k APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-mse-k-stats PROPERTIES 
  DEPENDS plm-bsp-mse-k)
set_tests_properties (plm-bsp-mse-k-check PROPERTIES 
  DEPENDS plm-bsp-mse-k-stats)

plm_add_test (
  "plm-bsp-mse-l" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-l.txt"
  )
plm_add_test (
  "plm-bsp-mse-l-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-l-img.mha"
  )
plmtest_check_interval ("plm-bsp-mse-l-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-l-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "${PLM_BSPLINE_TEST_LOWER_THRESH}"
  "${PLM_BSPLINE_TEST_UPPER_THRESH}"
  )
set_property (TEST plm-bsp-mse-l APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-mse-l APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-mse-l-stats PROPERTIES 
  DEPENDS plm-bsp-mse-l)
set_tests_properties (plm-bsp-mse-l-check PROPERTIES 
  DEPENDS plm-bsp-mse-l-stats)

## This test (and bsp-mi-k) fails on windows with 2008 compiler.
plm_add_test (
  "plm-bsp-mi-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mi-c.txt"
  )
plm_add_test (
  "plm-bsp-mi-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-mi-c-img.mha"
  )
plmtest_check_interval ("plm-bsp-mi-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mi-c-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-114.0"
  "-113.0"
  )
set_property (TEST plm-bsp-mi-c APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-mi-c APPEND PROPERTY DEPENDS gauss-3)
set_tests_properties (plm-bsp-mi-c-stats PROPERTIES 
  DEPENDS plm-bsp-mi-c)
set_tests_properties (plm-bsp-mi-c-check PROPERTIES 
  DEPENDS plm-bsp-mi-c-stats)

plm_add_test (
  "plm-bsp-mi-k" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mi-k.txt"
  )
plm_add_test (
  "plm-bsp-mi-k-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-mi-k-img.mha"
  )
plmtest_check_interval ("plm-bsp-mi-k-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-mi-k-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-114.0"
  "-113.0"
  )
set_property (TEST plm-bsp-mi-k APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-mi-k APPEND PROPERTY DEPENDS gauss-3)
set_tests_properties (plm-bsp-mi-k-stats PROPERTIES 
  DEPENDS plm-bsp-mi-k)
set_tests_properties (plm-bsp-mi-k-check PROPERTIES 
  DEPENDS plm-bsp-mi-k-stats)

plm_add_test (
  "plm-bsp-gm-k" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-gm-k.txt"
  )
plm_add_test (
  "plm-bsp-gm-k-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-gm-k-img.mha"
  )
plmtest_check_interval ("plm-bsp-gm-k-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-gm-k-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-122.5"
  "-121.5"
  )
set_property (TEST plm-bsp-gm-k APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-gm-k APPEND PROPERTY DEPENDS gauss-3)
set_tests_properties (plm-bsp-gm-k-stats PROPERTIES 
  DEPENDS plm-bsp-gm-k)
set_tests_properties (plm-bsp-gm-k-check PROPERTIES 
  DEPENDS plm-bsp-gm-k-stats)

plm_add_test (
  "plm-bsp-sm-multi-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-sm-multi-a.txt"
  )
plm_add_test (
  "plm-bsp-sm-multi-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-sm-multi-a-img.mha"
  )
# plmtest_check_interval ("plm-bsp-sm-multi-a-check"
#   "${PLM_BUILD_TESTING_DIR}/plm-bsp-sm-multi-a-stats.stdout.txt"
#   "AVE *([-0-9.]*)"
#   "-122.5"
#   "-121.5"
#   )
set_property (TEST plm-bsp-sm-multi-a APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-sm-multi-a APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-sm-multi-a-stats PROPERTIES 
  DEPENDS plm-bsp-sm-multi-a)
# set_tests_properties (plm-bsp-sm-multi-a-check PROPERTIES 
#   DEPENDS plm-bsp-sm-multi-a-stats)

plm_add_test (
  "plm-bsp-openmp" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-bsp-openmp.txt"
  )
plm_add_test (
  "plm-bsp-openmp-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-openmp-img.mha"
  )
plmtest_check_interval ("plm-bsp-openmp-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-openmp-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "${PLM_BSPLINE_TEST_LOWER_THRESH}"
  "${PLM_BSPLINE_TEST_UPPER_THRESH}"
  )
set_property (TEST plm-bsp-openmp APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-openmp APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-openmp-stats PROPERTIES 
  DEPENDS plm-bsp-openmp)
set_tests_properties (plm-bsp-openmp-check PROPERTIES 
  DEPENDS plm-bsp-openmp-stats)

plm_add_test (
  "plm-bsp-cuda" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-cuda.txt"
  )
plm_add_test (
  "plm-bsp-cuda-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-cuda-img.nrrd"
  )
plmtest_check_interval ("plm-bsp-cuda-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-cuda-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "${PLM_BSPLINE_TEST_LOWER_THRESH}"
  "${PLM_BSPLINE_TEST_UPPER_THRESH}"
  )
set_property (TEST plm-bsp-cuda APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-cuda APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-cuda-stats PROPERTIES 
  DEPENDS plm-bsp-cuda)
set_tests_properties (plm-bsp-cuda-check PROPERTIES 
  DEPENDS plm-bsp-cuda-stats)

plm_add_test (
  "plm-bsp-resume" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-resume.txt"
  )
plm_add_test (
  "plm-bsp-resume-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-resume-img.mha"
  )
# plmtest_check_interval ("plm-bsp-resume-check"
#   "${PLM_BUILD_TESTING_DIR}/plm-bsp-resume-stats.stdout.txt"
#   "AVE *([-0-9.]*)"
#   "${PLM_BSPLINE_TEST_LOWER_THRESH}"
#   "${PLM_BSPLINE_TEST_UPPER_THRESH}"
#   )
set_property (TEST plm-bsp-resume APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-resume APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-resume-stats PROPERTIES 
  DEPENDS plm-bsp-resume)
# set_tests_properties (plm-bsp-resume-check PROPERTIES 
#   DEPENDS plm-bsp-resume-stats)

plm_add_test (
  "plm-bsp-logfile" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-logfile.txt"
  )
plmtest_check_interval ("plm-bsp-logfile-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-logfile-logfile.txt"
  "\\\\[.*,  5] MSE *([-0-9.]*)"
  "281.5"
  "282.5"
  )
set_property (TEST plm-bsp-logfile APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-logfile APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-bsp-logfile-check PROPERTIES 
  DEPENDS plm-bsp-logfile)

plm_add_test (
  "plm-bsp-ushort"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-bsp-ushort.txt"
  )
plm_add_test (
  "plm-bsp-ushort-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-ushort-img.mha"
  )
plmtest_check_interval ("plm-bsp-ushort-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-ushort-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "106.6"
  "107.6"
  )
set_tests_properties (plm-bsp-ushort PROPERTIES 
  DEPENDS "gauss-ushort-1;gauss-ushort-2")
set_property (TEST plm-bsp-ushort 
  APPEND PROPERTY DEPENDS gauss-ushort-1)
set_property (TEST plm-bsp-ushort 
  APPEND PROPERTY DEPENDS gauss-ushort-2)
set_tests_properties (plm-bsp-ushort-stats PROPERTIES 
  DEPENDS plm-bsp-ushort)
set_tests_properties (plm-bsp-ushort-check PROPERTIES 
  DEPENDS plm-bsp-ushort-stats)

plm_add_test (
  "plm-bsp-double"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-double.txt"
  )
plm_add_test (
  "plm-bsp-double-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-double-img.mha"
  )
plmtest_check_interval ("plm-bsp-double-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-double-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "107.6"
  "108.6"
  )
set_tests_properties (plm-bsp-double PROPERTIES 
  DEPENDS "gauss-double-1;gauss-double-2")
set_tests_properties (plm-bsp-double-stats PROPERTIES 
  DEPENDS plm-bsp-double)
set_tests_properties (plm-bsp-double-check PROPERTIES 
  DEPENDS plm-bsp-double-stats)

plm_add_test (
  "plm-bsp-itk-output"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-bsp-itk-output.txt"
  )
set_property (TEST plm-bsp-itk-output APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-bsp-itk-output APPEND PROPERTY DEPENDS gauss-2)

plm_add_test (
  "plm-bsp-char-output"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-bsp-char-output.txt"
  )
plm_add_test (
  "plm-bsp-char-output-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-char-output.mha"
  )
plmtest_check_interval (
  "plm-bsp-char-output-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-char-output-stats.stdout.txt"
  "MIN *([-0-9.]*)"
  "-128.1"
  "-127.9"
  )
set_tests_properties (plm-bsp-char-output PROPERTIES 
  DEPENDS "rect-2;rect-3")
set_tests_properties (plm-bsp-char-output-stats PROPERTIES 
  DEPENDS plm-bsp-char-output)
set_tests_properties (plm-bsp-char-output-check PROPERTIES 
  DEPENDS plm-bsp-char-output-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 3)
##   register-bsp-rect
##   register-bsp-regularize-none
##   register-bsp-regularize-analytic
##   register-bsp-regularize-numeric
## -------------------------------------------------------------------------
plm_add_test (
  "plm-bsp-rect"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-rect.txt"
  )
set_tests_properties (plm-bsp-rect PROPERTIES DEPENDS "rect-2;rect-3")

plm_add_test (
  "plm-bsp-regularize-none" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-bsp-regularize-none.txt"
  )
plm_add_test (
  "plm-bsp-regularize-none-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-regularize-none-vf.mha"
  )
plmtest_check_interval (
  "plm-bsp-regularize-none-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-regularize-none-stats.stdout.txt"
  "MINJAC *([-0-9.]*)"
  "0.31"
  "0.32"
  )
set_tests_properties (plm-bsp-regularize-none PROPERTIES 
  DEPENDS "rect-3;sphere-2")
set_tests_properties (plm-bsp-regularize-none-stats PROPERTIES 
  DEPENDS plm-bsp-regularize-none)
set_tests_properties (plm-bsp-regularize-none-check PROPERTIES 
  DEPENDS plm-bsp-regularize-none-stats)

plm_add_test (
  "plm-bsp-regularize-analytic" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-bsp-regularize-analytic.txt"
  )
plm_add_test (
  "plm-bsp-regularize-analytic-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-regularize-analytic-vf.mha"
  )
plmtest_check_interval (
  "plm-bsp-regularize-analytic-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-regularize-analytic-stats.stdout.txt"
  "MINJAC *([-0-9.]*)"
  "0.415"
  "0.425"
  )
set_tests_properties (plm-bsp-regularize-analytic PROPERTIES 
  DEPENDS "rect-3;sphere-2")
set_tests_properties (plm-bsp-regularize-analytic-stats PROPERTIES 
  DEPENDS plm-bsp-regularize-analytic)
set_tests_properties (plm-bsp-regularize-analytic-check PROPERTIES 
  DEPENDS plm-bsp-regularize-analytic-stats)

plm_add_test (
  "plm-bsp-regularize-numeric" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-regularize-numeric.txt"
  )
plm_add_test (
  "plm-bsp-regularize-numeric-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-regularize-numeric-vf.mha"
  )
plmtest_check_interval (
  "plm-bsp-regularize-numeric-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-regularize-numeric-stats.stdout.txt"
  "MINJAC *([-0-9.]*)"
  "0.440"
  "0.445"
  )
set_tests_properties (plm-bsp-regularize-numeric PROPERTIES 
  DEPENDS "rect-3;sphere-2")
set_tests_properties (plm-bsp-regularize-numeric-stats PROPERTIES 
  DEPENDS plm-bsp-regularize-numeric)
set_tests_properties (plm-bsp-regularize-numeric-check PROPERTIES 
  DEPENDS plm-bsp-regularize-numeric-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 4)
##   plm-bsp-landmark-a
##   plm-bsp-landmark-b
##   plm-bsp-landmark-c         Same as "b", but landmarks in stage
##   plm-bsp-landmark-d         Same as "b", but landmarks within file
##   plm-bsp-landmark-e         Same as "b", but txt instead of fcsv
## -------------------------------------------------------------------------
plm_add_test (
  "plm-bsp-landmark-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-a.txt"
  )
set_tests_properties (plm-bsp-landmark-a PROPERTIES DEPENDS "rect-2;sphere-2")

plm_add_test (
  "plm-bsp-landmark-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-b.txt"
  )
plm_add_test (
  "plm-bsp-landmark-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-b-img.mha"
  )
plmtest_check_interval (
  "plm-bsp-landmark-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-991.5"
  "-991.0"
  )
set_tests_properties (plm-bsp-landmark-b PROPERTIES DEPENDS "rect-15;rect-16")
set_tests_properties (plm-bsp-landmark-b-stats PROPERTIES 
    DEPENDS plm-bsp-landmark-b)
set_tests_properties (plm-bsp-landmark-b-check PROPERTIES 
    DEPENDS plm-bsp-landmark-b-stats)

plm_add_test (
  "plm-bsp-landmark-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-c.txt"
  )
plm_add_test (
  "plm-bsp-landmark-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-c-img.mha"
  )
plmtest_check_interval (
  "plm-bsp-landmark-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-c-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-991.5"
  "-991.0"
  )
set_tests_properties (plm-bsp-landmark-c PROPERTIES DEPENDS "rect-15;rect-16")
set_tests_properties (plm-bsp-landmark-c-stats PROPERTIES 
    DEPENDS plm-bsp-landmark-c)
set_tests_properties (plm-bsp-landmark-c-check PROPERTIES 
    DEPENDS plm-bsp-landmark-c-stats)

plm_add_test (
  "plm-bsp-landmark-d"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-d.txt"
  )
plm_add_test (
  "plm-bsp-landmark-d-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-d-img.mha"
  )
plmtest_check_interval (
  "plm-bsp-landmark-d-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-d-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-991.5"
  "-991.0"
  )
set_tests_properties (plm-bsp-landmark-d PROPERTIES DEPENDS "rect-15;rect-16")
set_tests_properties (plm-bsp-landmark-d-stats PROPERTIES 
    DEPENDS plm-bsp-landmark-d)
set_tests_properties (plm-bsp-landmark-d-check PROPERTIES 
    DEPENDS plm-bsp-landmark-d-stats)

plm_add_test (
  "plm-bsp-landmark-e"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-e.txt"
  )
plm_add_test (
  "plm-bsp-landmark-e-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-e-img.mha"
  )
plmtest_check_interval (
  "plm-bsp-landmark-e-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-landmark-e-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-991.5"
  "-991.0"
  )
set_tests_properties (plm-bsp-landmark-e PROPERTIES DEPENDS "rect-15;rect-16")
set_tests_properties (plm-bsp-landmark-e-stats PROPERTIES 
    DEPENDS plm-bsp-landmark-e)
set_tests_properties (plm-bsp-landmark-e-check PROPERTIES 
    DEPENDS plm-bsp-landmark-e-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 5)
##   plm-bsp-dcos-a       fixed=standard moving=rotated, flavor=g
##   plm-bsp-dcos-b       fixed=rotated moving=standard, flavor=g
##   plm-bsp-dcos-c       fixed=standard moving=rotated, flavor=c
##   plm-bsp-dcos-d       fixed=rotated moving=standard, flavor=c
##   plm-bsp-dcos-e       fixed=standard moving=rotated, flavor=h
##   plm-bsp-dcos-f       fixed=rotated moving=standard, flavor=h
## -------------------------------------------------------------------------
plm_add_test (
  "plm-bsp-dcos-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-a.txt"
  )
plm_add_test (
  "plm-bsp-dcos-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-a-img.nrrd"
  )
plmtest_check_interval (
  "plm-bsp-dcos-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-816"
  "-815"
  )
set_tests_properties (plm-bsp-dcos-a PROPERTIES 
  DEPENDS "rect-2;rect-5")
set_tests_properties (plm-bsp-dcos-a-stats PROPERTIES 
  DEPENDS plm-bsp-dcos-a)
set_tests_properties (plm-bsp-dcos-a-check PROPERTIES 
  DEPENDS plm-bsp-dcos-a-stats)

plm_add_test (
  "plm-bsp-dcos-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-b.txt"
  )
plm_add_test (
  "plm-bsp-dcos-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-b-img.nrrd"
  )
plmtest_check_interval (
  "plm-bsp-dcos-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-796.8"
  "-795.8"
  )
set_tests_properties (plm-bsp-dcos-b PROPERTIES 
  DEPENDS "rect-2;rect-5")
set_tests_properties (plm-bsp-dcos-b-stats PROPERTIES 
  DEPENDS plm-bsp-dcos-b)
set_tests_properties (plm-bsp-dcos-b-check PROPERTIES 
  DEPENDS plm-bsp-dcos-b-stats)

plm_add_test (
  "plm-bsp-dcos-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-c.txt"
  )
plm_add_test (
  "plm-bsp-dcos-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-c-img.nrrd"
  )
plmtest_check_interval (
  "plm-bsp-dcos-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-c-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-816"
  "-815"
  )
set_tests_properties (plm-bsp-dcos-c PROPERTIES 
  DEPENDS "rect-2;rect-5")
set_tests_properties (plm-bsp-dcos-c-stats PROPERTIES 
  DEPENDS plm-bsp-dcos-c)
set_tests_properties (plm-bsp-dcos-c-check PROPERTIES 
  DEPENDS plm-bsp-dcos-c-stats)

plm_add_test (
  "plm-bsp-dcos-d"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-d.txt"
  )
plm_add_test (
  "plm-bsp-dcos-d-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-d-img.nrrd"
  )
plmtest_check_interval (
  "plm-bsp-dcos-d-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-d-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-796.8"
  "-795.8"
  )
set_tests_properties (plm-bsp-dcos-d PROPERTIES 
  DEPENDS "rect-2;rect-5")
set_tests_properties (plm-bsp-dcos-d-stats PROPERTIES 
  DEPENDS plm-bsp-dcos-d)
set_tests_properties (plm-bsp-dcos-d-check PROPERTIES 
  DEPENDS plm-bsp-dcos-d-stats)

plm_add_test (
  "plm-bsp-dcos-e"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-e.txt"
  )
plm_add_test (
  "plm-bsp-dcos-e-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-e-img.nrrd"
  )
plmtest_check_interval (
  "plm-bsp-dcos-e-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-e-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-816"
  "-815"
  )
set_tests_properties (plm-bsp-dcos-e PROPERTIES 
  DEPENDS "rect-2;rect-5")
set_tests_properties (plm-bsp-dcos-e-stats PROPERTIES 
  DEPENDS plm-bsp-dcos-e)
set_tests_properties (plm-bsp-dcos-e-check PROPERTIES 
  DEPENDS plm-bsp-dcos-e-stats)

plm_add_test (
  "plm-bsp-dcos-f"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-f.txt"
  )
plm_add_test (
  "plm-bsp-dcos-f-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-f-img.nrrd"
  )
plmtest_check_interval (
  "plm-bsp-dcos-f-check"
  "${PLM_BUILD_TESTING_DIR}/plm-bsp-dcos-f-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-796.8"
  "-795.8"
  )
set_tests_properties (plm-bsp-dcos-f PROPERTIES 
  DEPENDS "rect-2;rect-5")
set_tests_properties (plm-bsp-dcos-f-stats PROPERTIES 
  DEPENDS plm-bsp-dcos-f)
set_tests_properties (plm-bsp-dcos-f-check PROPERTIES 
  DEPENDS plm-bsp-dcos-f-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 6)
##   plm-reg-multi-a      versor, then demons
##   plm-reg-multi-b      two b-spline stages
## -------------------------------------------------------------------------
plm_add_test (
  "plm-reg-multi-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-multi-a.txt"
  )
plm_add_test (
  "plm-reg-multi-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-multi-a-vf.nrrd"
  )
plmtest_check_interval (
  "plm-reg-multi-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-multi-a-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "-22"
  "-18"
  )
set_tests_properties (plm-reg-multi-a PROPERTIES 
  DEPENDS "rect-2;sphere-2")
set_tests_properties (plm-reg-multi-a-stats PROPERTIES 
  DEPENDS plm-reg-multi-a)
set_tests_properties (plm-reg-multi-a-check PROPERTIES 
  DEPENDS plm-reg-multi-a-stats)

plm_add_test (
  "plm-reg-multi-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-multi-b.txt"
  )
plm_add_test (
  "plm-reg-multi-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-multi-b-img.nrrd"
  )
plmtest_check_interval (
  "plm-reg-multi-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-multi-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-987"
  "-984"
  )
set_tests_properties (plm-reg-multi-b PROPERTIES 
  DEPENDS "rect-2;sphere-2")
set_tests_properties (plm-reg-multi-b-stats PROPERTIES 
  DEPENDS plm-reg-multi-b)
set_tests_properties (plm-reg-multi-b-check PROPERTIES 
  DEPENDS plm-reg-multi-b-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 7) - check default value
##   plm-reg-dv-itk-translation
## -------------------------------------------------------------------------
plm_add_test (
  "plm-reg-dv-itk-translation"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-dv-itk-translation.txt"
  )
plm_add_test (
  "plm-reg-dv-itk-translation-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-dv-itk-translation-img.nrrd"
  )
plmtest_check_interval ("plm-reg-dv-itk-translation-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-dv-itk-translation-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-467.1"
  "-366.9"
  )
set_property (TEST plm-reg-dv-itk-translation APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-reg-dv-itk-translation APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-reg-dv-itk-translation-stats PROPERTIES 
  DEPENDS plm-reg-dv-itk-translation)
set_tests_properties (plm-reg-dv-itk-translation-check PROPERTIES 
  DEPENDS plm-reg-dv-itk-translation-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 8)
##   plm-reg-roi-a         bspline + mi
##   plm-reg-roi-b         itk 
##   plm-reg-roi-c         b-spline, upper rectangle
##   plm-reg-roi-d         b-spline, lower rectangle
##   plm-reg-roi-e         itk (no roi), b-spline (roi)
##   plm-reg-stiffness-a   b-spline, rect vs sphere
## -------------------------------------------------------------------------
plm_add_test (
  "plm-reg-roi-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-roi-a.txt"
  )
set_tests_properties (plm-reg-roi-a PROPERTIES DEPENDS "rect-10;rect-10-mask-1;rect-11;rect-11-mask")

plm_add_test (
  "plm-reg-roi-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-roi-b.txt"
  )
set_tests_properties (plm-reg-roi-b PROPERTIES DEPENDS "rectarr-01;rectarr-02;rectarr-m-01")

plm_add_test (
  "plm-reg-roi-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-roi-c.txt"
  )
set_tests_properties (plm-reg-roi-c PROPERTIES DEPENDS "rectarr-01;rectarr-02;rectarr-m-01")

plm_add_test (
  "plm-reg-roi-d"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-roi-d.txt"
  )
set_tests_properties (plm-reg-roi-d PROPERTIES DEPENDS "rectarr-01;rectarr-02;rectarr-m-02")

plm_add_test (
  "plm-reg-roi-e"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-roi-e.txt"
  )
set_tests_properties (plm-reg-roi-e PROPERTIES DEPENDS "rectarr-01;rectarr-02;rectarr-m-02")

plm_add_test (
    "plm-reg-stiffness-a"
    ${PLM_PLASTIMATCH_PATH}/plastimatch
    "register;${PLM_BUILD_TESTING_DIR}/plm-reg-stiffness-a.txt"
    )
set_tests_properties (plm-reg-stiffness-a
    PROPERTIES DEPENDS "rect-19;roi-1;sphere-4")

## -------------------------------------------------------------------------
## plastimatch register (group 9)
##   plm-reg-process-a
## -------------------------------------------------------------------------
plm_add_test (
  "plm-reg-process-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-process-a.txt"
  )
plm_add_test (
  "plm-reg-process-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-process-a.mha"
  )
plmtest_check_interval ("plm-reg-process-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-process-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-491.0"
  "-490.0"
  )
set_property (TEST plm-reg-process-a APPEND PROPERTY DEPENDS gauss-1)
set_tests_properties (plm-reg-process-a-stats PROPERTIES 
  DEPENDS plm-reg-process-a)
set_tests_properties (plm-reg-process-a-check PROPERTIES 
  DEPENDS plm-reg-process-a-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 10)
##   plm-reg-trans-a     Translation with grid search (mse, legacy form)
##   plm-reg-trans-b     Translation with grid search (mse, new form)
##   plm-reg-trans-mi-a  Translation with grid search (mutual information)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-reg-trans-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-trans-a.txt"
  )
plm_add_test (
  "plm-reg-trans-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-trans-a.mha"
  )
plmtest_check_interval ("plm-reg-trans-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-trans-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-661.75"
  "-661.65"
  )
set_property (TEST plm-reg-trans-a APPEND PROPERTY DEPENDS gauss-1)
set_tests_properties (plm-reg-trans-a-stats PROPERTIES 
  DEPENDS plm-reg-trans-a)
set_tests_properties (plm-reg-trans-a-check PROPERTIES 
  DEPENDS plm-reg-trans-a-stats)

plm_add_test (
  "plm-reg-trans-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-trans-b.txt"
  )
plm_add_test (
  "plm-reg-trans-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-trans-b.mha"
  )
plmtest_check_interval ("plm-reg-trans-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-trans-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-661.75"
  "-661.65"
  )
set_property (TEST plm-reg-trans-b APPEND PROPERTY DEPENDS gauss-1)
set_tests_properties (plm-reg-trans-b-stats PROPERTIES 
  DEPENDS plm-reg-trans-b)
set_tests_properties (plm-reg-trans-b-check PROPERTIES 
  DEPENDS plm-reg-trans-b-stats)

plm_add_test (
  "plm-reg-trans-mi-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-trans-mi-a.txt"
  )
plm_add_test (
  "plm-reg-trans-mi-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-trans-mi-a.mha"
  )
plmtest_check_interval ("plm-reg-trans-mi-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-trans-mi-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-612.75"
  "-612.60"
  )
set_tests_properties (plm-reg-trans-mi-a PROPERTIES DEPENDS "gauss-1;gauss-2")
set_tests_properties (plm-reg-trans-mi-a-stats PROPERTIES 
  DEPENDS plm-reg-trans-mi-a)
set_tests_properties (plm-reg-trans-mi-a-check PROPERTIES 
  DEPENDS plm-reg-trans-mi-a-stats)

## -------------------------------------------------------------------------
## plastimatch register (group 11)
##   plm-reg-autores-a    Manually set sampling rate using mm
##   plm-reg-autores-b    Manually set sampling rate using pct
## -------------------------------------------------------------------------
plm_add_test (
  "plm-reg-autores-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-autores-a.txt"
  )
plm_add_test (
  "plm-reg-autores-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-autores-a.mha"
  )
plmtest_check_interval ("plm-reg-autores-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-autores-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-597.6"
  "-597.5"
  )
set_property (TEST plm-reg-autores-a APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-reg-autores-a APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-reg-autores-a-stats PROPERTIES 
  DEPENDS plm-reg-autores-a)
set_tests_properties (plm-reg-autores-a-check PROPERTIES 
  DEPENDS plm-reg-autores-a-stats)

plm_add_test (
  "plm-reg-autores-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_BUILD_TESTING_DIR}/plm-reg-autores-b.txt"
  )
plm_add_test (
  "plm-reg-autores-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-reg-autores-b.mha"
  )
plmtest_check_interval ("plm-reg-autores-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-reg-autores-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-597.5"
  "-597.4"
  )
set_property (TEST plm-reg-autores-b APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-reg-autores-b APPEND PROPERTY DEPENDS gauss-2)
set_tests_properties (plm-reg-autores-b-stats PROPERTIES 
  DEPENDS plm-reg-autores-b)
set_tests_properties (plm-reg-autores-b-check PROPERTIES 
  DEPENDS plm-reg-autores-b-stats)

## -------------------------------------------------------------------------
## plastimatch compose
##   This test uses registers from gauss-1 to gauss-2 which are done above, 
##   and composes with results from an registration from gauss-2 to 
##   gauss-5.  The composed vf should be correctly align gauss-1 to gauss-5.
##   
##   plm-reg-compose    Make vf for testing composition
##   plm-compose-a           Compose translation and vector field
##   plm-compose-b           Compose vector field and vector field
##   plm-compose-c           Compose bspline and vector field
##   
##   Remark: these tests are correct, in the sense that they work as 
##   implemented.  Transformation vectors which are not defined are 
##   set to zero in plastimatch, and this causes artifacts in the composed 
##   vector field.
## -------------------------------------------------------------------------
plm_add_test (
  "plm-reg-compose" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "${PLM_TESTING_DATA_DIR}/plm-reg-compose.txt"
  )
set_tests_properties (plm-reg-compose PROPERTIES DEPENDS "gauss-2;gauss-5")

plm_add_test (
  "plm-compose-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "compose;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-translation-xf.txt;${PLM_BUILD_TESTING_DIR}/plm-reg-compose-vf.mha;${PLM_BUILD_TESTING_DIR}/plm-compose-a-vf.mha"
  )
plm_add_test (
  "plm-compose-a-warp" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--input;${PLM_BUILD_TESTING_DIR}/gauss-5.mha;--xf;${PLM_BUILD_TESTING_DIR}/plm-compose-a-vf.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-compose-a-warp.mha"
  )
plm_add_test (
  "plm-compose-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-compose-a-vf.mha"
  )
plmtest_check_interval ("plm-compose-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-compose-a-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "13.75"
  "13.85"
  )
set_tests_properties (plm-compose-a PROPERTIES 
  DEPENDS "plm-reg-itk-translation;plm-reg-compose")
set_tests_properties (plm-compose-a-warp PROPERTIES DEPENDS plm-compose-a)
set_tests_properties (plm-compose-a-stats PROPERTIES DEPENDS plm-compose-a)
set_tests_properties (plm-compose-a-check PROPERTIES 
  DEPENDS plm-compose-a-stats)

plm_add_test (
  "plm-compose-b" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "compose;${PLM_BUILD_TESTING_DIR}/plm-reg-itk-translation-vf.mha;${PLM_BUILD_TESTING_DIR}/plm-reg-compose-vf.mha;${PLM_BUILD_TESTING_DIR}/plm-compose-b-vf.mha"
  )
plm_add_test (
  "plm-compose-b-warp" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--input;${PLM_BUILD_TESTING_DIR}/gauss-5.mha;--xf;${PLM_BUILD_TESTING_DIR}/plm-compose-b-vf.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-compose-b-warp.mha"
  )
plm_add_test (
  "plm-compose-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-compose-b-vf.mha"
  )
plmtest_check_interval ("plm-compose-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-compose-b-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "13.75"
  "13.85"
  )
set_tests_properties (plm-compose-b PROPERTIES 
  DEPENDS "plm-reg-itk-translation;plm-reg-compose")
set_tests_properties (plm-compose-b-warp PROPERTIES DEPENDS plm-compose-b)
set_tests_properties (plm-compose-b-stats PROPERTIES DEPENDS plm-compose-b)
set_tests_properties (plm-compose-b-check PROPERTIES 
  DEPENDS plm-compose-b-stats)

plm_add_test (
  "plm-compose-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "compose;${PLM_BUILD_TESTING_DIR}/plm-bsp-mse-h-xf.txt;${PLM_BUILD_TESTING_DIR}/plm-reg-compose-vf.mha;${PLM_BUILD_TESTING_DIR}/plm-compose-c-vf.mha"
  )
plm_add_test (
  "plm-compose-c-warp" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--input;${PLM_BUILD_TESTING_DIR}/gauss-5.mha;--xf;${PLM_BUILD_TESTING_DIR}/plm-compose-c-vf.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-compose-c-warp.mha"
  )
plm_add_test (
  "plm-compose-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-compose-c-vf.mha"
  )
plmtest_check_interval ("plm-compose-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-compose-c-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "12.5"
  "12.6"
  )
set_tests_properties (plm-compose-c PROPERTIES 
  DEPENDS "plm-reg-itk-translation;plm-reg-compose")
set_tests_properties (plm-compose-c-warp PROPERTIES DEPENDS plm-compose-c)
set_tests_properties (plm-compose-c-stats PROPERTIES DEPENDS plm-compose-c)
set_tests_properties (plm-compose-c-check PROPERTIES 
  DEPENDS plm-compose-c-stats)

## -------------------------------------------------------------------------
## plm-resample-a    Image, subsample
## plm-resample-b    Image, resample to fixed
## plm-resample-c    Vector field, resample to fixed
## -------------------------------------------------------------------------
plm_add_test (
  "plm-resample-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "resample;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-resample-a.mha;--output-type;float;--subsample;2 2 2"
  )
plm_add_test (
  "plm-resample-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-resample-a.mha"
  )
plmtest_check_interval ("plm-resample-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-resample-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-879.0"
  "-878.0"
  )
set_tests_properties (plm-resample-a PROPERTIES DEPENDS gauss-1)
set_tests_properties (plm-resample-a-stats PROPERTIES DEPENDS plm-resample-a)
set_tests_properties (plm-resample-a-check PROPERTIES 
  DEPENDS plm-resample-a-stats)

plm_add_test (
  "plm-resample-b" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "resample;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-resample-b.mha;--output-type;float;--fixed;${PLM_BUILD_TESTING_DIR}/black-1.mha"
  )
plm_add_test (
  "plm-resample-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-resample-b.mha"
  )
plmtest_check_interval ("plm-resample-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-resample-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-879.0"
  "-878.0"
  )
set_property (TEST plm-resample-b APPEND PROPERTY DEPENDS gauss-1)
set_property (TEST plm-resample-b APPEND PROPERTY DEPENDS black-1)
set_tests_properties (plm-resample-b-stats PROPERTIES DEPENDS plm-resample-b)
set_tests_properties (plm-resample-b-check PROPERTIES 
  DEPENDS plm-resample-b-stats)

plm_add_test (
  "plm-resample-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "resample;--input;${PLM_BUILD_TESTING_DIR}/bspline-mi-c-1-vf.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-resample-c-vf.mha;--fixed;${PLM_BUILD_TESTING_DIR}/black-1.mha"
  )
plm_add_test (
  "plm-resample-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-resample-c-vf.mha"
  )
plmtest_check_interval ("plm-resample-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-resample-c-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "6.67"
  "6.74"
  )
set_tests_properties (plm-resample-c PROPERTIES 
  DEPENDS "bspline-mi-c-1;black-1")
set_tests_properties (plm-resample-c-stats PROPERTIES DEPENDS plm-resample-c)
set_tests_properties (plm-resample-c-check PROPERTIES 
  DEPENDS plm-resample-c-stats)

## -------------------------------------------------------------------------
## plastimatch scale
##  plm-scale-a      Scale an image
##  plm-scale-vf-a   Scale a vector field
## -------------------------------------------------------------------------
plm_add_test (
  "plm-scale-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "scale;--output;${PLM_BUILD_TESTING_DIR}/plm-scale-a.nrrd;--weight;0.4;${PLM_BUILD_TESTING_DIR}/rect-1.mha"
  )
plm_add_test (
  "plm-scale-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-scale-a.nrrd"
  )
plmtest_check_interval ("plm-scale-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-scale-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-3.735"
  "-3.730"
  )
set_tests_properties (plm-scale-a PROPERTIES DEPENDS "rect-1")
set_tests_properties (plm-scale-a-stats PROPERTIES DEPENDS plm-scale-a)
set_tests_properties (plm-scale-a-check PROPERTIES DEPENDS plm-scale-a-stats)

plm_add_test (
  "plm-scale-vf-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "scale;--output;${PLM_BUILD_TESTING_DIR}/plm-scale-vf-a.nrrd;--weight;0.4;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha"
  )
plm_add_test (
  "plm-scale-vf-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-scale-vf-a.nrrd"
  )
plmtest_check_interval ("plm-scale-vf-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-scale-vf-a-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "4.000"
  "4.000"
  )
set_tests_properties (plm-scale-vf-a PROPERTIES DEPENDS vf-trans-1)
set_tests_properties (plm-scale-vf-a-stats PROPERTIES DEPENDS plm-scale-vf-a)
set_tests_properties (plm-scale-vf-a-check PROPERTIES DEPENDS plm-scale-vf-a-stats)

## -------------------------------------------------------------------------
## plastimatch sift
##  plm-sift-a       Run SIFT on rect
## -------------------------------------------------------------------------
plm_add_test (
  "plm-sift-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "sift;--output-ps-1;${PLM_BUILD_TESTING_DIR}/plm-sift-a-ps-1.fcsv;--output-ps-2;${PLM_BUILD_TESTING_DIR}/plm-sift-a-ps-2.fcsv;--output-match-1;${PLM_BUILD_TESTING_DIR}/plm-sift-a-match-1.fcsv;--output-match-2;${PLM_BUILD_TESTING_DIR}/plm-sift-a-match-2.fcsv;${PLM_BUILD_TESTING_DIR}/lung-1.mha;${PLM_BUILD_TESTING_DIR}/lung-2.mha"
  )
plmtest_check_string ("plm-sift-a-check-1" 
  "${PLM_BUILD_TESTING_DIR}/plm-sift-a-match-1.fcsv"
  "^p1-1,(.*)"
  "-7.500,7.500,-17.500"
  )
plmtest_check_string ("plm-sift-a-check-2" 
  "${PLM_BUILD_TESTING_DIR}/plm-sift-a-match-2.fcsv"
  "^p1-1,(.*)"
  "-7.500,7.500,-22.500"
  )
set_tests_properties (plm-sift-a PROPERTIES DEPENDS "lung-1;lung-2")
set_tests_properties (plm-sift-a-check-1 PROPERTIES DEPENDS plm-sift-a)
set_tests_properties (plm-sift-a-check-2 PROPERTIES DEPENDS plm-sift-a)

## -------------------------------------------------------------------------
## plm-thumbnail-a: make png file of sphere
## plm-thumbnail-b: make png file of gauss
## plm-thumbnail-c: make png file of gauss (auto-adjust)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-thumbnail-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "thumbnail;--input;${PLM_BUILD_TESTING_DIR}/sphere-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-thumbnail-a.png;--spacing;2"
  )
plm_add_test (
  "plm-thumbnail-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-thumbnail-a.png"
  )
plmtest_check_interval ("plm-thumbnail-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-thumbnail-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "76.85"
  "76.95"
  )
set_tests_properties (plm-thumbnail-a PROPERTIES DEPENDS sphere-1)
set_tests_properties (plm-thumbnail-a-stats PROPERTIES DEPENDS plm-thumbnail-a)
set_tests_properties (plm-thumbnail-a-check PROPERTIES 
  DEPENDS plm-thumbnail-a-stats)

plm_add_test (
  "plm-thumbnail-b" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "thumbnail;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-thumbnail-b.png;--spacing;2"
  )
plm_add_test (
  "plm-thumbnail-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-thumbnail-b.png"
  )
plmtest_check_interval ("plm-thumbnail-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-thumbnail-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.0"
  "0.0"
  )
set_tests_properties (plm-thumbnail-b PROPERTIES DEPENDS gauss-1)
set_tests_properties (plm-thumbnail-b-stats PROPERTIES DEPENDS plm-thumbnail-b)
set_tests_properties (plm-thumbnail-b-check PROPERTIES 
  DEPENDS plm-thumbnail-b-stats)

plm_add_test (
  "plm-thumbnail-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "thumbnail;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-thumbnail-c.png;--spacing;2;--auto-adjust"
  )
plm_add_test (
  "plm-thumbnail-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-thumbnail-c.png"
  )
plmtest_check_interval ("plm-thumbnail-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-thumbnail-c-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "8.1"
  "8.3"
  )
set_tests_properties (plm-thumbnail-c PROPERTIES DEPENDS gauss-1)
set_tests_properties (plm-thumbnail-c-stats PROPERTIES DEPENDS plm-thumbnail-c)
set_tests_properties (plm-thumbnail-c-check PROPERTIES 
  DEPENDS plm-thumbnail-c-stats)

## -------------------------------------------------------------------------
## plm-union
## -------------------------------------------------------------------------
plm_add_test (
  "plm-union"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "union;--output;${PLM_BUILD_TESTING_DIR}/plm-union.mha;${PLM_BUILD_TESTING_DIR}/rect-10-roi-1.mha;${PLM_BUILD_TESTING_DIR}/rect-11-roi.mha"
  )
plm_add_test (
  "plm-union-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-union.mha"
  )
plmtest_check_interval ("plm-union-check"
  "${PLM_BUILD_TESTING_DIR}/plm-union-stats.stdout.txt"
  "NONZERO *([-0-9.]*)"
  "1271"
  "1271"
  )
set_tests_properties (plm-union 
  PROPERTIES DEPENDS "rect-10-roi-1;rect-11-roi")
set_tests_properties (plm-union-stats PROPERTIES DEPENDS plm-union)
set_tests_properties (plm-union-check PROPERTIES DEPENDS plm-union-stats)

## -------------------------------------------------------------------------
## plm-threshold-a
## plm-threshold-b
## plm-threshold-c
## -------------------------------------------------------------------------
plm_add_test (
  "plm-threshold-a"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "threshold;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-threshold-a.mha;--above;-888"
  )
plm_add_test (
  "plm-threshold-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-threshold-a.mha"
  )
plmtest_check_interval ("plm-threshold-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-threshold-a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.306"
  "0.308"
  )
set_tests_properties (plm-threshold-a PROPERTIES DEPENDS "gauss-1")
set_tests_properties (plm-threshold-a-stats 
  PROPERTIES DEPENDS plm-threshold-a)
set_tests_properties (plm-threshold-a-check 
  PROPERTIES DEPENDS plm-threshold-a-stats)

plm_add_test (
  "plm-threshold-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "threshold;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-threshold-b.mha;--below;-900"
  )
plm_add_test (
  "plm-threshold-b-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-threshold-b.mha"
  )
plmtest_check_interval ("plm-threshold-b-check"
  "${PLM_BUILD_TESTING_DIR}/plm-threshold-b-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.670"
  "0.672"
  )
set_tests_properties (plm-threshold-b PROPERTIES DEPENDS "gauss-1")
set_tests_properties (plm-threshold-b-stats 
  PROPERTIES DEPENDS plm-threshold-b)
set_tests_properties (plm-threshold-b-check 
  PROPERTIES DEPENDS plm-threshold-b-stats)

plm_add_test (
  "plm-threshold-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "threshold;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-threshold-c.mha;--range;-900,-888"
  )
plm_add_test (
  "plm-threshold-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-threshold-c.mha"
  )
plmtest_check_interval ("plm-threshold-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-threshold-c-stats.stdout.txt"
  "NONZERO *([-0-9.]*)"
  "1206"
  "1206"
  )
set_tests_properties (plm-threshold-c PROPERTIES DEPENDS "gauss-1")
set_tests_properties (plm-threshold-c-stats 
  PROPERTIES DEPENDS plm-threshold-c)
set_tests_properties (plm-threshold-c-check 
  PROPERTIES DEPENDS plm-threshold-c-stats)

## -------------------------------------------------------------------------
## plastimatch warp pointset
##   plm-warp-pointset-a:  xf = bspline, output fcsv
##   plm-warp-pointset-b:  xf = bspline, output txt
##   plm-warp-pointset-c:  xf = vf, output fcsv
## -------------------------------------------------------------------------
plm_add_test (
  "plm-warp-pointset-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_BUILD_TESTING_DIR}/plm-bsp-rect-xf.txt;--input;${PLM_TESTING_DATA_DIR}/fiducials-rect-2.fcsv;--output-pointset;${PLM_BUILD_TESTING_DIR}/plm-warp-pointset-a.fcsv"
  )
plm_add_test (
  "plm-warp-pointset-b" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_BUILD_TESTING_DIR}/plm-bsp-rect-xf.txt;--input;${PLM_TESTING_DATA_DIR}/fiducials-rect-2.fcsv;--output-pointset;${PLM_BUILD_TESTING_DIR}/plm-warp-pointset-b.txt"
  )
plm_add_test (
  "plm-warp-pointset-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_BUILD_TESTING_DIR}/landmark-warp-c-vf.mha;--input;${PLM_TESTING_DATA_DIR}/fiducials-rect-2.fcsv;--output-pointset;${PLM_BUILD_TESTING_DIR}/plm-warp-landmark-c.fcsv"
  )
set_tests_properties (plm-warp-pointset-a PROPERTIES DEPENDS plm-bsp-rect)
set_tests_properties (plm-warp-pointset-b PROPERTIES DEPENDS plm-bsp-rect)
set_tests_properties (plm-warp-pointset-c PROPERTIES DEPENDS landmark-warp-c)

## -------------------------------------------------------------------------
## plastimatch warp
##   plm warp a: xf = bspline, algorithm = itk
##   plm warp b: xf = bspline, algorithm = native
##   plm warp c: xf = bspline, algorithm = itk, interp = nn
##   plm warp d: xf = bspline, algorithm = native, interp = nn
##   plm warp e: input = xio, multiple outputs
##   plm warp f: input = dicom, multiple outputs
##   plm warp g: xf = translation
##   plm warp h: xf = translation, ushort images
##   plm warp i: xf = translation, double images
## -------------------------------------------------------------------------
plm_add_test (
  "plm-warp-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--algorithm;itk;--xf;${PLM_BUILD_TESTING_DIR}/bspline_c_xf.txt;--output-vf;${PLM_BUILD_TESTING_DIR}/plm-warp-a-vf.mha;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-a.mha"
  )
plm_add_test (
  "plm-warp-a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-a-vf.mha"
  )
plmtest_check_interval ("plm-warp-a-check"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-a-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "5.77"
  "5.78"
  )
## Note: don't include dependency on gauss-1 as workaround for cmake bug.
set_property (TEST plm-warp-a APPEND PROPERTY DEPENDS bspline-c)
#set_property (TEST plm-warp-a APPEND PROPERTY DEPENDS gauss-1)
set_tests_properties (plm-warp-a-stats PROPERTIES DEPENDS plm-warp-a)
set_tests_properties (plm-warp-a-check PROPERTIES DEPENDS plm-warp-a-stats)

plm_add_test (
  "plm-warp-b" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_BUILD_TESTING_DIR}/bspline_c_xf.txt;--output-vf;${PLM_BUILD_TESTING_DIR}/plm-warp-b-vf.mha;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-b.mha;--default-value;0"
  )
plm_add_test (
  "plm-warp-b-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-b-vf.mha"
  )
plmtest_check_interval ("plm-warp-b-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-b-stats-1.stdout.txt"
  "Mean: *([-0-9.]*)"
  "5.77"
  "5.78"
  )
plm_add_test (
  "plm-warp-b-stats-2"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-b.mha"
  )
plmtest_check_interval ("plm-warp-b-check-2"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-b-stats-2.stdout.txt"
  "AVE *([-0-9.]*)"
  "-775.7"
  "-775.6"
  )
## Note: don't include dependency on gauss-1 as workaround for cmake bug.
set_property (TEST plm-warp-b APPEND PROPERTY DEPENDS bspline-c)
#set_property (TEST plm-warp-b APPEND PROPERTY DEPENDS gauss-1)
set_tests_properties (plm-warp-b-stats-1 PROPERTIES DEPENDS plm-warp-b)
set_tests_properties (plm-warp-b-check-1 PROPERTIES DEPENDS plm-warp-b-stats-1)
set_tests_properties (plm-warp-b-stats-2 PROPERTIES DEPENDS plm-warp-b)
set_tests_properties (plm-warp-b-check-2 PROPERTIES DEPENDS plm-warp-b-stats-2)

plm_add_test (
  "plm-warp-c" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--algorithm;itk;--xf;${PLM_BUILD_TESTING_DIR}/bspline_c_xf.txt;--output-vf;${PLM_BUILD_TESTING_DIR}/plm-warp-c-vf.mha;--input;${PLM_BUILD_TESTING_DIR}/sphere-1.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-c.mha;--interpolation;nn;--default-value;0"
  )
plm_add_test (
  "plm-warp-c-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-c.mha"
  )
plmtest_check_interval ("plm-warp-c-check"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-c-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "9.2"
  "9.3"
  )
set_tests_properties (plm-warp-c PROPERTIES DEPENDS "bspline-c;sphere-1")
set_tests_properties (plm-warp-c-stats PROPERTIES DEPENDS plm-warp-c)
set_tests_properties (plm-warp-c-check PROPERTIES DEPENDS plm-warp-c-stats)

plm_add_test (
  "plm-warp-d" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_BUILD_TESTING_DIR}/bspline_c_xf.txt;--output-vf;${PLM_BUILD_TESTING_DIR}/plm-warp-d-vf.mha;--input;${PLM_BUILD_TESTING_DIR}/sphere-1.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-d.mha;--interpolation;nn;--default-value;0"
  )
plm_add_test (
  "plm-warp-d-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-warp-d.mha"
  )
plmtest_check_interval ("plm-warp-d-check"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-d-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "9.2"
  "9.3"
  )
set_tests_properties (plm-warp-d PROPERTIES DEPENDS "bspline-c;sphere-1")
set_tests_properties (plm-warp-d-stats PROPERTIES DEPENDS plm-warp-d)
set_tests_properties (plm-warp-d-check PROPERTIES DEPENDS plm-warp-d-stats)

plm_add_test (
  "plm-warp-g" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_TESTING_DATA_DIR}/xf-translation-1.txt;--input;${PLM_BUILD_TESTING_DIR}/gauss-1.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-g-img.nrrd;--default-value;0"
  )
plm_add_test (
  "plm-warp-g-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;plm-warp-g-img.nrrd"
  )
plmtest_check_interval ("plm-warp-g-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-g-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "-675.5"
  "-674.5"
  )
set_tests_properties (plm-warp-g PROPERTIES DEPENDS gauss-1)
set_tests_properties (plm-warp-g-stats-1 PROPERTIES DEPENDS plm-warp-g)
set_tests_properties (plm-warp-g-check-1 PROPERTIES DEPENDS plm-warp-g-stats-1)

plm_add_test (
  "plm-warp-h" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_TESTING_DATA_DIR}/xf-translation-1.txt;--input;${PLM_BUILD_TESTING_DIR}/gauss-ushort-1.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-h-img.nrrd;--default-value;0"
  )
plm_add_test (
  "plm-warp-h-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;plm-warp-h-img.nrrd"
  )
plmtest_check_interval ("plm-warp-h-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-h-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "113.5"
  "114.0"
  )
set_tests_properties (plm-warp-h PROPERTIES DEPENDS gauss-ushort-1)
set_tests_properties (plm-warp-h-stats-1 PROPERTIES DEPENDS plm-warp-h)
set_tests_properties (plm-warp-h-check-1 PROPERTIES DEPENDS plm-warp-h-stats-1)

plm_add_test (
  "plm-warp-i" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--xf;${PLM_TESTING_DATA_DIR}/xf-translation-1.txt;--input;${PLM_BUILD_TESTING_DIR}/gauss-double-1.mha;--output-img;${PLM_BUILD_TESTING_DIR}/plm-warp-i-img.nrrd;--default-value;0"
  )
plm_add_test (
  "plm-warp-i-stats-1"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;plm-warp-i-img.nrrd"
  )
plmtest_check_interval ("plm-warp-i-check-1"
  "${PLM_BUILD_TESTING_DIR}/plm-warp-i-stats-1.stdout.txt"
  "AVE *([-0-9.]*)"
  "114.4"
  "114.7"
  )
set_tests_properties (plm-warp-i PROPERTIES DEPENDS gauss-double-1)
set_tests_properties (plm-warp-i-stats-1 PROPERTIES DEPENDS plm-warp-i)
set_tests_properties (plm-warp-i-check-1 PROPERTIES DEPENDS plm-warp-i-stats-1)

## -------------------------------------------------------------------------
## plm-xf-convert-a    plm bspline to vf
## plm-xf-convert-b    vf to plm bspline
## plm-xf-convert-c    plm bspline to vf
## plm-xf-convert-d    plm bspline to itk bspline
## plm-xf-convert-e    itk rigid to dicom rigid (referenced dicom images)
## plm-xf-convert-f    itk rigid to dicom rigid (exported dicom images)
## -------------------------------------------------------------------------
plm_add_test (
  "plm-xf-convert-a" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "xf-convert;--input;${PLM_TESTING_DATA_DIR}/xf-bspline-1.txt;--output;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-a-vf.mha;--output-type;vf;--dim;4 4 4;--origin;-187.5 -187.5 -187.5;--spacing;125 125 125;"
  )

plm_add_test (
  "plm-xf-convert-b"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "xf-convert;--input;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-a-vf.mha;--output;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-b.txt;--output-type;bspline;--dim;4 4 4;--origin;-187.5 -187.5 -187.5;--spacing;125 125 125;--grid-spacing;10 10 10"
  )
set_tests_properties (plm-xf-convert-b PROPERTIES DEPENDS plm-xf-convert-a)

plm_add_test (
  "plm-xf-convert-c"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "xf-convert;--input;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-b.txt;--output;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-c-vf.mha;--output-type;vf;--dim;4 4 4;--origin;-187.5 -187.5 -187.5;--spacing;125 125 125"
  )
set_tests_properties (plm-xf-convert-c PROPERTIES DEPENDS plm-xf-convert-b)

plm_add_test (
  "plm-xf-convert-d" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "xf-convert;--input;${PLM_TESTING_DATA_DIR}/plm-xf-convert.txt;--output;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-xf.tfm;--output-type;itk_bspline"
  )
plm_add_test (
  "plm-xf-convert-d-warp-1" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--input;${PLM_BUILD_TESTING_DIR}/gauss-2.mha;--xf;${PLM_TESTING_DATA_DIR}/plm-xf-convert.txt;--output-img;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-warp-1.nrrd"
  )
plm_add_test (
  "plm-xf-convert-d-warp-1-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-warp-1.nrrd"
  )
plmtest_check_interval (
  "plm-xf-convert-d-warp-1-check"
  "${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-warp-1-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "-745.8"
  "-744.8"
  )
plm_add_test (
  "plm-xf-convert-d-warp-2" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "warp;--input;${PLM_BUILD_TESTING_DIR}/gauss-2.mha;--xf;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-xf.tfm;--output-img;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-warp-2.nrrd"
  )
plm_add_test (
  "plm-xf-convert-d-warp-2-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-warp-2.nrrd"
  )
plmtest_check_interval (
  "plm-xf-convert-d-warp-2-check"
  "${PLM_BUILD_TESTING_DIR}/plm-xf-convert-d-warp-2-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  # GCS 2011-10-08.  The ITK B-spline resampling method is very poor, and 
  # gives different results on different platforms.  Probably we need 
  # to convert all ITK B-splines to native, and resample using the 
  # native method.
  "-751.0"
  #  "-745.8"
  "-744.8"
  )
set_tests_properties (plm-xf-convert-d-warp-1 PROPERTIES DEPENDS gauss-2)
set_tests_properties (plm-xf-convert-d-warp-1-stats PROPERTIES 
  DEPENDS plm-xf-convert-d-warp-1)
set_tests_properties (plm-xf-convert-d-warp-1-check PROPERTIES 
  DEPENDS plm-xf-convert-d-warp-1-stats)
set_tests_properties (plm-xf-convert-d-warp-2 PROPERTIES 
  DEPENDS "gauss-2;plm-xf-convert-d")
set_tests_properties (plm-xf-convert-d-warp-2-stats PROPERTIES 
  DEPENDS plm-xf-convert-d-warp-2)
set_tests_properties (plm-xf-convert-d-warp-2-check PROPERTIES 
  DEPENDS plm-xf-convert-d-warp-2-stats)

plm_add_test (
  "plm-xf-convert-e" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "xf-convert;--input;${PLM_TESTING_DATA_DIR}/itk-rigid-a.tfm;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-e;--source-rcs;${PLM_BUILD_TESTING_DIR}/rect-2-dicom;--registered-rcs;${PLM_BUILD_TESTING_DIR}/rect-3-dicom"
  )
set_tests_properties (plm-xf-convert-e PROPERTIES DEPENDS "rect-2;rect-3")

plm_add_test (
  "plm-xf-convert-f" 
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "xf-convert;--input;${PLM_TESTING_DATA_DIR}/itk-rigid-a.tfm;--output-dicom;${PLM_BUILD_TESTING_DIR}/plm-xf-convert-f;--fixed-image;${PLM_BUILD_TESTING_DIR}/rect-2.mha;--moving-image;${PLM_BUILD_TESTING_DIR}/rect-3.mha"
  )
set_tests_properties (plm-xf-convert-f PROPERTIES DEPENDS "rect-2;rect-3")

## -------------------------------------------------------------------------
## vf-invert
## -------------------------------------------------------------------------
## vf-invert-zero-1    invert zero vector field
## vf-invert-trans-1   invert translation x direction, 1cm
## vf-invert-gauss-1   invert gaussian warp, 10cm std, 1cm x, 2cm y
## -------------------------------------------------------------------------
plm_add_test (
  "vf-invert-zero-1"
  ${PLM_PLASTIMATCH_PATH}/vf_invert
  "--input;${PLM_BUILD_TESTING_DIR}/vf-zero.mha;--fixed;${PLM_BUILD_TESTING_DIR}/vf-zero.mha;--output;${PLM_BUILD_TESTING_DIR}/vf-invert-zero-1.mha"
  )
plm_add_test (
  "vf-invert-zero-1-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/vf-invert-zero-1.mha"
  )
plmtest_check_interval ("vf-invert-zero-1-check"
  "${PLM_BUILD_TESTING_DIR}/vf-invert-zero-1-stats.stdout.txt"
  "Mean abs: *([-0-9.]*)"
  "0.000"
  "0.000"
  )
set_tests_properties (vf-invert-zero-1 PROPERTIES DEPENDS vf-zero)
set_tests_properties (vf-invert-zero-1-stats 
  PROPERTIES DEPENDS vf-invert-zero-1)
set_tests_properties (vf-invert-zero-1-check 
  PROPERTIES DEPENDS vf-invert-zero-1-stats)

plm_add_test (
  "vf-invert-trans-1"
  ${PLM_PLASTIMATCH_PATH}/vf_invert
  "--input;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha;--fixed;${PLM_BUILD_TESTING_DIR}/vf-trans-1.mha;--output;${PLM_BUILD_TESTING_DIR}/vf-invert-trans-1.mha;--iterations;10"
  )
plm_add_test (
  "vf-invert-trans-1-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/vf-invert-trans-1.mha"
  )
plmtest_check_interval ("vf-invert-trans-1-check"
  "${PLM_BUILD_TESTING_DIR}/vf-invert-trans-1-stats.stdout.txt"
  "Mean: *([-0-9.]*)"
  "-10.0"
  "-9.9"
  )
set_tests_properties (vf-invert-trans-1 PROPERTIES DEPENDS vf-trans)
set_tests_properties (vf-invert-trans-1-stats 
  PROPERTIES DEPENDS vf-invert-trans-1)
set_tests_properties (vf-invert-trans-1-check 
  PROPERTIES DEPENDS vf-invert-trans-1-stats)

## -------------------------------------------------------------------------
## bragg-curve
## proton-dose-1    Flavor a, sobp
## proton-dose-2    Flavor f, single bragg peak, target, autom. rgcomp
## proton-dose-3    Flavor g, multiple beam
## proton-dose-4    Flavor h, load range comp file
## proton-dose-5a   Flavor a, sobp target with aperture and rgc
## proton-dose-5f   Flavor f, sobp target with aperture and rgc
## proton-dose-5g   Flavor g, sobp target with aperture and rgc
## proton-dose-5h   Flavor h, sobp target with aperture and rgc
## proton-dose-6a   Flavor a, target geometry differs from CT geometry
## -------------------------------------------------------------------------
plm_add_test (
  "bragg-curve"
  ${PLM_PLASTIMATCH_PATH}/bragg_curve
  "-O;${PLM_BUILD_TESTING_DIR}/bragg-curve.txt"
  )
add_test (bragg-curve-check ${CMAKE_COMMAND} -E compare_files
  "${PLM_TESTING_DATA_DIR}/bragg-curve.txt"
  "${PLM_BUILD_TESTING_DIR}/bragg-curve.txt")
set_tests_properties (bragg-curve-check PROPERTIES DEPENDS bragg-curve)

plm_add_test (
    "proton-dose-1"
    ${PLM_PLASTIMATCH_PATH}/proton_dose
    "${PLM_BUILD_TESTING_DIR}/proton-dose-1.txt"
    )
plm_add_test (
  "proton-dose-1-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/proton-dose-1.mha"
  )
plmtest_check_interval ("proton-dose-1-check"
  "${PLM_BUILD_TESTING_DIR}/proton-dose-1-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.140"
  "0.145"
  )
set_tests_properties (proton-dose-1 PROPERTIES DEPENDS rect-17)
set_tests_properties (proton-dose-1-stats
    PROPERTIES DEPENDS proton-dose-1)
set_tests_properties (proton-dose-1-check
    PROPERTIES DEPENDS proton-dose-1-stats)

plm_add_test (
    "proton-dose-2"
    ${PLM_PLASTIMATCH_PATH}/proton_dose
    "${PLM_BUILD_TESTING_DIR}/proton-dose-2.txt"
    )
plm_add_test (
  "proton-dose-2-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/proton-dose-2.mha"
  )
plmtest_check_interval ("proton-dose-2-check"
  "${PLM_BUILD_TESTING_DIR}/proton-dose-2-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.10"
  "0.12"
  )
set_tests_properties (proton-dose-2 PROPERTIES DEPENDS rect-17)
set_tests_properties (proton-dose-2 PROPERTIES DEPENDS ptv-1)
set_tests_properties (proton-dose-2-stats
    PROPERTIES DEPENDS proton-dose-2)
set_tests_properties (proton-dose-2-check
    PROPERTIES DEPENDS proton-dose-2-stats)

plm_add_test (
    "proton-dose-3"
    ${PLM_PLASTIMATCH_PATH}/proton_dose
    "${PLM_BUILD_TESTING_DIR}/proton-dose-3.txt"
    )
plm_add_test (
  "proton-dose-3-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/proton-dose-3.mha"
  )
plmtest_check_interval ("proton-dose-3-check"
  "${PLM_BUILD_TESTING_DIR}/proton-dose-3-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.21"
  "0.22"
  )
set_tests_properties (proton-dose-3 PROPERTIES DEPENDS rect-17)
set_tests_properties (proton-dose-3 PROPERTIES DEPENDS ptv-1)
set_tests_properties (proton-dose-3-stats
    PROPERTIES DEPENDS proton-dose-3)
set_tests_properties (proton-dose-3-check
    PROPERTIES DEPENDS proton-dose-3-stats)

## This seems to not work, gives zero dose.
#plm_add_test (
#    "proton-dose-4"
#    ${PLM_PLASTIMATCH_PATH}/proton_dose
#    "${PLM_BUILD_TESTING_DIR}/proton-dose-4.txt"
#    )
#set_tests_properties (proton-dose-4 PROPERTIES DEPENDS rect-18)
#set_tests_properties (proton-dose-4 PROPERTIES DEPENDS rgc-1)

plm_add_test (
    "proton-dose-5a"
    ${PLM_PLASTIMATCH_PATH}/proton_dose
    "${PLM_BUILD_TESTING_DIR}/proton-dose-5a.txt"
    )
plm_add_test (
  "proton-dose-5a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/proton-dose-5a.mha"
  )
plmtest_check_interval ("proton-dose-5a-check"
  "${PLM_BUILD_TESTING_DIR}/proton-dose-5a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.26"
  "0.27"
  )
set_tests_properties (proton-dose-5a PROPERTIES DEPENDS rect-17)
set_tests_properties (proton-dose-5a PROPERTIES DEPENDS ptv-1)
set_tests_properties (proton-dose-5a-stats
    PROPERTIES DEPENDS proton-dose-5a)
set_tests_properties (proton-dose-5a-check
    PROPERTIES DEPENDS proton-dose-5a-stats)

plm_add_test (
    "proton-dose-5g"
    ${PLM_PLASTIMATCH_PATH}/proton_dose
    "${PLM_BUILD_TESTING_DIR}/proton-dose-5g.txt"
    )
plm_add_test (
  "proton-dose-5g-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/proton-dose-5g.mha"
  )
plmtest_check_interval ("proton-dose-5g-check"
  "${PLM_BUILD_TESTING_DIR}/proton-dose-5g-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.365"
  "0.375"
  )
set_tests_properties (proton-dose-5g PROPERTIES DEPENDS rect-17)
set_tests_properties (proton-dose-5g PROPERTIES DEPENDS ptv-1)
set_tests_properties (proton-dose-5g-stats
    PROPERTIES DEPENDS proton-dose-5g)
set_tests_properties (proton-dose-5g-check
    PROPERTIES DEPENDS proton-dose-5g-stats)

plm_add_test (
    "proton-dose-6a"
    ${PLM_PLASTIMATCH_PATH}/proton_dose
    "${PLM_BUILD_TESTING_DIR}/proton-dose-6a.txt"
    )
plm_add_test (
  "proton-dose-6a-stats"
  ${PLM_PLASTIMATCH_PATH}/plastimatch
  "stats;${PLM_BUILD_TESTING_DIR}/proton-dose-6a.mha"
  )
plmtest_check_interval ("proton-dose-6a-check"
  "${PLM_BUILD_TESTING_DIR}/proton-dose-6a-stats.stdout.txt"
  "AVE *([-0-9.]*)"
  "0.26"
  "0.27"
  )
set_tests_properties (proton-dose-6a PROPERTIES DEPENDS rect-17)
set_tests_properties (proton-dose-6a PROPERTIES DEPENDS ptv-1)
set_tests_properties (proton-dose-6a-stats
    PROPERTIES DEPENDS proton-dose-6a)
set_tests_properties (proton-dose-6a-check
    PROPERTIES DEPENDS proton-dose-6a-stats)

## -------------------------------------------------------------------------
## wed-a           ct to proj-wed, proj-ct, and wed-ct
## wed-b           proj-wed to dew-ct
## -------------------------------------------------------------------------
plm_add_test (
  "wed-a"
  ${PLM_PLASTIMATCH_PATH}/wed
  "${PLM_BUILD_TESTING_DIR}/wed-a.txt"
  )
set_tests_properties (wed-a PROPERTIES DEPENDS lung-1)

plm_add_test (
  "wed-b"
  ${PLM_PLASTIMATCH_PATH}/wed
  "${PLM_BUILD_TESTING_DIR}/wed-b.txt"
  )
set_tests_properties (wed-b PROPERTIES DEPENDS lung-1)
set_tests_properties (wed-b PROPERTIES DEPENDS wed-a)

## -------------------------------------------------------------------------
## i hate myself
## -------------------------------------------------------------------------
if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "^(ppc|powerpc)")
  plmtest_debug (debug-fdk-cpu-a
    "${PLM_BUILD_TESTING_DIR}/fdk-cpu-a.stdout.txt"
    "${PLM_BUILD_TESTING_DIR}/fdk-cpu-a.stderr.txt")
  plmtest_debug (debug-fdk-cpu-a-stats
    "${PLM_BUILD_TESTING_DIR}/fdk-cpu-a-stats.stdout.txt"
    "${PLM_BUILD_TESTING_DIR}/fdk-cpu-a-stats.stderr.txt")
  set_tests_properties (debug-fdk-cpu-a PROPERTIES 
    DEPENDS fdk-cpu-a)
  set_tests_properties (debug-fdk-cpu-a-stats PROPERTIES 
    DEPENDS fdk-cpu-a-stats)

  plmtest_debug (debug-landmark-warp-b
    "${PLM_BUILD_TESTING_DIR}/landmark-warp-b.stdout.txt"
    "${PLM_BUILD_TESTING_DIR}/landmark-warp-b.stderr.txt")
  plmtest_debug (debug-landmark-warp-b-stats
    "${PLM_BUILD_TESTING_DIR}/landmark-warp-b-stats.stdout.txt"
    "${PLM_BUILD_TESTING_DIR}/landmark-warp-b-stats.stderr.txt")
  set_tests_properties (debug-landmark-warp-b PROPERTIES 
    DEPENDS landmark-warp-b)
  set_tests_properties (debug-landmark-warp-b-stats PROPERTIES 
    DEPENDS landmark-warp-b-stats)

  plmtest_debug (debug-plm-bsp-char-output
    "${PLM_BUILD_TESTING_DIR}/plm-bsp-char-output.stdout.txt"
    "${PLM_BUILD_TESTING_DIR}/plm-bsp-char-output.stderr.txt")
  plmtest_debug (debug-plm-bsp-char-output-stats
    "${PLM_BUILD_TESTING_DIR}/plm-bsp-char-output-stats.stdout.txt"
    "${PLM_BUILD_TESTING_DIR}/plm-bsp-char-output-stats.stderr.txt")
  set_tests_properties (debug-plm-bsp-char-output PROPERTIES 
    DEPENDS plm-bsp-char-output)
  set_tests_properties (debug-plm-bsp-char-output-stats PROPERTIES 
    DEPENDS plm-bsp-char-output-stats)
endif ()
