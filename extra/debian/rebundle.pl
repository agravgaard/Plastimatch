#! /usr/bin/perl
# This script does this following:
# (1) Make tarball from svn repo
# (2) Copy tarball to debian-med directory
# (3) Run get-orig-source

use Getopt::Long;
use Cwd 'abs_path';

# --working option uses standard dir instead of pristine dir
my $working;
$result = GetOptions (
    "working" => \$working
    );

#$debmed_dir = $ENV{HOME} . "/debian-med/plastimatch";
$debmed_dir = abs_path(".");
my $pristine = 1;
$run_cmake = 1;
$make_tarball = 1;

if ($working) {
    print "NOT using pristine.\n";
    $pristine = 0;
}

if (${pristine}) {
    $src_dir = $ENV{HOME} . "/work/plastimatch-pristine";
    $build_dir = $ENV{HOME} . "/build/plastimatch-pristine";
} else {
    $src_dir = $ENV{HOME} . "/work/plastimatch";
    $build_dir = $ENV{HOME} . "/build/plastimatch";
}

if ($run_cmake) {
    chdir ${src_dir};
    system ("svn update");
    chdir ${build_dir};
    system ("cmake \"${src_dir}\"");
}
if ($make_tarball) {
    chdir ${build_dir};
    system ("rm *.gz");
    system ("rm *.bz2");
    system ("make package_source");
}

chdir ${build_dir};
$input_tarball = `ls *.bz2`;
chomp ($input_tarball);
if ($input_tarball =~ /plastimatch-(.*)-Source.tar.bz2/) {
    $deb_ver = "plastimatch_$1";
    print "Plastimatch version is $1\n";
} else {
    die "Couldn't parse plastimatch version\n";
}
$input_src_dir = $input_tarball;
$input_src_dir =~ s/\.tar\..*$//;
$debmed_input_tarball = "${debmed_dir}/${input_tarball}";
$build_input_tarball = "${build_dir}/${input_tarball}";

print "input_src_dir = $input_src_dir\n";
print "build_input_tarball = $build_input_tarball\n";
print "debmed_input_tarball = $debmed_input_tarball\n";

##########################################################
# There seems to be a bug in mk-origtargz, which causes one of the 
# tar commands to fail unless the tarball is repackaged.
# Therefore, instead of just linking the file like this
##########################################################
  #chdir ${debmed_dir};
  #system ("rm ${debmed_input_tarball} 2> /dev/null");
  #system ("ln -s ${build_input_tarball} ${debmed_input_tarball}");
##########################################################
# We untar, then re-tar, like this
##########################################################
chdir ${debmed_dir};
system ("tar xvf ${build_input_tarball}");
system ("tar czvf ${debmed_input_tarball} ${input_src_dir}");

# Finally, do the actual rebundle
chdir "${debmed_dir}/trunk";
system ("mk-origtargz --repack --compression bzip2 \"${debmed_input_tarball}\"");

chdir ${debmed_dir};
