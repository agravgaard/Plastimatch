#! /usr/bin/perl

###### The number at the end of the output directory 
######    is the "StudyID" from the dicom header
###### Find it like this:
######    dcmdump `ls | head -n 1` | grep StudyID
######
######    105	Adv Sim RT Plans
######    103	Adv Sim RT Structure Sets
######    107	Advantage Fusion RT Structure Sets

use File::Copy;
use File::Spec;
use File::Path;

#####################################################################
##     configuration variables
#####################################################################
$move_files = 1;
$want_mip = 1;
$want_min_ip = 1;
$want_ave_ip = 1;
$want_unsorted = 1;
$need_sleep = 0;

##$free_breathing = 3;
$free_breathing = -1;
$mip = -1;
$min_ip = -1;
$ave_ip = -1;
$unsorted = -1;
$s103 = -1;
$s105 = -1;
$s107 = -1;


#####################################################################
##     handle_directory_change
#####################################################################
sub move_or_copy {
    my ($infn,$outfn,$move_files) = @_;
    if ($move_files) {
	move ($infn, $outfn);
    } else {
	`ln -s $infn $outfn`;
    }
}

#####################################################################
##     my_mkdir
#####################################################################
sub my_mkdir {
    my ($dirname) = @_;
    print "Trying to mkdir $dirname\n";
    mkdir $dirname;
    $need_sleep && sleep 4;
}

#####################################################################
##     MAIN
#####################################################################
if ($#ARGV == 1) {
    $indir = $ARGV[0];
    $outdir = $ARGV[1];
    $use_no_phi = 0;
} else {
    die "Usage: conquest_split.pl indir outdir\n";
}

opendir(DIR, $indir);
if ($use_no_phi) {
    @files = grep(/\-no\-phi\.v2$/,readdir(DIR));
} else {
    @files = grep(/.v2$/,readdir(DIR));
    rewinddir DIR;
    @files = (@files, grep(/.dcm$/,readdir(DIR)));
}
closedir(DIR);

$f = File::Spec->catfile($indir, $files[1]);
$studyid = `dcmdump $f | grep StudyID`;
chomp($studyid);
$studyid =~ s/[^[]*\[//;
$studyid =~ s/].*//;

foreach $file (@files) {
    $file =~ /_([^_]*)/;
    $series = $1;
    if (!defined $series_hash{$series}) {
	$series_hash{$series} = 1;
	$f = File::Spec->catfile($indir, $file);
	$series_description = `dcmdump $f | grep \"\(0008,103e\)\"`;
	chomp($series_description);
	$series_description =~ s/[^[]*\[//;
	$series_description =~ s/].*//;
	
	print "$series $series_description\n";
	$series_description_hash{$series} = $series_description;
    } else {
	$series_hash{$series} = $series_hash{$series} + 1;
    }
}

## Use heuristics to identify series
## GCS: These heuristics are good for scans from GE, but only if it 
## was not anonymized on GE.  Descriptions are cleared by GE anon tool.
foreach $k (keys %series_description_hash) {
    if ($series_hash{$k} > 500) {
	$unsorted = $k;
    } 
    elsif ($series_description_hash{$k} =~ /^T=(\d*)\%/) {
	$sno = $1 / 10;
	$sorted[$sno] = $k;
    }
    elsif ($series_description_hash{$k} =~ /contrast/i) {
	$free_breathing = $k;
    }
    elsif ($series_description_hash{$k} =~ /w\//i) {
	$free_breathing = $k;
    }
    elsif ($series_description_hash{$k} =~ /without/i) {
	$free_breathing = $k;
    }
    elsif ($series_description_hash{$k} =~ /esoph/i) {
	$free_breathing = $k;
    }
    elsif ($series_description_hash{$k} =~ /helic/i) {
	$free_breathing = $k;
    }
    elsif ($series_description_hash{$k} =~ /^MIP/) {
	$mip = $k;
    }
    elsif ($series_description_hash{$k} =~ /^Min-IP/) {
	$min_ip = $k;
    }
    elsif ($series_description_hash{$k} =~ /^Ave-IP/) {
	$ave_ip = $k;
    }
    elsif ($k == 103) {
	$s103 = $k;
    }
    elsif ($k == 105) {
	$s105 = $k;
    }
    elsif ($k == 107) {
	$s107 = $k;
    }
}

## Display heuristics results
print "--------------------\n";
$indir =~ /([^\\\/]*)$/;
print "DIR = $1\n";
print "StudyID = $studyid\n";
print "--------------------\n";
print "UNSORTED:       $unsorted\n";
print "SORTED:         ";
foreach (0..9) { print $sorted[$_] . " "; }
print "\n";
print "FREE_BREATHING: $free_breathing\n";
print "MIP:            $mip\n";
print "MIN IP:         $min_ip\n";
print "AVE IP:         $ave_ip\n";
print "STRUCTS:        $s103 $s105 $s107\n";
print "--------------------\n";

## Make parent dir
mkpath($outdir);
$need_sleep && sleep 4;

foreach $series (keys %series_hash) {
    $od = File::Spec->catfile($outdir, $series);
    my_mkdir ($od);
    @move_files = grep(/_${series}_/,@files);
    for $file (@move_files) {
	$f = File::Spec->catfile($indir,$file);
	#print "MV $f $od\n";
	&move_or_copy ($f, $od, $move_files);
    }
}
