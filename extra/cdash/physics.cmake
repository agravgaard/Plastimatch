SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/build/physics_1/plastimatch")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly/plastimatch")
SET (CTEST_CMAKE_COMMAND "/usr/local/bin/cmake")
SET (CTEST_COMMAND "/usr/local/bin/ctest -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Unix Makefiles

//Name of the build
BUILDNAME:STRING=lin64-Pisr-cd

//Name of the computer/site where compile is being run
SITE:STRING=physics-gcc4.1.2

//The directory containing ITKConfig.cmake.  This is either the
// root of the build tree, or PREFIX/lib/InsightToolkit for an
// installation.
ITK_DIR:PATH=/home/gcs6/build/itk-3.18.0

//Build with shared libraries.
BUILD_SHARED_LIBS:BOOL=ON
")
