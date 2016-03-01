#!/usr/bin/perl
# Special thanks to perlmonks
# http://www.perlmonks.org/?node_id=136482
# http://www.perlmonks.org/?node_id=584507

# Not quite finished.  Need to do the following:
#  -> Choose which fiducials to use
#  -> Make libsvm format
#  -> Figure out single or double output learning

use File::Copy;
use File::Find;
use File::Path;
use File::Spec;
use File::Spec::Functions;

$dropbox_dir = "$ENV{HOME}/Dropbox";
$indir = "$dropbox_dir/autolabel/gold";
$outdir = "$ENV{HOME}/scratch/autolabel/t_spine_xy";

# Do it!
mkpath $outdir;
process_recursive ($indir);

sub parse_fcsv {
    ($fn) = @_;
    @locs = ();

    open FP, "<$fn";
    while (<FP>) {
	next if /^#/;
	chomp;
	($label,$x,$y,$z,$rest) = split (/,/, $_, 5);
	# Convert RAS to LPS
	$x = -$x;
	$y = -$y;
	push @locs, "$label,$x,$y,$z";
    }
    close FP;

    return @locs;
}

sub process_file {
    my $fn = shift;
    return if (not $fn =~ /\.fcsv$/);

    my $in_base = $fn;
    $in_base =~ s/\.fcsv$//;
    my $in_nrrd = "${in_base}.nrrd";
    return if (! -f $in_nrrd);

    ($_, $_, $base) = File::Spec->splitpath ($fn);
    $base =~ s/\.fcsv$//;

    my $out_libsvm = catfile ($outdir, "${base}_libsvm.txt");
    open OF, ">$out_libsvm";

    @locs = parse_fcsv ($fn);
    for $loc (@locs) {
	($label,$x,$y,$z) = split (/,/, $loc, 4);

	$out_mhd = catfile ($outdir, "${base}_${label}.mhd");
	$out_raw = catfile ($outdir, "${base}_${label}.raw");
	$cmd = "plastimatch thumbnail "
	  . "--input \"$in_nrrd\" --output \"$out_mhd\" "
	    . "--thumbnail-dim 16 --thumbnail-spacing 25.0 --slice-loc $z";
	print "$cmd\n";
	print `$cmd`;

	open FH, "<$out_raw";
	binmode FH;
	print OF "$x $y ";
	while (read (FH, $buf, 4)) {
	    print OF unpack ('f', $buf) . " ";
	}
	close FH;
	print OF "\n";
    }
    close OF;
    exit;
}

sub process_recursive {
    my $path = shift;

    # Get list of files, not including ".", ".."
    opendir (DIR, $path) or
      die "Can't open directory $path: $!\n";
    my @files = grep { !/^\.{1,2}$/ } readdir (DIR);
    closedir (DIR);

    # Change @files from relative to absolute path
    @files = map { $path . '/' . $_ } @files;

    for (@files) {
	if (-d $_) {
	    # Recurse on directory
            process_recursive ($_);
	} else {
	    # Process file
	    process_file ($_);
	}
    }
}
