package Unireg;

use strict;
use warnings;
use File::Spec::Functions;
use File::Path;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
    
    # set the version for version checking
    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    ## $VERSION = do { my @r = (q$Revision: 2.21 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
    

    @ISA         = qw(Exporter);
##    @EXPORT      = qw(&func1 &func2 &func4);
    @EXPORT = qw(Convert4DCT FindPatientContour 
		 Subsample ApplyMask RunBspline);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    
    # your exported package globals go here,
    # as well as any optionally exported functions
##    @EXPORT_OK   = qw($Var1 %Hashit &func3);
#    @EXPORT_OK   = qw(Convert4DCT);
    @EXPORT_OK   = qw();
}
our @EXPORT_OK;

#############################################################
##  Public Functions
#############################################################
sub Convert4DCT {
    my ($inbase, $outbase) = @_;

    my $cvt_cmd = "dicom_to_mha";
    my $only_05 = 0;
    my $outdir = catfile ($outbase, "mha");

    opendir(DIR, $inbase) or die "Can't opendir input directory $inbase: $!\n";
    my @list = sort readdir(DIR);
    for (@list){
	chomp;

	# Skip structures
	next if (/^10/);

	# Skip rpm traces
	next if (/^rpm/);

	# Skip unsorted
	next if (/^unsorted/);

	if ($only_05) {
	    # Convert t0 & t5
	    if (!(/^t[05]/)) {
		next;
	    }
	} else {
	    # Convert t0 - t9
	    if (!(/^t[0-9]/)) {
		next;
	    }
	}

	# We got this far, so make the output directory
	if (!-d $outdir) {
	    mkpath ($outdir);
	}
	
	my $indir = catfile ($inbase, $_);
	my $outfile = catfile ($outdir, "${_}.mha");

	# Skip if output file already exists
	next if (-f $outfile);
	my $cmd = "$cvt_cmd $indir $outfile -s2s";
	print "$cmd\n";
	print `$cmd`;
    }
}

sub FindPatientContour {
    my ($outbase) = @_;

    my $pm_cmd = "patient_mask";
    my $indir = catfile ($outbase, "mha");
    my $outdir = catfile ($outbase, "masks");

    opendir(DIR, $indir) or die "Can't opendir input directory $indir: $!\n";
    my @list = sort readdir(DIR);
    for (@list){
	chomp;
	next if (!(/\.mha$/));

	# Skip if subsampled image
	next if (/_221\.mha$/);

	my $mha_fn = catfile ($indir, $_);
	my $mask_fn = $_;
	$mask_fn =~ s/\.mha$/p\.mha/;
	$mask_fn = catfile ($outdir, $mask_fn);

	# We got this far, so make the output directory
	if (!-d $outdir) {
	    mkpath ($outdir);
	}
	
	# Skip if output file already exists
	next if (-f $mask_fn);
	my $cmd = "$pm_cmd $mha_fn $mask_fn";
	print "$cmd\n";
	print `$cmd`;
    }
}

sub Subsample {
    my ($outbase, $subdir, $type) = @_;

    my $ss_cmd = "resample_mha";
    my $indir = catfile ($outbase, $subdir);

    opendir(DIR, $indir) or die "Can't opendir input directory $indir: $!\n";
    my @list = sort readdir(DIR);
    for (@list){
	chomp;
	next if (!(/\.mha$/));

	# Skip if subsampled image
	next if (/_221\.mha$/);

	my $in_fn = catfile ($indir, $_);
	my $out_fn = $in_fn;
	$out_fn =~ s/\.mha$/_221\.mha/;

	# Skip if output file already exists
	next if (-f $out_fn);
	my $cmd = "$ss_cmd --input=\"${in_fn}\" --output=\"${out_fn}\" --input_type=${type} --output_type=${type} --subsample=\"2 2 1\" ";
	print "$cmd\n";
	print `$cmd`;
    }
}

