use Unireg;
use File::Spec::Functions;
use Sys::Hostname;

my $host = hostname;

# Find processed directory
if ($host eq "deformable") {
    $database = "/home/deformable/database";
} elsif ($host eq "reality-iqtnb9z") {
    $database = "Y:\\database";
}

# Open directory
opendir(DIR, $database) or 
  die "Can't opendir input directory $database: $!\n";

my @list = sort readdir(DIR);
for (@list){
    next if (($_ eq ".") or ($_ eq ".."));
    $outdir = catfile ($database, $_);

#    last if ($_ gt "0001");

    print "$outdir\n";

    my $outdir = catfile ($outdir, "bspline");
    (-d $outdir) or next;

    my $f1 = catfile ($outdir, "t_f0m5p_221_vf.mha");
    my $f2 = catfile ($outdir, "t_f5m0p_221_vf.mha");
    (-f $f1) or next;
    (-f $f2) or next;
    my $outfile = catfile ($outdir, "t_c050_221_vf.mha");
    next if (-f $outfile);

    $cmd = "compose_vector_fields $f1 $f2 $outfile";
    print "$cmd\n";
    print `$cmd\n`;
}
