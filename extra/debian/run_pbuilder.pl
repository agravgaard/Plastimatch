#! /usr/bin/perl

$pkg = "plastimatch";

# Find version number
$wc = `ls *.dsc | wc -l`;
chomp ($wc);
($wc == 1) || die "Sorry, you need one (and only one) dsc file\n";
$dsc = `ls *.dsc`;
chomp ($dsc);

$cmd = "sudo pbuilder --build $dsc";
print "$cmd\n";
system ("$cmd");
