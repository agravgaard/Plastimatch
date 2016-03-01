Windows packaging
=================
This section describes the recommended build configuration for 
building official plastimatch windows binaries
and binaries.

The Windows build uses the MSVC 2013 express compiler.  
This compiler is capable of both 32-bit and 64-bit targets, 
and also supports OpenMP.

Third party libraries to be used::

  CUDA            3.0.14            (TBD)
  DCMTK           3.6.0             (If mondoshot is built)
  FFTW            3.2.2             (TBD)
  ITK             3.20.1            (TBD)
  wxWidgets       2.8.12            (If mondoshot is built)

Configuration settings::

  BUILD_SHARED                  ON      (this is not default)
  PLM_CONFIG_USE_SS_IMAGE_VEC   ON      (this is default)
  PLM_CUDA_ALL_DEVICES          ON      (this is default)
  PLM_INSTALL_RPATH             OFF     (this is default)

#. Build/install all required 3rd party libraries.
#. Double check CPACK version number (at bottom of CMakeLists.txt)
#. Verify that svn is not modified (i.e. do svn update; svn diff)
#. Build plastimatch (start with a fresh cmake)
#. Run test cases, make sure all pass
#. Build package
#. Test package for missing dlls by making sure plastimatch runs
