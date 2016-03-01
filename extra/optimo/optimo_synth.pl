#########################################################################
#  Optimo synth
#  - Generate synthetic vector fields
#  - Run registration
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
fixed=FIXED
moving=MOVING
vf_out=VF_OUT
xform_out=XF_OUT
img_out=IMG_OUT
logfile=LOGFILE

[STAGE]
xform=bspline
optim=lbfgsb
impl=plastimatch
max_its=100
grid_spac=100 100 100
res=4 4 2

[STAGE]
xform=bspline
optim=lbfgsb
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
#File::Path::remove_tree ($scratch_base, {keep_root => 1});
File::Path::remove_tree ('/dosf/scratch/optimo');

# Look for images to register
$in_base = $base_dir;
opendir (DIR, $in_base) or die "Can't opendir input directory $in_base: $!\n";
my @nrrd_list = ();
my @list = sort readdir(DIR);
for (@list) {
    next if (not /\.nrrd$/);
    push @nrrd_list, catfile($in_base,$_);
}
closedir (DIR);

$subdir = 0;
for $nrrd (@nrrd_list) {
    print "$nrrd\n";
    # Parse header
    $cmd = "plastimatch header $nrrd";
    open PH, "$cmd|";
    while (<PH>) {
	if (/Origin = (.*)$/) {
	    $origin = $1;
	}
	if (/Size = (.*)$/) {
	    $size = $1;
	}
	if (/Spacing = (.*)$/) {
	    $spacing = $1;
	}
    }

    # Compute image center
    @ori = split ' ', $origin;
    @spa = split ' ', $spacing;
    @dim = split ' ', $size;
    for $i (0..2) {
	$ctr[$i] = $ori[$i] + $spa[$i] * ($dim[$i] - 1) / 2;
    }
    $ctr_string = join (' ', @ctr);

    # Set up filenames
    my $nrrd_base = File::Basename::fileparse ($nrrd);
    ($vf_gt_fn = catfile ($scratch_base, "$subdir", $nrrd_base)) 
      =~ s/\.nrrd/_vf_gt\.nrrd/;
    ($fixed_fn = catfile ($scratch_base, "$subdir", $nrrd_base)) 
      =~ s/\.nrrd/_warped\.nrrd/;
    $img_out_fn = catfile ($scratch_base, "$subdir", "img_out.nrrd");
    $xf_out_fn = catfile ($scratch_base, "$subdir", "xf_out.nrrd");
    $vf_out_fn = catfile ($scratch_base, "$subdir", "vf_out.nrrd");
    $logfile = catfile ($scratch_base, "$subdir", "logfile.txt");
    $script_fn = catfile ($scratch_base, "$subdir", "parms.txt");

    # Make synthetic vector field
    $cmd = "plastimatch synth-vf --fixed $nrrd --xf-gauss --gauss-center \"$ctr_string\" --output $vf_gt_fn --gauss-mag \"10 20 0\" --gauss-std \"100\"";
    print "$cmd\n";
    system ($cmd);

    # Warp image with vector field
    $cmd = "plastimatch warp --input $nrrd --xf $vf_gt_fn --output-img $fixed_fn";
    print "$cmd\n";
    system ($cmd);

    # Make registration command file
    $script = $script_template;
    $script =~ s/FIXED/$fixed_fn/;
    $script =~ s/MOVING/$nrrd/;
    $script =~ s/VF_OUT/$vf_out_fn/;
    $script =~ s/XF_OUT/$xf_out_fn/;
    $script =~ s/IMG_OUT/$img_out_fn/;
    $script =~ s/LOGFILE/$logfile/;
    open (SCR, ">$script_fn");
    print SCR $script;
    close (SCR);

    # Run registration
    $cmd = "plastimatch register $script_fn";
    print "$cmd\n";
    system ($cmd);

    $subdir ++;
    if ($subdir > 1) {
	last;
    }
}
