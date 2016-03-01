Getting started
===============

Getting help
------------

Don't panic!

If you have problems downloading or installing plastimatch, 
please send an email to our mailing list.  We're friendly people.

  http://groups.google.com/group/plastimatch

Downloading the code
--------------------

The recommended method for downloading plastimatch is to use subversion
to download the source code, and then compile the source.
To download using subversion, use the following command::

  $ svn co https://forge.abcd.harvard.edu/svn/plastimatch/plastimatch/trunk plastimatch

You will need to supply the user name and password::

  User: anonymous
  Password: <empty>

If you are using Windows, you will need a subversion client.  
We recommend cygwin (http://cygwin.com) for command-line users, 
and TortoiseSVN (http://tortoisesvn.tigris.org) for graphical users.

If you have already downloaded a previous version, 
you can update to the latest version by executing the following command 
from within the plastimatch source directory::

  $ svn update

Build dependencies
------------------

Debian installs
^^^^^^^^^^^^^^^
The following will get you started on a debian system::

  sudo apt-get install cmake-curses-gui g++ make libdcmtk2-dev libinsighttoolkit4-dev libpng12-dev subversion zlib1g-dev


Cmake (required)
^^^^^^^^^^^^^^^^
Plastimatch uses cmake, so you must download and install cmake 
before you can build plastimatch.  Download from here:

  http://cmake.org/

Cmake 2.6 or higher is required.  Cmake 2.8 is required if you 
want to compile reg-2-3.

C/C++ Compiler (required)
^^^^^^^^^^^^^^^^^^^^^^^^^
You will need a C/C++ compiler.  If you are running 
Windows, we recommend Microsoft Visual Studio (Express or Full).
Any version 2008 or higher should be fine.
You can download the Microsoft Visual Studio Express compiler 
from here:

  http://www.microsoft.com/Express/

On windows, you may also use the MinGW compiler.
However, ITK does not support cygwin.

On unix systems, either g++ or clang should be fine.
For ITK 3.20, only g++ is supported.  
The clang compiler will work for newer versions of ITK.

On OSX, you need the Xcode package, and you must also install the 
command line tools.  
If you wish to use g++ instead of clang, do something like 
the following:

  CC=/usr/bin/gcc CXX=/usr/bin/g++ ccmake /path/to/plastimatch/

ITK (required)
^^^^^^^^^^^^^^
ITK is required for the main plastimatch program.  But if you only 
want the DRR and FDK programs, you don't need it.  Get ITK from here:

  http://itk.org/

We currently support version ITK 3.20.1 and greater, 
and ITK 4.1 and greater.
For ITK 4, you will need to install DCMTK if you want DICOM support. ::

  ITK < 3.20.1          Not supported
  ITK 3.20.1            Supported (with caveats)
  ITK 3.20.2            Recommended
  ITK < 4.1             Not supported
  ITK >= 4.1            Recommended (install DCMTK)

ITK 3.20.1 is a decent version, but it has a few bugs 
that cause problems on recent versions of gcc.  
These bugs are fixed in the ITK 3.20.2 maintenance release.  
To get ITK 3.20.2, do the following::

  git clone git://itk.org/ITK.git
  cd ITK
  git checkout -b release-3.20 origin/release-3.20

When you build ITK, the following settings are recommended or required::

  CMAKE_BUILD_TYPE                          Release
  BUILD_EXAMPLES                            OFF
  BUILD_SHARED_LIBS                         (EITHER)
  BUILD_TESTING                             OFF
  ITK_USE_REVIEW                            ON         # Below ITK 4.5
  Module_ITKReview                          ON         # ITK 4.5 and greater
  ITK_USE_OPTIMIZED_REGISTRATION_METHODS    ON         # ITK 3.20.X only

DCMTK (optional)
^^^^^^^^^^^^^^^^
DCMTK is needed for DICOM-RT support with ITK 4.  
The supported version is 3.6.  On linux, feel free to 
use the dcmtk that comes from your package manager (that's what I do).

There are special considerations to building dcmtk:

#. PNG, TIFF, and ZLIB are not required
#. Set CMAKE_INSTALL_PREFIX to an install directory of your 
   choice; I use $HOME/build/dcmtk-3.6.0-install
#. On linux x86_64 platforms, you need to add -fPIC to 
   CMAKE_CXX_FLAGS and CMAKE_C_FLAGS
#. On windows, you need to set DCMTK_OVERWRITE_WIN32_COMPILER_FLAGS to OFF
#. After building, you need to install; on linux do "make install", or 
   on Visual Studio build the INSTALL target
#. When you run cmake on plastimatch, set DCMTK_DIR to the install directory


VTK (optional)
^^^^^^^^^^^^^^
VTK is required for compiling reg-2-3, for 2D-3D image registration.  
You don't need VTK if you only need plastimatch.
Get VTK from here:

  http://vtk.org/

Only VTK version 5.6.1 is supported.  On linux x86_64 platforms, 
you will need to adjust the compile flags, and add "-fPIC" to 
CMAKE_CXX_FLAGS and CMAKE_C_FLAGS.  

In addition, VTK 5.6.1 has a small bug which prevents it from compiling 
on gcc version 4.6.  You will need to edit the VTK source code.  
Specifically, you need to 
edit the file 
Utilities/vtkmetaio/metaUtils.cxx, and add the following line
somewhere near the top of the file (for example after line 20)::

  #include <cstddef>

CUDA (optional)
^^^^^^^^^^^^^^^
CUDA is needed if you want GPU acceleration of the DRR, FDK, and B-Spline 
registration codes.  
You need to install the driver and toolkit, but the SDK is not needed.

Please note that CUDA is constantly evolving in order to provide new
high performance computing features. 
The following table will help you with selecting the
correct CUDA version to install/upgrade::

  CUDA 2.X              Not supported
  CUDA 3.X              Supported
  CUDA 4.X              Supported
  CUDA 5.0              Supported, Required for Kepler

Download CUDA from here:

  http://developer.nvidia.com/object/cuda_archive.html

FFTW (optional)
^^^^^^^^^^^^^^^
The FFTW library is used to implement the ramp filter for FDK 
cone-beam reconstruction.  So if you are not using the FDK code, 
you don't need this.  We recommend the most current version of FFTW 3.

  http://www.fftw.org/

On windows, the precompiled DLLs work fine.  
However, you do need to create the import libraries.  
See this page for details:

  http://www.fftw.org/install/windows.html  

WxWidgets (optional)
^^^^^^^^^^^^^^^^^^^^
WxWidgets is needed if you want to build "Mondoshot", the dicom screen 
capture program.  Download WxWidgets from here:

  http://wxwidgets.org

Compiling plastimatch (Windows)
-------------------------------
Before compiling plastimatch, compile or install the desired 
prerequisites.  At a minimum, you must compile required 
packages such as ITK.  Be sure to build ITK and plastimatch 
using the same build type (e.g. both as Debug, or both as Release).

Run CMake as follows:

#. Select source directory and binary directory
#. Click configure
#. Select makefile format (e.g. MS VC 2005)
#. <CMake configures>
#. Set the ITK directory (sometimes it might be found automatically)
#. Set directories for optional components (such as slicer)
#. Click configure
#. <CMake configures>
#. Click OK
#. <CMake generates>

Then build in Visual Studio as follows:

#. Navigate to your binary directory
#. Open the project file plastimatch.sln into MSVC.  
#. Change the build type (e.g. release, debug) to match ITK (and other 
   dependencies.  You probably want release.
#. Click "Build Solution".  Let the project build.

Special instructions for running cmake with MSYS/gcc on Windows
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
There is a trick to building with MSYS/gcc.  
The trick is that you need to run the win32 cmake from 
the MSYS command line instead of the GUI.  For example, here is 
the command that I use::

   $ mkdir /c/gcs6/build/plastimatch-mingw
   $ cd /c/gcs6/build/plastimatch-mingw
   $ /c/Program\ Files/CMake\ 2.8/bin/cmake \
       -DITK_DIR=/c/gcs6/build/itk-mingw \
       -G"MSYS Makefiles" \
       /c/gcs6/projects/plastimatch

Then, edit CMakeCache.txt to set your options.  Re-run cmake 
to create the MSYS Makefile, and then run make to build.

Special instructions for Visual Studio 2010
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The CUDA compiler nvcc is not compatible with Visual Studio 2010.
That is why we use Visual Studo 2008.  But, if you 
insist on using VS 2010, there are some workarounds
(Google is your friend).

Compiling plastimatch (Unix)
----------------------------

Build plastimatch as follows:

#. mkdir /path/to/build/files; cd /path/to/build/files
#. ccmake /path/to/source/files
#. Type "c" to configure
#. <CMake configures>
#. Set the ITK directory (it may be found automatically)
#. Set directories for other optional components (if necessary)
#. Type "c" to configure
#. <CMake configures>
#. Type "g" to generate
#. <CMake generates>
#. Type "make"

Users with multicore systems can speed up the process of compiling
plastimatch considerably by invoking make with the -j option.  For
example, a user with a dual-core system would type:

   make -j 2

whereas a user with a quad-core system would type:

   make -j 4

You can probably get even better performance by increasing the 
the number of processes (specified by the -j option) 
beyond the number of cores.  One rule of thumb is to 
use approximately 1.5 times the number of available CPUs (see 
`[1] <http://developers.sun.com/solaris/articles/parallel_make.html#3>`_,
`[2] <http://stackoverflow.com/questions/414714/compiling-with-g-using-multiple-cores>`_).

Compiling the 3D Slicer extensions
----------------------------------
The 3D Slicer extension is now included in SlicerRT.  Please see 
the developer instructions on the SlicerRT assembla page for 
detailed instructions.

https://www.assembla.com/spaces/slicerrt/wiki/SlicerRt_developers_page

