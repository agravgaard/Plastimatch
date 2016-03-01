#########################################################################
#  Optimo
#  - Run registration for each pair of images in a directory
#########################################################################
#!/usr/bin/perl
use File::Copy;
use File::Find;
use File::Path;
use File::Spec;
use File::Spec::Functions;

$base_dir = "/dosf/spideroak/autolabel/gold/rider-pilot";
$scratch_base = "/dosf/scratch/optimo";

$script_template = <<EODATA
[GLOBAL]
fixed=$base_dir/FIXED
moving=$base_dir/MOVING

#vf_out=SCRATCH/PREFIX-vf.nrrd
xform_out=SCRATCH/PREFIX-xf.txt
#img_out=SCRATCH/PREFIX-img.nrrd

logfile=SCRATCH/PREFIX-log.txt

[STAGE]
xform=translation
optim=rsg
impl=itk
max_its=10
convergence_tol=3
grad_tol=0.1
res=4 4 2

[STAGE]
xform=bspline
optim=OPTIM
impl=plastimatch
max_its=100
grid_spac=100 100 100
res=4 4 2

[STAGE]
xform=bspline
optim=OPTIM
impl=plastimatch
max_its=100
grid_spac=70 70 70
res=2 2 1
EODATA
  ;

#########################################################################
#  Main
#########################################################################

# Clean the scratch directory
rmtree ($scratch_base, {keep_root => 1});

# Look for image pairs to register
$in_base = $base_dir;
opendir (DIR, $in_base) or die "Can't opendir input directory $in_base: $!\n";
my @list = sort readdir(DIR);
for (@list) {
    next if (not /\.nrrd$/);
    ( $prefix = $_ ) =~ s/_.*//;
    if ($file1{$prefix}) {
	$file2{$prefix} = $_;
    } else {
	$file1{$prefix} = $_;
    }
}
closedir (DIR);

@optim_settings = ("nocedal", "steepest");

# For each image pair, create and run a script
for $optim (@optim_settings) {
    while (($prefix, $f2) = each(%file2)) {
	$f1 = $file1{$prefix};

	# Create the script file
	$scratch_dir = catfile ($scratch_base, $optim, $prefix);
	mkpath ($scratch_dir);
	$script = $script_template;
	$script =~ s/FIXED/$f1/g;
	$script =~ s/MOVING/$f2/g;
	$script =~ s/PREFIX/$prefix/g;
	$script =~ s/SCRATCH/$scratch_dir/g;
	$script =~ s/OPTIM/$optim/g;
	$script_fn = catfile ($scratch_dir, "parms.txt");
	open (SCR, ">$script_fn");
	print SCR $script;
	close (SCR);

	# Run the script file
	$cmd = "plastimatch register $script_fn";
	print "$cmd\n";
	system ($cmd);
    }
#    last;
}
