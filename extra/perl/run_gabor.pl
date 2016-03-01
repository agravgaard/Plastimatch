#! /usr/bin/perl

$indir = "/home/gcs6/shared/rtog0522-resampled";
$outdir = "/home/gcs6/shared/rtog0522-features";

opendir (my $dh, "$indir") || die;

while (readdir $dh) {
    m/^0522/ || next;
    for my $i (0 .. 11) {
	$outfile = sprintf "$outdir/$_/gabor_%02d.nrrd", $i;
	if (! -f $outfile) {
	    $cmd = "plastimatch filter --output \"${outfile}\" --pattern gabor --gabor-k-fib \"$i 12\" \"$indir/$_/img.nrrd\"";
	    print "$cmd\n";
	    print `$cmd`;
	}
    }
    for my $i (0 .. 2) {
	$gw = 5 * ($i+1);
	$outfile = sprintf "$outdir/$_/gauss_%02d.nrrd", $gw;
	if (! -f $outfile) {
	    $cmd = "plastimatch filter --output \"${outfile}\" --pattern gauss --gauss-width \"$gw\" \"$indir/$_/img.nrrd\"";
	    print "$cmd\n";
	    print `$cmd`;
	}
    }
}

closedir $dh;
