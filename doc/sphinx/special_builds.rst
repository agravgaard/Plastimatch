Special build instructions
==========================

SlicerRT embedded build
-----------------------
#. Build plastimatch in reduced mode with special flags::

    cmake \
     -DBUILD_SHARED_LIBS:BOOL=OFF \
     -DBUILD_TESTING:BOOL=OFF \
     -DPLM_CONFIG_INSTALL_LIBRARIES:BOOL=ON \
     -DPLM_CONFIG_DISABLE_CUDA:BOOL=ON \
     -DPLM_CONFIG_LIBRARY_BUILD:BOOL=ON \
     -DPLMLIB_CONFIG_ENABLE_REGISTER:BOOL=TRUE \
     -DPLMLIB_CONFIG_ENABLE_DOSE:BOOL=TRUE \
     -DDCMTK_DIR:STRING=/path/to/Slicer-build/DCMTK-build \
     -DITK_DIR:STRING=/path/to/Slicer-build/ITKv4-build \
     /path/to/plastimatch-source

#. Then, when building SlicerRT, tell it where the plastimatch 
   build is located::

    cmake \
     -DSLICERRT_ENABLE_EXPERIMENTAL_MODULES:BOOL=TRUE \
     -DSlicer_DIR:STRING=/path/to/Slicer-build/Slicer-build \
     -DPlastimatch_DIR:STRING=/path/to/plastimatch-build \
     /path/to/SlicerRT-source

Coverage (gcov) build
---------------------
#. Build plastimatch in debug mode, with special flags::

    CMAKE_BUILD_TYPE:STRING=Debug
    CMAKE_CXX_FLAGS:STRING=-fprofile-arcs -ftest-coverage

#. To run standalone, go to the build directory, and run::

    ./run_lcov.sh

#. To run an entire test suite, do this::

    ctest -D NightlyCoverage

Profile (gprof) build
---------------------
#. Set CMAKE_BUILD_TYPE to "Debug"

#. Set CMAKE_C_FLAGS to "-pg"

#. Set CMAKE_CXX_FLAGS to "-pg"

#. Set CMAKE_EXE_LINKER_FLAGS to "-pg"

#. Run "program args"

#. Run "gprof program"


Memcheck (valgrind) build
-------------------------
#. Build ITK in debug mode (optional, but recommended).

#. Build plastimatch in debug mode.

#. To run an individual test case, do this::

    valgrind --leak-check=yes plastimatch [options]

#. To run an entire test suite, normally, you would do something like this::

    ctest -D ExperimentalMemCheck

   However, this doesn't work, because ctest prepends valgrind directly
   to the test command, which in our case is a script.  
