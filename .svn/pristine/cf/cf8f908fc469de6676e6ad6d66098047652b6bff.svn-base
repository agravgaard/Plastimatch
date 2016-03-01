SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/build/nightly/src/plastimatch")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly/gelato_08/plastimatch")
SET (CTEST_CMAKE_COMMAND "/usr/local/bin/cmake")
SET (CTEST_COMMAND "/usr/local/bin/ctest -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Unix Makefiles

//Name of the build
BUILDNAME:STRING=08-lin64-PirCd-S4

//Name of the computer/site where compile is being run
SITE:STRING=gelato-gcc4.4.5

//Directory with SlicerConfig.cmake or Slicer3Config.cmake
Slicer_DIR:PATH=/PHShome/gcs6/build/slicer-4/Slicer-build

//Disable REG-2-3
PLM_CONFIG_DISABLE_REG23:BOOL=ON

// Use anonymous checkout
SVN_UPDATE_OPTIONS:STRING=--username anonymous --password \\\"\\\"

//Build with shared libraries.
BUILD_SHARED_LIBS:BOOL=ON
")
