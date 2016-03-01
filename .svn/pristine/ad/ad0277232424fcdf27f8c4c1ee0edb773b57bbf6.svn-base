#! /usr/bin/perl

# ref:
#   http://developer.ubuntu.com/packaging/html/packaging-new-software.html

$pkg_dir = $ENV{HOME} . "/pack";
$src_dir = $ENV{HOME} . "/src/plastimatch-ppa";
$build_dir = $ENV{HOME} . "/build/plastimatch-ppa";


########################################################################
##  FUNCTIONS

# just prints infomation regarding what needs to be edited
# in the debian/ directory
sub print_help
{
    print " [1] edit debian/changelog\n";
    print "   Change the version number to an Ubuntu version (ex: 1.5.10-0ubuntu1).\n";
    print "   This means upstream version 1.5.10, Debian version 0, Ubuntu version 1).\n";
    print "   Also change unstable to the Ubuntu release: lucid, mavrick, precise, etc.\n";
    print " [2] edit debian/control\n";
    print " [3] edit debian/copyright\n";
    print " [4] populate debian/docs\n";
    print " [5] deal with README.source and README.Debian\n";
    print " [6] deal with debian/rules (Makefile) - use debhelper\n";
    print "\n";
    print " When done, try building your package via:\n";
    print "   $ debuild -us -uc\n\n";
}

# generates source tarball from ${src_dir} and
# places it into ${build_dir}; returns package version
sub gen_src_tarball
{
    # update svn
    chdir ${src_dir};
    system ("svn update");

    # run cmake
    chdir ${build_dir};
    system ("cmake ${src_dir}");

    # generate source tarball
    chdir ${build_dir};
    system ("rm *.bz2");
    system ("make package_source");
    $source_bz2 = `ls *.bz2`;
    chomp ($source_bz2);

    if ($source_bz2 =~ /plastimatch-(.*)-Source.tar.bz2/) {
        $ver = "$1";
        $deb_ver = "plastimatch_${ver}";
    } else {
        die "Couldn't parse plastimatch version\n";
    }

    return ${ver};
}

sub gen_package
{
    # (re)create symbolic link to source package in build dir
    system ("rm ${pkg_bz2} 2> /dev/null");
    system ("ln -s ${src_bz2} ${pkg_bz2}");

    # remove packaging files if they already exists
    system ("rm -rf ${pkg_dir}/plastimatch");
    system ("rm ${pkg_tgz}");

    # create the packaging files
    chdir ${pkg_dir};
    system ("bzr dh-make plastimatch ${ver} ${pkg_bz2}");
    system ("rm ${pkg_bz2}");
    print "\n";

    # enter the debian directory and rm stuff we don't need
    system ("cd ${pkg_dir}/plastimatch/debian");
    chdir ${pkg_dir} . "/plastimatch/debian";
    system ("rm *ex *EX");
}


########################################################################
##  BEGIN

$ver = gen_src_tarball();
print "Plastimatch version is ${ver}\n";
$src_bz2 = "${build_dir}/plastimatch-${ver}-Source.tar.bz2";
$pkg_bz2 = "${pkg_dir}/plastimatch_${ver}.orig.tar.bz2";
$pkg_tgz = "${pkg_dir}/plastimatch_${ver}.orig.tar.gz";

gen_package();
print_help();
