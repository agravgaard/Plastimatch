SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/build/nightly/src/plastimatch")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly/icemilk_01/plastimatch")
SET (CTEST_CMAKE_COMMAND "/usr/local/bin/cmake -Wno-dev")
SET (CTEST_COMMAND "/usr/local/bin/ctest -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Unix Makefiles

//Name of the build
//BUILDNAME:STRING=osx10.7.3-Pisr-Cd-gcc4.2-cuda4.1
BUILDNAME:STRING=x01-osx10.7.5-Pisr-cd

//Name of the computer/site where compile is being run
SITE:STRING=icemilk-gcc-4.2.1

//C compiler.
CMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc

//CXX compiler.
CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++

//The directory containing ITKConfig.cmake.  This is either the
// root of the build tree, or PREFIX/lib/InsightToolkit for an
// installation.
ITK_DIR:PATH=/Users/gsharp/build/gcc/itk-3.20.1

//Build with shared libraries.
BUILD_SHARED_LIBS:BOOL=OFF

// Use anonymous checkout
SVN_UPDATE_OPTIONS:STRING=--username anonymous --password \\\"\\\"

//Disable REG-2-3
PLM_CONFIG_DISABLE_REG23:BOOL=ON
")
