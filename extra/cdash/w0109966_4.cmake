SET (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/build/nightly/src/plastimatch")
SET (CTEST_BINARY_DIRECTORY "$ENV{HOME}/build/nightly/w0109966_4")
SET (CTEST_CMAKE_COMMAND "c:/Program Files/CMake 2.8/bin/cmake.exe")
SET (CTEST_COMMAND "c:/Program Files/CMake 2.8/bin/ctest.exe -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Visual Studio 9 2008

//Name of the build
BUILDNAME:STRING=w04-wxp32-PisrCd-static

//Name of the computer/site where compile is being run
SITE:STRING=w0109966-vse9

//Build with shared libraries.
BUILD_SHARED_LIBS:BOOL=OFF

//The directory containing ITKConfig.cmake.  This is either the
// root of the build tree, or PREFIX/lib/InsightToolkit for an
// installation.
ITK_DIR:PATH=C:/gcs6/build/vs2008/itk-3.20.1

//The directory containing a CMake configuration file for VTK.
VTK_DIR:PATH=C:/gcs6/build/vs2008/vtk-5.6.1

//Disable reg23 testing (it builds, but doesn't run on this computer)
ORAIFUTILS_BUILD_TESTING:BOOL=OFF
REG23_BUILD_TESTING:BOOL=OFF

//Path to the CVS
CVSCOMMAND:FILEPATH=C:/cygwin/bin/cvs.exe
SVNCOMMAND:FILEPATH=C:/cygwin/bin/svn.exe
")
