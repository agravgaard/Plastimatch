#! /usr/bin/perl
use Cwd;
use Getopt::Long;

my $structure = "";
my $convert_dir = "/PHShome/gcs6/gelato_shared/mabs/pddca/convert";

my $output_float = "";
my $skip_testing = "";
my $skip_training = "";

my $execute = 1;
sub execute_command {
    my ($cmd) = @_;
    print "$cmd\n";
    if ($execute) {
	system ($cmd);
    }
}

my @group_a = 
  (
   "0522c0001",
   "0522c0002",
   "0522c0003",
   "0522c0009",
   "0522c0013",
   "0522c0014",
   "0522c0017",
   "0522c0057",
   "0522c0070",
   "0522c0077",
   "0522c0079"
);

@group_b = 
  (
   "0522c0081",
   "0522c0125",
   "0522c0132",
   "0522c0147",
   "0522c0149",
   "0522c0159",
   "0522c0161",
   "0522c0190",
   "0522c0195",
   "0522c0226",
   "0522c0248"
);

@group_c = 
  (
   "0522c0251",
   "0522c0253",
   "0522c0328",
   "0522c0329",
   "0522c0330",
   "0522c0427",
   "0522c0433",
   "0522c0441",
   "0522c0455",
   "0522c0457",
   "0522c0479",
);

sub classify_group {
    my ($array_ref, $group, $suffix, $vw_file) = @_;
    foreach my $item (@$array_ref) {
	$cmd = "cat ${item}${suffix}.txt | sed -e 's/ / | /' | vw -t -i ${vw_file} --binary -p ${item}_predicted.txt";
	execute_command ($cmd);
	$cmd = "plastimatch ml-convert --mask ${item}_mask.nrrd --input-ml-results ${item}_predicted.txt --output ${item}_predicted.nrrd";
	execute_command ($cmd);
	if ($output_float) {
	    $cmd = "cat ${item}${suffix}.txt | sed -e 's/ / | /' | vw -t -i ${vw_file} -p ${item}_predicted_f.txt";
	    execute_command ($cmd);
	    $cmd = "plastimatch ml-convert --mask ${item}_mask.nrrd --input-ml-results ${item}_predicted_f.txt --output-type float --output ${item}_predicted_f.nrrd";
	execute_command ($cmd);
	}
	$cmd = "plastimatch dice --dice --hausdorff ${convert_dir}/${item}/structures/${structure}.nrrd ${item}_predicted.nrrd";
	print "$cmd\n";
	$dice = -1;
	$hd = -1;
	open (CMD, "$cmd|");
	while (<CMD>) {
	    if (/^DICE:\s*(\d.*)/) {
		$dice = $1;
	    }
	    elsif (/^Hausdorff distance =\s*(\d.*)/) {
		$hd = $1;
	    }
	}
	close CMD;
	open (RESULT, ">>result.csv");
	if ($dice != -1 and $hd != -1) {
	    print RESULT "$item,$dice,$hd\n";
	}
	close RESULT
    }
}

$usage = "run_vw_groups.pl [options]\n";
GetOptions (
    "output-float" => \$output_float,
    "skip-testing" => \$skip_testing,
    "skip-training" => \$skip_training
    ) or die $usage;

$cmd = "zcat group_a-ml-sorted.txt.gz group_b-ml-sorted.txt.gz | shuf | vw -f group_ab.vw --loss_function=hinge --binary";
execute_command ($cmd) if (not $skip_training);

$cmd = "zcat group_a-ml-sorted.txt.gz group_c-ml-sorted.txt.gz | shuf | vw -f group_ac.vw --loss_function=hinge --binary";
execute_command ($cmd) if (not $skip_training);

$cmd = "zcat group_b-ml-sorted.txt.gz group_c-ml-sorted.txt.gz | shuf | vw -f group_bc.vw --loss_function=hinge --binary";
execute_command ($cmd) if (not $skip_training);

# Grab structure from current directory
$structure = getcwd;
$structure =~ s/.*\///;

if (not $skip_testing) {
    unlink "result.csv";
    classify_group (\@group_a, "group_a", "-ml-sorted", "group_bc.vw");
    classify_group (\@group_b, "group_b", "-ml-sorted", "group_ac.vw");
    classify_group (\@group_c, "group_c", "-ml-sorted", "group_ab.vw");
}