sub ApplyMask {
    my ($outbase) = @_;

    my $am_cmd = "mask_mha";
    my $indir_mha = catfile ($outbase, "mha");
    my $indir_masks = catfile ($outbase, "masks");
    my $outdir = catfile ($outbase, "masked");

    opendir(DIR, $indir_mha) or die "Can't opendir input directory $indir_mha: $!\n";
    my @list = sort readdir(DIR);
    for (@list){
	chomp;

	# Skip if NOT subsampled image
	next if (!(/_221\.mha$/));

	my $in_mha = catfile ($indir_mha, $_);
	my $in_mask = catfile ($indir_masks, $_);
	$in_mask =~ s/_221\.mha/p_221\.mha/;
	next if (!-f $in_mask);

	my $out_fn = catfile ($outdir, $_);
	$out_fn =~ s/_221\.mha/p_221\.mha/;

	# We got this far, so make the output directory
	if (!-d $outdir) {
	    mkpath ($outdir);
	}

	# Skip if output file already exists
	next if (-f $out_fn);

	my $cmd = "$am_cmd $in_mha $in_mask -1200 $out_fn";
	print "$cmd\n";
	print `$cmd`;
    }
}

## "Private" function
sub gen_parm {
    my ($indir,$outdir,$i,$j,$m) = @_;

    my ($grid_spac, $grad_tol, $conv_tol);
    
    ## Optimization settings
    $grid_spac = 30;
    $grid_spac = 15;

    ## Really fast
    $grad_tol = 10;
    $conv_tol = 30;
    ## Really accurate
    $grad_tol = 0.1;
    $conv_tol = 3.0;
    ## These are defaults
    $grad_tol = 1.5;
    $conv_tol = 5.0;

    ## Fixed image is $i, moving image is $j
    my ($infile1, $infile2, $outfile12, $xfout_file12, $vffile12, $imgfile12);
    $infile1 = catfile ($indir, "t${i}${m}_221.mha");
    $infile2 = catfile ($indir, "t${j}${m}_221.mha");
    $outfile12 = catfile ($outdir, "t_f${i}m${j}${m}_221_img.mha");
    $xfout_file12 = catfile ($outdir, "t_f${i}m${j}${m}_221_bsp.txt");
    $vffile12 = catfile ($outdir, "t_f${i}m${j}${m}_221_vf.mha");
    $imgfile12 = catfile ($outdir, "t_f${i}m${j}${m}_221_img.mha");

    if (!-f $infile1 || !-f $infile2) {
	return "";
    }

    # We got this far, so make the output directory
    if (!-d $outdir) {
	mkpath ($outdir);
    }

    my $parm_fn = catfile($outdir,"parms_f${i}m${j}${m}.txt");
    open PF, ">$parm_fn";
    print PF "[GLOBAL]\n";
    print PF "fixed=$infile1\n";
    print PF "moving=$infile2\n";
    print PF "xf_out=$xfout_file12\n";
    print PF "vf_out=$vffile12\n";
    print PF "img_out=$imgfile12\n";
    
    print PF "\n[STAGE]\n";
    print PF "xform=bspline\n";
    print PF "optim=lbfgsb\n";
    print PF "max_its=50\n";
    print PF "convergence_tol=$conv_tol\n";
    print PF "grad_tol=$grad_tol\n";
    print PF "grid_spac=100 100 100\n";
    print PF "res=4 2 2\n";
    close PF;

    return $parm_fn;
}

sub RunBspline {
    my ($base,$options) = @_;

    my $ref_phase = 0;
    my $run_fwd = 1;
    my $run_inv = 0;

    $|=1;

    ## Which phases
    # @all_phases = (0,2,4,6,8);
    my @all_phases = (0,5);

    ## Segmentation options
    my $run_patient_mask = 1;

    my $reg_cmd = "rad_registration";
    my $indir = catfile ($base, "masked");
    my $outdir = catfile ($base, "bspline");

    opendir(DIR, $indir) or die "Can't opendir input directory $indir: $!\n";
    my ($i, $pfn);
    for $i (@all_phases) {
	next if ($i == $ref_phase);
	if ($run_patient_mask) {
	    if ($run_fwd) {
		if ($pfn = &gen_parm ($indir,$outdir,$i,$ref_phase,"p")) {
		    my $cmd = "$reg_cmd $pfn";
		    print "$cmd\n";
		    system ($cmd);
		}
	    }
	    if ($run_inv) {
		if ($pfn = &gen_parm ($indir,$outdir,$ref_phase,$i,"p")) {
		    my $cmd = "$reg_cmd $pfn";
		    print "$cmd\n";
		    system ($cmd);
		}
	    }
	}
    }
}


#############################################################
##  Module Cleanup
#############################################################
END { }

#############################################################
##  Main
#############################################################
print "Hello from unireg\n";
1;  # don't forget to return a true value from the file
