SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/build/nightly/src/plastimatch")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly/w0109966_6")
SET (CTEST_CMAKE_COMMAND "c:/Program Files/CMake 2.8/bin/cmake.exe")
SET (CTEST_COMMAND "c:/Program Files/CMake 2.8/bin/ctest.exe -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Visual Studio 9 2008

//Name of the build
BUILDNAME:STRING=w06-wxp32-PisrCd-S4

//Name of the computer/site where compile is being run
SITE:STRING=w0109966-vse9

//Directory with SlicerConfig.cmake or Slicer3Config.cmake
Slicer_DIR:PATH=c:/gcs6/build/slicer-4/Slicer-build

//Disable REG-2-3
PLM_CONFIG_DISABLE_REG23:BOOL=ON

//Path to the CVS
CVSCOMMAND:FILEPATH=C:/cygwin/bin/cvs.exe
SVNCOMMAND:FILEPATH=C:/cygwin/bin/svn.exe
GIT_EXECUTABLE:FILEPATH=C:/cygwin/bin/git.exe
")
