SET (CTEST_SOURCE_DIRECTORY "D:/src/plastimatch-nightly")
SET (CTEST_BINARY_DIRECTORY "D:/build/plastimatch-nightly-shared")
SET (CTEST_CMAKE_COMMAND "c:/Program Files (x86)/CMake 2.8/bin/cmake.exe")
SET (CTEST_COMMAND "c:/Program Files (x86)/CMake 2.8/bin/ctest.exe -D Nightly")
SET (CTEST_INITIAL_CACHE "
//Name of generator.
CMAKE_GENERATOR:INTERNAL=Visual Studio 8 2005

//Name of the build
BUILDNAME:STRING=win7-Pisr-Cd-vse8-cuda4.1-DLLs

//Name of the computer/site where compile is being run
SITE:STRING=zerowing

//Build without shared libraries.
BUILD_SHARED_LIBS:BOOL=ON

//Used dynamically compiled ITK
ITK_DIR:FILEPATH=D:/build/itk-3.20.1-shared

//Don't Build REG-2-3 for now
PLM_CONFIG_DISABLE_REG23:BOOL=OFF

//Path to the CVS
CVSCOMMAND:FILEPATH=C:/cygwin/bin/cvs.exe
SVNCOMMAND:FILEPATH=C:/cygwin/bin/svn.exe
")
