use Unireg;
use File::Spec::Functions;
use Sys::Hostname;

my $host = hostname;

## Find processed directory
if ($host eq "deformable") {
    $outbase = "/home/deformable/database";
} elsif ($host eq "reality-iqtnb9z") {
    $outbase = "Y:\\database-testing";
}

opendir(DIR, $outbase) or die "Can't opendir input directory $outbase: $!\n";

my @list = sort readdir(DIR);
for (@list){
   next if (($_ eq ".") or ($_ eq ".."));
   $outdir = catfile ($outbase, $_);

#   next if ($_ lt "0004");
#   last if ($_ gt "0040");

   RunBspline ($outdir);
}
