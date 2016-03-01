#! /usr/bin/perl
use Getopt::Long;
use File::Basename;

sub sort_file {
    my ($infile) = @_;

    $outfile = $infile;
    $outfile =~ s/\.txt$/-sorted\.txt/;

    open (FH, "<", $infile);
    open (OFH, ">", $outfile);
    while (<FH>) {
	chomp;
	($label,$intensity,$_) = split (' ', $_, 3);

	# Remove feature numbers
	$_ =~ s/(\d+)://g;

	print OFH "$label $intensity";
	undef %dhash;
	while (($a,$b,$_) = split (' ', $_, 3)) {
	    $dhash{$b} = $a;
	}

	@dhash_keys = sort { $dhash{$a} <=> $dhash{$b} } keys %dhash;

	$i = 2;
	for (@dhash_keys) {
	    print OFH " $i:$_";
	    $i++;
	    print OFH " $i:$dhash{$_}";
	    $i++;
	}
	print OFH "\n";
    }
    close FH;
    close OFH;
}

opendir DIR, ".";
while (my $f = readdir(DIR)) {
    ($f eq "." || $f eq "..") and next;
    ($f =~ /^0522.*\.txt/) or next;
    ($f =~ /-sorted\.txt$/) and next;
    print "Processing: $f\n";
    sort_file ($f);
}
closedir DIR;
