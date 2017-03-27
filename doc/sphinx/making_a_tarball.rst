.. _making_a_tarball:

Making a tarball
================
This section describes how to create an official packaged version
of plastimatch.

Step 1: Preliminary testing
---------------------------
#. Use tarball described in "Debian packaging" page (need link here)
#. Unpack and test tarball on linux (don't skip this step)
#. Unpack and test tarball on windows (don't skip this step)

Step 2: Marking the version
---------------------------
#. Bump version number in CHANGELOG
#. Bump version number in CMakeLists
#. Bump version number in doc/sphinx/conf.py
#. Regenerate man pages
#. Push to remote
#. Tag version::

     git tag -a "v1.6.5" -m "Version 1.6.5"
     git push origin --tags

Step 3: Making the final version
--------------------------------
#. Download tarball from gitlab.
#. Edit gitlab changelog.
#. Unpack and test tarball on linux (don't skip this step)
#. Unpack and test tarball on windows (don't skip this step)
#. Upload to sourceforge::

     sftp gregsharp,plastimatch@frs.sourceforge.net
     cd /home/pfs/project/p/pl/plastimatch/Source
     put plastimatch-1.6.4.tar.bz2
