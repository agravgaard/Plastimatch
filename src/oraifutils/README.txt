ORAIFUTILS (Version 1.1.0 - May 2011)
--------------------------------------------------------

The open-radART utilities library provides:
 * Image access (ITK-formats, VTK-formats, RTI, zlib-based XRAW-IO, 
   ORA-XML, type independent representation)
 * Structure access (3D to 2D contours, radART structure set to VTK)
 * 2D/3D n-way registration framework with GLSL DRR engine, stochastic rank 
   correlation metric) 
 * Image tools (utility filters)
 * Common tools (INI-file access, logging, sting manipulation, debugging)
 * GUI components


Binaries are available at: TODO


Release Notes:
--------------
* The GLSL DRR engine only supports n-vidia graphic cards.


Operating Systems:
------------------
 * MS Windows XP / Vista / 7 (MinGW, MSVC9)
 * Ubuntu / Kubuntu (9.10 Karmic Koala, 10.04.x Lucid Lynx)
 * Debian (5.0 Lenny) 


Dependencies:
-------------
 * ORAIFUTILS depends on a set of external open source libraries and tools.
 * Note that the library / tool versions are obligatory! 
1) ITK 3.20.0 - The Insight Segmentation and Registration Toolkit
   available here: http://itk.org/ITK/resources/software.html
2) VTK 5.6.1 - The Visualization Toolkit
   available here: http://vtk.org/VTK/resources/software.html
3) [optional] Qt SDK 2010.04 (Qt 4.6.3) Nokia Qt UI Framework
   to build the GUI components and some tools
   available here: http://download.qt.nokia.com/qtsdk/


Build Notes:
------------
  NOTE: We always use out-of-source-builds 
  (source directory and build directory are separated on the same file hierarchy-level, 
  see http://www.paraview.org/Wiki/Eclipse_CDT4_Generator#Out-Of-Source_Builds)!


Windows with MinGW:
-------------------
Required:
 * MinGW (with GCC 4.4.x) - Minimalist GNU for Windows
   available here: http://www.mingw.org/ or as part of Qt SDK
 * CMake 2.8.x - Cross-platform build tool CMake
   available here: http://www.cmake.org/cmake/resources/software.html
Building:
1) Download and install MinGW or use the internal MinGW-package from the Qt SDK
   1.1) Add the MINGW/bin (or the qtdir/mingw/bin) directory to the Path 
        environment variable.
   1.2) Create the environment variable LANG with value 'en' (otherwise the 
        Eclipse command-line parser recognizes warnings as errors and to force 
        the c++ locale to en)
2) Download and install CMake
3) Download and install Qt SDK Framework [optional]
4) Download ITK, configure an appropriate out-of-source-build (CMake) and 
   build it.
   NOTE: Check that ONLY CMAKE_USE_WIN32_THREADS is set. Disable 
   CMAKE_USE_PTHREADS if it is enabled too!
5) Download VTK, configure an appropriate out-of-source-build (CMake) and 
   build it with the following changes:
   * VTK_USE_QT=ON if Qt is used too
   NOTE: Check that ONLY CMAKE_USE_WIN32_THREADS is set. Disable 
   CMAKE_USE_PTHREADS if it is enabled too! 
6) Download the ORAIFUTILS source code
7) Configure an appropriate out-of-source-build (CMake) and link against the 
   previously built libraries


Windows with MSVC9 - Microsoft Visual C++ 2008:
-----------------------------------------------
Required:
 * CMake 2.8.x - Cross-platform build tool CMake
   available here: http://www.cmake.org/cmake/resources/software.html
 * MSVC9 - Microsoft Visual Studio C++ 2008
   available here: http://www.microsoft.com/downloads/en/details.aspx?FamilyID=27673c47-b3b5-4c67-bd99-84e525b5ce61
Building:
1) Download and install Visual Studio 2008
2) Download and install CMake
3) Download and install Qt SDK Framework [optional]
4) Download ITK, configure an appropriate out-of-source-build (CMake) and 
   build it.
5) Download VTK, configure an appropriate out-of-source-build (CMake) and 
   build it with the following changes:
   * VTK_USE_QT=ON if Qt is used too
6) Download the ORAIFUTILS source code
7) Configure an appropriate out-of-source-build (CMake) and link against the 
   previously built libraries


Linux:
------
Required:
 * CMake 2.8.x - Cross-platform build tool CMake
   apt-get install cmake
Building:
1) Download and install GCC
   1.1) Create the environment variable LANG with value 'en' (otherwise the 
        Eclipse command-line parser recognizes warnings as errors and to force 
        the c++ locale to en)
2) Download and install CMake
3) Download and install Qt SDK Framework [optional]
4) Download ITK, configure an appropriate out-of-source-build (CMake) and 
   build it.
5) Download VTK, configure an appropriate out-of-source-build (CMake) and 
   build it with the following changes:
   * VTK_USE_QT=ON if Qt is used too
6) Download the ORAIFUTILS source code
7) Configure an appropriate out-of-source-build (CMake) and link against the 
   previously built libraries

