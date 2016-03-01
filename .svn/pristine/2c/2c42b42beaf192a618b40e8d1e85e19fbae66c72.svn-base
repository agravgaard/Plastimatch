#! /usr/bin/perl
use Getopt::Long;
use File::Basename;

my $display_failures = 0;
my $save_optimization_result = 0;

sub digest_file {
    my ($fn, $reghash, $seghash) = @_;
    open (FP, "<$fn") or die "Couldn't open file $fn for read";
    while (<FP>) {
	$line = $_;
	$target = "";
	$atlas = "";
	$reg = "";
	$structure = "";
	$rho = -1;
	$sigma = -1;
	$minsim = -1;
	$thresh = -1;
	$dice = -1;
	$hd = -1;
	$bhd = -1;
	$hd95 = -1;
	$bhd95 = -1;
	$ahd = -1;
	$abhd = -1;
	while (($val,$_) = split (',',$_,2)) {
	    if ($val =~ /target=(.*)/) {
		$target = $1;
		next;
	    }
	    if ($val =~ /atlas=(.*)/) {
		$atlas = $1;
		next;
	    }
	    if ($val =~ /reg=(.*)/) {
		$reg = $1;
		next;
	    }
	    if ($val =~ /struct=(.*)/) {
		$structure = $1;
		next;
	    }
	    if ($val =~ /rho=(.*)/) {
		$rho = $1;
		next;
	    }
	    if ($val =~ /sigma=(.*)/) {
		$sigma = $1;
		next;
	    }
	    if (($val =~ /minsum=(.*)/) || ($val =~ /minsim=(.*)/)) {
		$minsim = $1;
		next;
	    }
	    if ($val =~ /thresh=(.*)/) {
		$thresh = $1;
		next;
	    }
	    if ($val =~ /dice=(.*)/) {
		$dice = $1;
		next;
	    }
	    if ($val =~ /^hd=(.*)/) {
		$hd = $1;
		next;
	    }
	    if ($val =~ /^bhd=(.*)/) {
		$bhd = $1;
		next;
	    }
	    if ($val =~ /^95hd=(.*)/) {
		$hd95 = $1;
		if ($hd95 > $max_hd) {
		    $hd95 = $max_hd;
		}
		next;
	    }
	    if ($val =~ /^95bhd=(.*)/) {
		$bhd95 = $1;
		next;
	    }
	    if ($val =~ /^ahd=(.*)/) {
		$ahd = $1;
		next;
	    }
	    if ($val =~ /^abhd=(.*)/) {
		$abhd = $1;
		next;
	    }
	}
	
	# For registration evaluation
	$reg_key = "$reg";
	if (exists $reghash->{$reg_key}) {
	    ($num,$dice_sum,$hd95_sum) = split (',', $reghash->{$reg_key});
	    $num++;
	    $dice_sum += $dice;
	    $hd95_sum += $hd95;
	} else {
	    $num = 1;
	    $dice_sum = $dice;
	    $hd95_sum = $hd95;
	}
	$reghash->{$reg_key} = "$num,$dice_sum,$hd95_sum";
	if ($display_failures && $dice < 1.1) {
	    print "$dice $reg $target $atlas $structure\n";
	}
	
	# For segmentation evaluation
	if ($thresh > -1) {
	    $seg_key = "$structure,$rho,$sigma,$minsim,$thresh";
	    if (exists $seghash->{$seg_key}) {
		($num,$dice_sum,$hd95_sum) = split (',', $seghash->{$seg_key});
		$num++;
		$dice_sum += $dice;
		$hd95_sum += $hd95;
	    } else {
		$num = 1;
		$dice_sum = $dice;
		$hd95_sum = $hd95;
	    }
	    $seghash->{$seg_key} = "$num,$dice_sum,$hd95_sum";
	}
	
	# $key_2 = "$target,$rho,$sigma,$minsim,$thresh";
	# if ($structure eq "brainstem") {
	#     $brainstem_hash{$key_2} = $dice;
	# }
	# if ($structure eq "left_eye_ball") {
	#     $left_eye_hash{$key_2} = $dice;
	# }
	# if ($structure eq "left_parotid") {
	#     $left_parotid{$key_2} = $dice;
	# }
    }
}


###########################################################################
##  MAIN
###########################################################################
$usage = "Usage: digest_mabs_stats.pl [options] training_dir\n";

if ($#ARGV < 0) {
    die $usage;
}

GetOptions ("display-failures" => \$display_failures,
	    "save-optimization-result" => \$save_optimization_result
	   )
  or die $usage;

$training_dir = shift;

$max_hd = 200;

%reghash = ();
%seghash = ();

if (-d $training_dir) {
    opendir DIR, $training_dir or die "Can't open \"$training_dir\" for parsing";
    while (my $f = readdir(DIR)) {
	($f eq "." || $f eq "..") and next;
	$fn = "$training_dir/$f";
	if (-f $fn) {
	    if ($f eq "seg_dice.csv") {
		digest_file ($fn, \%reghash, \%seghash);
	    }
	    next;
	}
	$fn = "$training_dir/$f/reg_dice.csv";
	if (-f "$fn") {
	    digest_file ($fn, \%reghash, \%seghash);
	} 
	$fn = "$training_dir/$f/seg_dice.csv";
	if (-f "$fn") {
	    digest_file ($fn, \%reghash, \%seghash);
	} 
    }
    closedir DIR;
} else {
    die "Can't open \"$training_dir\" for parsing";
}

