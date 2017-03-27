#$ptid = "0031";
#$dir = 'C:\Boston\4dct\lung-0031\dicom\unsorted';

$ptid = "0002";
$dir = 'g:\reality\new-data\0002\unsorted';


sub linear_time ()
{
    ($the_time) = @_;
    $hrs = substr($the_time,0,2);
    $mins = substr($the_time,2,2);
    $secs = substr($the_time,4,2);
    return $hrs*60*60 + $mins*60 + $secs;
}


## MAIN
open FO, ">$ptid.out";
for $file (`ls $dir`) {
    chomp ($file);
    print "Processing $file\n";

    $cmd = "dcmdump $dir\\$file";
#     $ct = `$cmd | grep -i ContentTime`;
#     chomp($ct);
#     $ct =~ s/^.*\[//;
#     $ct =~ s/\].*//;
#     $in = `$cmd | grep -i InstanceNumber`;
#     chomp($in);
#     $in =~ s/^.*\[//;
#     $in =~ s/\].*//;
    open FI, "$cmd|";
    while (<FI>) {
	if (/InstanceNumber/) {
	    chomp;
	    s/^.*\[//;
	    s/\].*//;
	    $in = $_;
	} elsif (/ContentTime/) {
	    chomp;
	    s/^.*\[//;
	    s/\].*//;
	    $ct = $_;
	} elsif (/AcquisitionTime/) {
	    chomp;
	    s/^.*\[//;
	    s/\].*//;
	    $at = $_;
	} elsif (/SeriesTime/) {
	    chomp;
	    s/^.*\[//;
	    s/\].*//;
	    $st = $_;
	} elsif (/SliceLocation/) {
	    chomp;
	    s/^.*\[//;
	    s/\].*//;
	    $sl = $_;
	} elsif (/ExposureTime/) {
	    chomp;
	    s/^.*\[//;
	    s/\].*//;
	    $et = $_;
	    ## Midscan time.  For example:
	    ##(0019,1024) ?? 32\31\36\2e\33\39\39\39\39\34            #  10, 1 Unknown Tag & Data
	} elsif (/^\(0019,1024\)/) {
	    chomp;
	    s/^.*\?\? //;
	    s/ .*//;
	    @hexarr = split(/\\/,$_);
	    $s = "";
	    for $v (@hexarr) {
		if (hex($v) >= hex("30") && hex($v) <= hex("39")) {
		    $v = hex($v) - hex("30");
		    $s = $s . "$v";
		} elsif ($v == "2e") {
		    $s = $s . ".";
		}
	    }
	    $mst = $s;
	}
    }
    $stl = &linear_time($st);
    $atl = &linear_time($at);
    $ctl = &linear_time($ct);
    $atl = $atl - $stl;
    $ctl = $ctl - $stl;
    
    print FO "$file $in $ctl $atl $sl $ct $at $st $et $mst\n";
}
close FO;
