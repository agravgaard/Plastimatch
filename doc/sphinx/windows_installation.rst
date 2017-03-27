.. _windows_installation:

User manual for plastimatch windows binary
===========================================

1. Download a plastimatch window binary file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * Webpage for downloading: https://sourceforge.net/projects/plastimatch/files/Windows%20Binaries/
 * Download either win64 or win32 according to your windows version.

2. Run the downloaded “plastimatch-O.O.O-winOO.msi” file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * During the installation, specify the installation directory. Default directory may be “C:/Program Files/~”.  To avoid any access permission issue, a user-created folder is recommended.

  .. image:: ../figures/windows_installation_1.png
   :width: 30 %

3. Prepare the command line prompt
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * Browse the plastimatch “bin” folder where all the binary files are installed.
 * Right mouse click at the “launch_cmd_prompt.bat” file and do “Run as administrator”.

  .. image:: ../figures/windows_installation_2.png
   :width: 30 %

 * The plastimatch command prompt will be generated both on your desktop and the plastimatch bin folder.

  .. image:: ../figures/windows_installation_3.png
   :width: 30 %
      
4. Run a sample deformable image registration using a command line
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 * Run the command prompt just created.
 * Type the following command line (OR copy the following line and paste it onto the command prompt using "mouse-right-click and paste")::

	plastimatch register "./sample/command_file_example.txt"

  .. image:: ../figures/windows_installation_4.png
   :width: 30 %

 * A deformable registration will be performed between sample CT and MRI images and the result will be saved in ”~/bin/sample” folder.

  .. image:: ../figures/windows_installation_5.png
   :width: 30 %


5. [Optional] Review images using an image viewer.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  * Since there is no image viewer available inside the plastimatch, users are recommended to use third-party software to see the images generated from plastimatch such as ”.nrrd” and “.mha” files.
  * Recommended software: 3DSlicer
	- Download webpage for win64 users: http://download.slicer.org/
	- Download webpage for win32 users: https://www.slicer.org/slicer3-downloads/Release/win32/
  * Install the 3Dslicer
  * View image files using 3Dslicer
	- Drag&Drop the image files onto the 3Dslicer (available in up-to-date version)
	- Select images for overlay and review them by using the overlay slide bar.

  .. image:: ../figures/windows_installation_6.png
   :width: 30 %

  .. image:: ../figures/windows_installation_7.png
   :width: 30 %

6. Question/Feedback?
^^^^^^^^^^^^^^^^^^^^^
  * Post your questions to the plastimatch community: https://groups.google.com/forum/#!forum/plastimatch