if ($display_failures) {
    exit;
}

$best_reg = "";
$best_reg_score = 0;
foreach $reg (sort keys %reghash) {
    ($num,$dice_sum,$hd95_sum) = split (',', $reghash{$reg});
    $avg_dice = $dice_sum / $num;
    $avg_hd95 = $hd95_sum / $num;
    print "reg: $reg,$avg_dice,$avg_hd95\n";
    if ($avg_dice > $best_reg_score) {
	$best_reg_score = $avg_dice;
	$best_reg = $reg;
    }
}
if ($best_reg ne "") {
    ## Update training file
##    $fn = "$dice_source_dir/optimization_result_reg.txt";
##    open FP, ">$fn";
##    print FP "[OPTIMIZATION_RESULT]\nregistration=$best_reg\n";
##    close FP;
}

if (keys(%seghash) == 0) {
    exit (0);
}

## Find best global values over all structures.
foreach $seg (sort keys %seghash) {
    ($num,$dice_sum,$hd95_sum) = split (',', $seghash{$seg});
#    print "$seg,$seghash{$seg}\n";
    ($struct,$parms) = split (',', $seg, 2);
    if (not exists $seg_dice_hash{$parms}) {
	$seg_dice_hash{$parms} = $dice_sum;
	$seg_hd95_hash{$parms} = $hd95_sum;
	$seg_num{$parms} = $num;
    } else {
	$seg_dice_hash{$parms} += $dice_sum;
	$seg_hd95_hash{$parms} += $hd95_sum;
	$seg_num{$parms} += $num;
    }
}
$best_dice_seg = "";
$best_dice_seg_score = 0;
$best_hd95_seg = "";
$best_hd95_seg_score = 0;
foreach $parms (sort keys %seg_dice_hash) {
    $avg_dice = $seg_dice_hash{$parms} / $seg_num{$parms};
    $avg_hd95 = $seg_hd95_hash{$parms} / $seg_num{$parms};
    if ($avg_dice > $best_dice_seg_score) {
	$best_dice_seg = $parms;
	$best_dice_seg_score = "$avg_dice,$avg_hd95";
    }
    if ($avg_hd95 > $best_hd95_seg_score) {
	$best_hd95_seg = $parms;
	$best_hd95_seg_score = "$avg_dice,$avg_hd95";
    }
}

## Find best values separately for each structure.
undef %seg_dice_hash, %seg_hd95_hash, %seg_num;
foreach $seg (sort keys %seghash)
{
    ($num,$dice_sum,$hd95_sum) = split (',', $seghash{$seg});
    if (not exists $seg_dice_hash{$seg}) {
	$seg_dice_hash{$seg} = $dice_sum;
	$seg_hd95_hash{$seg} = $hd95_sum;
	$seg_num{$seg} = $num;
    } else {
	$seg_dice_hash{$seg} += $dice_sum;
	$seg_hd95_hash{$seg} += $hd95_sum;
	$seg_num{$seg} += $num;
    }
    ($struct,$parms) = split (',', $seg, 2);
    $struct_hash{$struct} = 1;
}

foreach $struct (sort keys %struct_hash)
{
    $best_dice = -1;
    $best_dice_parms = "";
    $best_hd95 = -1;
    $best_hd95_parms = "";
    foreach $seg (sort keys %seghash) {
	($seg_struct,$parms) = split (',', $seg, 2);
	($seg_struct eq $struct) or next;

	$dice = $seg_dice_hash{$seg} / $seg_num{$seg};
	$hd95 = $seg_hd95_hash{$seg} / $seg_num{$seg};
	if ($best_dice < 0 || $dice > $best_dice) {
	    $best_dice = $dice;
	    $best_dice_parms = $parms;
	}
	if ($best_hd95 < 0 || $hd95 < $best_hd95) {
	    $best_hd95 = $hd95;
	    $best_hd95_parms = $parms;
	}
    }
    print "seg (dice): $struct,$best_dice_parms,$best_dice\n";
    print "seg (hd95): $struct,$best_hd95_parms,$best_hd95\n";
    $best_seg_parms{$struct} = $best_dice_parms;
}

## Print the best result
if ($save_optimization_result) {
    $fn = "$training_dir/optimization_result_seg.txt";
    open FP, ">$fn";
    foreach $struct (sort keys %struct_hash) {
	$best_seg = $best_seg_parms{$struct};
	if ($best_seg ne "") {
	    ## Update training file
	    ($rho,$sigma,$minsim,$thresh) = split (',', $best_seg);
	    $output_string = <<EOSTRING
[OPTIMIZATION_RESULT]
structure=$struct
gaussian_weighting_voting_rho=$rho
gaussian_weighting_voting_sigma=$sigma
gaussian_weighting_voting_minsim=$minsim
gaussian_weighting_voting_thresh=$thresh
EOSTRING
	      ;
	    print FP $output_string;
	    print $output_string;
	}
    }
    close FP;
}
