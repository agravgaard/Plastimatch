#!/usr/bin/perl

$indir = "/PHShome/gcs6/build/conquest-1.4.17/data";
$outdir = ".";

opendir (my $dh, $indir) || die "Can't open file $indir";
while (readdir $dh) {
    m/0522/ or next;
    $cmd = "plastimatch convert --input \"$indir/$_\" --output-img \"$outdir/$_/img.nrrd\" --output-prefix \"$outdir/$_/structures\" --prefix-format nrrd";
    print "$cmd\n";
    system ("$cmd");
    
}
closedir $dh;
