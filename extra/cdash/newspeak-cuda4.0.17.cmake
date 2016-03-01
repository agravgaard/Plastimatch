SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/src/plastimatch-nightly")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly-4.0.17")
SET (CTEST_CMAKE_COMMAND "/usr/local/bin/cmake -Wno-dev")
SET (CTEST_COMMAND "/usr/local/bin/ctest -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Unix Makefiles

//Name of the build
BUILDNAME:STRING=lin64-Pisr-Cd-gcc4.4.3-cuda4.0.14

//Name of the computer/site where compile is being run
SITE:STRING=newspeak_2

//Build with shared libraries.
BUILD_SHARED_LIBS:BOOL=OFF

//PLM_CONFIG_DISABLE_REG23:BOOL=ON
")
