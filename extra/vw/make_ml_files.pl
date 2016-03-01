#! /usr/bin/perl
use Getopt::Long;
use File::Basename;
use File::Path;

my $convert_dir = "";
my $registration = "";
my $structure = "";
my $training_dir = "";
my $execute = "";
my $vw_format = "";

sub execute_command {
    my ($cmd) = @_;
    print "$cmd\n";
    if ($execute) {
	system ($cmd);
    }
}

## -- ##
sub process_one_atlas_structure {
    my ($atlas_id, $atlas_img_file, $atlas_structure_file) = @_;

    # Make mask
    my $mask_dmap_file = "${structure}/${atlas_id}_dmap_mask.nrrd";
    my $mask_file = "${structure}/${atlas_id}_mask.nrrd";
    my $cmd = "plastimatch dmap --output \"${mask_dmap_file}\" --input \"${atlas_structure_file}\"";
    execute_command ($cmd);
    $cmd = "plastimatch threshold --input \"${mask_dmap_file}\" --output \"${mask_file}\" --below 30";
    execute_command ($cmd);

    my $output_format = "--output-format libsvm";
    if ($vw_format) {
	$output_format = "";
    }

    my $outfile = "${structure}/${atlas_id}-ml.txt";
    $cmd = "plastimatch ml-convert --output \"${outfile}\" "
      . "--mask \"${mask_file}\" "
      . "--labelmap $atlas_structure_file ${output_format}";
    execute_command ($cmd);
    $cmd = "plastimatch ml-convert --append \"${outfile}\" "
      . "--mask \"${mask_file}\" ${output_format} $atlas_img_file";
    execute_command ($cmd);

    my $tdir1 = "$training_dir/$atlas_id";
    opendir TDIR1, $tdir1;
    while (my $f = readdir(TDIR1)) {
	($f eq "." || $f eq "..") and next;
	($f eq "segmentations") and next;
	my $tdir2 = "$tdir1/$f";
	(-d "$tdir2") or next;
	my $tdir3 = "";
	if ($registration eq "") {
	    opendir TDIR2, $tdir2;
	    while (my $f = readdir(TDIR2)) {
		($f eq "." || $f eq "..") and next;
		$tdir3 = "$tdir2/$f";
		(-d "$tdir3") or next;
		last;
	    }
	    closedir TDIR2;
	} else {
	    $tdir3 = "$tdir2/$registration";
	}
	(-d "$tdir3") or die "\"$tdir3\" is not a directory\n";
	my $dmap = "$tdir3/dmap_${structure}.nrrd";
	my $warp = "$tdir3/img.nrrd";
	(-f $dmap) or die "Error, file \"$dmap\" not found\n";
	(-f $warp) or die "Error, file \"$warp\" not found\n";
	my $cmd = "plastimatch ml-convert --append ${outfile} "
	  . "--mask ${mask_file} ${output_format} $dmap $warp";
	execute_command ($cmd);
    }
    closedir TDIR1, $atlas_structure_dir;
}

## -- ##
sub process_one_atlas {
    my ($atlas_id, $cdir) = @_;

    my $atlas_img_file = "$cdir/img.nrrd";
    (-f $atlas_img_file) or die "Sorry, \"$atlas_img_file\" not found?";

    my $atlas_structure_dir = "$cdir/structures";
    opendir ASDIR, $atlas_structure_dir
      or die "Can't open \"$atlas_structure_dir\" for parsing";
    while (my $f = readdir(ASDIR)) {
	($f eq "." || $f eq "..") and next;
	($f eq "${structure}.nrrd") or next;
	$f = "$atlas_structure_dir/$f";
	(-f $f) or next;
	process_one_atlas_structure ($atlas_id,$atlas_img_file,$f);
    }
    closedir ASDIR;
}


## Main ##
$usage = "make_ml_files.pl [options]\n";
GetOptions ("convert-dir=s" => \$convert_dir,
	    "training-dir=s" => \$training_dir,
	    "registration=s" => \$registration,
	    "structure=s" => \$structure,
	    "execute" => \$execute,
	    "vw-format" => \$vw_format
	   )
    or die $usage;

(-d $convert_dir) or die "Sorry, convert dir $convert_dir not proper\n" . $usage;
(-d $training_dir) or die "Sorry, training dir $training_dir not proper\n" . $usage;

(-d "$training_dir/mabs-train") and $training_dir = "$training_dir/mabs-train";
($structure eq "") and $structure = "BrainStem";

opendir ADIR, $convert_dir or die "Can't open \"$convert_dir\" for parsing";
while (my $f = readdir(ADIR)) {
    ($f eq "." || $f eq "..") and next;
    $cdir = "$convert_dir/$f";
    (-d $cdir) or next;
    process_one_atlas ($f,$cdir);
#    last;
}
closedir ADIR;
