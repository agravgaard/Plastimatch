use Unireg;
use File::Spec::Functions;
use Sys::Hostname;

my $host = hostname;

## Find raw directory
$have_raw = 0;
if ($host eq "reality-iqtnb9z") {
    $have_raw = 1;
    $rawbase = "G:\\reality\\new-data";
}

## Find processed directory
if ($host eq "deformable") {
    $outbase = "/home/deformable/database";
} elsif ($host eq "reality-iqtnb9z") {
    $outbase = "Y:\\database";
}

## Find index
if ($have_raw) {
    opendir(DIR, $rawbase) or 
	die "Can't opendir input directory $rawbase: $!\n";
} else {
    opendir(DIR, $outbase) or 
	die "Can't opendir output directory $outbase: $!\n";
}

my @list = sort readdir(DIR);
for (@list){
   next if (($_ eq ".") or ($_ eq ".."));
   $rawdir = catfile ($rawbase, $_);
   $outdir = catfile ($outbase, $_);

   if ($have_raw) {
       next if (!-d $rawdir);
       next if (!-d catfile($rawdir,"t0"));
   }

#   next if ($_ lt "0055");
#   last if ($_ gt "0040");

   print "$outdir\n";
   if ($have_raw) {
       Convert4DCT ($rawdir, $outdir);
       next;
   }
   FindPatientContour ($outdir);
   Subsample ($outdir, "mha", "short");
   Subsample ($outdir, "masks", "uchar");
   ApplyMask ($outdir);
}
