#$id = "0045";
#$basedir = "G:\\reality\\processed-2.4.1";
#$indir = "$basedir\\$id\\mha";

if ($#ARGV < 0) {
    print "$#ARGV\n";
    die "Usage: fix_orientation.pl file [orientation]\n";
}

$fn = shift;
$orientation = shift;
if (!$orientation) {
    $orientation = "RAI";
}

if (length($orientation) != 3) {
    die "Orientation must be length 3\n";
}

print "Parsing $fn\n";
open (FP,"+<$fn") or die "Couldn't open $fn";
$curpos = tell(FP);
while (<FP>) {
    if (/AnatomicalOrientation =/) {
        seek(FP,$curpos,SEEK_SET);
        print FP "AnatomicalOrientation = $orientation";
	print "Replaced orientation.\n";
        break;
    }
    $curpos = tell(FP);
}
close FP;
