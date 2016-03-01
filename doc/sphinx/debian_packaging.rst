Debian packaging
================
This section describes the recommended build configuration for 
building and packaging the official source tarballs 
and debian packages.

Setting up a build system for the first time
--------------------------------------------
#. Set DEBEMAIL and DEBFULLNAME environment variables (see http://www.debian.org/doc/manuals/maint-guide/first.en.html)

#. Install the requisite packages::

     sudo apt-get install devscripts pbuilder debhelper gcc-5 git-buildpackage

   Note: devscripts must be 2.14.2 or higher, and gcc must be 5.0 
   or higher.  To set up gcc, you might need to do something like this:

      http://lektiondestages.blogspot.com/2013/05/installing-and-switching-gccg-versions.html

#. Make and register ssh keys::

     ssh-keygen -f ~/.ssh/id_rsa_alioth

   Be sure to set up your ~/.ssh/config file to tell it where to find the key::

     # Add this to ~/.ssh/config
     Host *.debian.org
             IdentityFile ~/.ssh/id_rsa_alioth

   Then go to https://alioth.debian.org/account/editsshkeys.php to register the public key.  Wait up to one hour for the key to be registered.

#. Download the relevant directory from the debian-med repository::

     debcheckout --git-track='*' --user <username> git://git.debian.org/debian-med/plastimatch.git

   There may be other ways to do this, such as::

     gbp clone ssh://<username>@git.debian.org/git/debian-med/plastimatch.git

   What is the difference between the above?

#. Link the helper scripts to the debian plastimatch directory::

     # This may not be needed any longer
     cd debian-med/plastimatch
     ln -s ~/work/plastimatch/extra/debian/* .

#. Initial setup of pbuilder environment::

     sudo apt-get install debian-archive-keyring
     sudo pbuilder create --distribution sid --mirror ftp://ftp.us.debian.org/debian/ --debootstrapopts "--keyring=/usr/share/keyrings/debian-archive-keyring.gpg"

   See this link for an explanation https://wiki.ubuntu.com/PbuilderHowto, 
   but use the sid distribution instead of squeeze.

#. Initial setup of pbuilder environment::

     sudo apt-get install debian-archive-keyring
     git-pbuilder create

   See this link for more information https://wiki.debian.org/git-pbuilder


Step 1: Preliminary testing
---------------------------
The preliminary testing is performed to make sure that the upstream 
tarball has everything it needs.

#. Run gbp buildpackage to create the dsc::

     gbp buildpackage --git-ignore-new -uc -us -j16

   All the junk that gbp buildpackage makes, such as the orig.tar.gz and the 
   dsc file, gets put in the parent directory.

#. If you want to clean the git directory, you can run::

     debuild clean

#. Test with pbuilder::

     gbp buildpackage --git-pbuilder --git-ignore-new -j16


Step 1: Preliminary testing (obsolete version)
----------------------------------------------
The preliminary testing is performed to make sure that the upstream 
tarball has everything it needs.

#. Update changelog (in an terminal, not emacs)::

     cd plastimatch
     dch -v 1.5.4+dfsg-1

#. Run rebundle.pl until satisfied::

     rebundle.pl

#. Refresh your pbuilder environment (if needed)::

     sudo pbuilder --clean && sudo pbuilder --update

#. Test out by running debuild::

     run_debuild.pl

#. Test out again by running pbuilder::

     run_pbuilder.pl

#. Test parallel regression tests::

      cd ~/build/plastimatch-3.20.0
      ctest -j 4

Step 2: Build the tarball
-------------------------
#. Make sure the changelog is up-to-date
#. Update source into plastimatch-pristene
#. Run make package_source
#. Unpack and test tarball (don't skip this step)
#. Reboot and test tarball on windows (don't skip this step)
#. Upload to web site

Then, do a few small things to get ready for next time

#. Add version number and date to changelog.  This is found in::

     ~/build/plastimatch-pristene/extra_stuff

#. Bump version number in CMakeLists
#. Bump version number in doc/sphinx/conf.py

Step 3: Build the debian package
--------------------------------
#. Commit changes to debian files

#. Clean up files from previous version::

     ./clean_directory.sh

#. Repackage tarball::

     cd trunk
     uscan --verbose --force-download

#. Test out by running debuild::

     run_debuild.pl

#. Test out again by running pbuilder::

     run_pbuilder.pl

Various hints
-------------

Switching between git branches
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Like this::
 git checkout pristine-tar
 git checkout upstream
 git checkout master


Rebuilding an existing debian source package
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Like this::

 apt-get source foo
 cd foo-0.0.1
 sudo apt-get build-dep foo
 debuild -i -us -uc -b

See: https://wiki.debian.org/HowToPackageForDebian
