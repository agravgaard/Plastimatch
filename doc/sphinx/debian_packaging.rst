Debian packaging
================
This section describes the recommended build configuration for 
building and packaging the official source tarballs 
and debian packages.

References:
  
#. http://oar.imag.fr/wiki:debian_packaging
#. https://wiki.debian.org/GridenginePackaging/GitGuide
#. http://linux.lsdev.sil.org/wiki/index.php/Packaging_using_gbp
#. https://wiki.debian.org/Diaspora/Packaging/origsource
#. https://wiki.debian.org/PackagingWithGit
#. http://honk.sigxcpu.org/projects/git-buildpackage/manual-html/gbp.import.html


Setting up a build system for the first time
--------------------------------------------
#. Set DEBEMAIL and DEBFULLNAME environment variables (see http://www.debian.org/doc/manuals/maint-guide/first.en.html)

#. Install the requisite packages::

     sudo apt-get install devscripts pbuilder debhelper git-buildpackage cowbuilder

   Note: if your host is not sid, you might need to install a newer gcc version 
   than exists on the native release.  You can do something like this:

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

     gbp clone --all --pristine-tar ssh://<username>@git.debian.org/git/debian-med/plastimatch.git

   What is the difference between the above?

#. Link the helper scripts to the debian plastimatch directory::

     # This may not be needed any longer
     cd debian-med/plastimatch
     ln -s ~/work/plastimatch/extra/debian/* .

#. Initial setup of pbuilder environment::

     sudo apt-get install debian-archive-keyring
     git-pbuilder create

Install packages into git-pbuilder.  This saves time when running
multiple times::

  git-pbuilder login --save-after-login
  apt-get update
  apt-get install libfftw3-dev libinsighttoolkit4-dev libpng-dev libtiff-dev uuid-dev zlib1g-dev
  
See this link for more information https://wiki.debian.org/git-pbuilder


Step 1: Preliminary testing
---------------------------
The preliminary testing is performed to make sure that the upstream 
tarball has everything it needs.

#. Refresh your git-pbuilder environment (if needed)::

     sudo git-pbuilder --update

#. Test parallel regression tests::

     cd ~/build/plastimatch-3.20.0
     ctest -j 4

#. Update changelog (in an terminal, not emacs)::

     cd plastimatch
     dch -v 1.6.3+dfsg-1
     git commit -a -m "Update changelog"

#. Run gbp import-orig.  This will update your source code from the tarball
   into the directory and local git repository, without pushing these changes
   onto the remote server::

     gbp import-orig --pristine-tar -u 1.6.3+dfsg \
     --filter=doc/*.doc \
     --filter=doc/*.odt \
     --filter=doc/*.pdf \
     --filter=doc/*.ppt \
     --filter=doc/*.txt \
     --filter=doc/figures \
     --filter=doc/man/bspline.7 \
     --filter=doc/man/proton_dose.7 \
     --filter=doc/sphinx \
     --filter=extra \
     --filter=src/fatm \
     --filter=src/ise \
     --filter=src/mondoshot \
     --filter=src/oraifutils \
     --filter=src/reg-2-3 \
     --filter=src/plastimatch/test/opencl_test.* \
     --filter=libs/lua-5.1.4 \
     --filter=libs/libf2c \
     --filter=libs/msinttypes \
     --filter=libs/sqlite-3.6.21 \
     --filter-pristine-tar \
     ~/build/plastimatch-pristine/plastimatch-1.6.3-Source.tar.bz2
   
#. If you make changes and you want to reset your repository, try this::

     git checkout pristine-tar
     git reset --hard origin/pristine-tar --
     git checkout upstream
     git reset --hard origin/upstream --
     git checkout master
     git reset --hard origin/master --
     git tag -d upstream/1.6.3+dfsg

#. Run gbp buildpackage to create the dsc::

     gbp buildpackage --git-ignore-new -uc -us -j16

   If the host does not contain all needed packages you will need to use pbuilder::

     gbp buildpackage --git-pbuilder --git-ignore-new -uc -us -j16
     
   All the junk that gbp buildpackage makes, such as the orig.tar.gz and the 
   dsc file, gets put in the parent directory.

#. If you want to clean the git directory, you can run::

     debuild clean

#. Test with pbuilder::

     gbp buildpackage --git-pbuilder --git-ignore-new -j16

      
Step 2: Build the tarball
-------------------------
Follow instructions in making_a_tarball

Step 3: Build the debian package
--------------------------------
#. Patch git with upstream::

     gbp import-orig --pristine-tar --uscan -u 1.6.3+dfsg

#. Test::

     gbp buildpackage

Do I need --pristine-tar here?
     
#. Push changes to server::

     git push --all --tags origin

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
