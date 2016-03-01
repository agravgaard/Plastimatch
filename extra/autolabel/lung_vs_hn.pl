use File::Spec::Functions;
use File::Path;
use File::Copy;
use File::Spec;
use File::Find;

$out_base = "/PHShome/gcs6/igrt_research/data/autolabel/data";

#$in_base = "/PHShome/gcs6/igrt_research/data/4dct";
#$series = "lung";

$in_base = "/PHShome/gcs6/igrt_research/data/hn";
$series = "hn";

$sampled = 0;
$index = 0;

sub make_sample {
    ($index) = @_;
    $infile = $File::Find::name;
    $outfile = catfile ($out_base, 
			$series . sprintf ("%04d", $index) . ".mhd");

    $cmd = "plastimatch thumbnail --output $outfile $infile";
    print "$cmd\n";
    print `$cmd`;
}

sub Wanted {
    $sampled and return;
    if (/masks/ || /bspline/ || /labelmaps/ || /cxt/) {
	$File::Find::prune = 1;
	return;
    }
    /\.mha/ or return;

    make_sample ($index);
    $index ++;
    $sampled = 1;
}

opendir (DIR, $in_base) or die "Can't opendir input directory $in_base: $!\n";
my @list = sort readdir(DIR);
for (@list){
   next if (($_ eq ".") or ($_ eq ".."));
   $outdir = catfile ($in_base, $_);

   -d $outdir or next;

   print "$outdir\n";
   $sampled = 0;
   find(\&Wanted, $outdir);
}
