
INTERFACE GOALS 2
-----------------
You are allowed to flip out the search image, the signal.  
Why should you need to know the search size area, as long as your 
score image is big enough, and you re-clip?  

At compile time, specify:
(1) The pattern
(2) The maximum search area size

At run time, specify:
(1) The signal
(2) The search area

How should the search area be specified?  The pixels where the 
score is desired?  The meaning needs to be clear for non-square pixels.


INTERFACE GOALS
---------------
The run() command should not require any malloc calls.  The philosophy 
is to call as follows:

  fancc_compile ()
  do (forever) {
    set pointer to search area
    fancc_run ()
  }
  fancc_free ()

Thus, at pattern compile time you should know the following:
(1) The pattern
(2) The search area size


MAKING USING GCC
----------------
Matlab interface can be built on either using cygwin, cygwin/mingw 
or mingw.  The cygwin/mingw is what I use, because pure cygwin 
causes matlab to crash when cygwin.dll is unloaded and reloaded.

To make for matlab (cygwin):
(1)  Install and configure using gnumex
(2)  Modify the mexopts.bat file as follows:
	set GM_ADD_LIBS=-lpng -ltiff -lz
	set OPTIMFLAGS=-O3 -malign-double -mcpu=pentium
(3)  cd ../matlab
(4)  make (i.e. run the matlab make.m)

To make for matlab (cygwin/mingw):
(1)  Install and configure using gnumex
(1.5) Install various mingw libraries from mingwrep.sf.net
     Be sure to set execute permission on the dll's and add to 
     your path.
(2)  Modify the mexopts.bat file as follows:
	set GM_ADD_LIBS=-Lc:\mingw\lib -lpng -ltiff -lz
	set OPTIMFLAGS=-O3 -malign-double -mcpu=pentium
	set COMPFLAGS=-c -DMATLAB_MEX_FILE -mno-cygwin -Ic:\mingw\include
(3)  cd ../matlab
(4)  make (i.e. run the matlab make.m)


CHECKING DLL'S
--------------
Sometimes it's good to check the dependencies of a DLL.  Here is how 
to do it.  These are more or less equivalent:

  dumpbin /dependents foo.dll     %% msvc
  objdump -p foo.dll              %% cygwin


OPAQUE POINTERS FOR MATLAB
--------------------------
mext1.cpp and mext2.cpp demonstrate how to do the following:

- Wrap an opaque C++ pointer in matlab
- Share the global variable among multiple dll's

In theory this should work for Unix too.


Wrapping
--------
The library needs to be useful for both C and matlab.
mexmatch.cpp is the matlab wrapper, which calls the C interface.
The C interface is in match.c.
