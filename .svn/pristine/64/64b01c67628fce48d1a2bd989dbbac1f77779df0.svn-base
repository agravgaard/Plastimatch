#! /usr/bin/perl

# This gets run on sherbert

$indir = "/home/gcs6/conquest-1.4.17/data";
$outdir = "/home/gcs6/shared/rtog0522-converted";

opendir (my $dh, "$indir") || die;

while (readdir $dh) {
    m/^0522/ || next;
    $cmd = "plastimatch convert --input \"$indir/$_\" --output-img \"$outdir/$_/img.nrrd\" --output-prefix \"$outdir/$_/structures\" --prefix-format nrrd";
    print "$cmd\n";
    print `$cmd`;
}

closedir DIR;
