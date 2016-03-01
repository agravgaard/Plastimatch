#! /usr/bin/perl

$pkg = "plastimatch";

# Find version number
$wc = `ls *.orig.tar.bz2 | wc -l`;
chomp ($wc);
($wc == 1) || die "Sorry, you need one (and only one) orig.tar.bz2 file\n";
$orig_source = `ls *.orig.tar.bz2`;
chomp ($orig_source);
if ($orig_source =~ /plastimatch_(.*)\.orig\.tar\.bz2/) {
    $deb_ver = "$1";
    $ori_ver = "$1";
    $ori_ver =~ s/\+.*//;
    print "Plastimatch version is $deb_ver ($ori_ver)\n";
} else {
    die "Sorry, couldn't pattern match your orig.tar.bz2 file\n";
}

$src_subdir = "${pkg}-${ori_ver}-Source";

system ("rm -rf ${src_subdir}");
system ("tar xvf ${orig_source}");
chdir ("${src_subdir}");
print "Changed directory to ${src_subdir}\n";
system ("cp -r ../trunk/debian .");
system ("debuild -j4 -i -us -uc -b");
system ("debuild -j4 -i -us -uc -S");
